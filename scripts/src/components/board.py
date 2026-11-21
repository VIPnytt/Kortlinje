import contextlib
import http.client
import json
import logging
import pathlib
import re
import subprocess
import tomllib
import typing
import urllib.request

from ..config.version import VERSION

if typing.TYPE_CHECKING:
    from .types import Environment
else:
    from SCons.Script import Environment


class Board:
    header: typing.Final[pathlib.Path] = pathlib.Path("firmware") / "include" / "constants" / "board.h"
    chip_id: int
    chip_type: str
    env: Environment
    flash_size_bytes: int
    package: str
    protocol: str | None
    revision: int
    rom_gitrev: int
    write: bool
    _tried_hosts: set[str]

    def __init__(self, env: Environment) -> None:
        self.chip_id = 0
        self.chip_type = ""
        self.env = env
        self.flash_size_bytes = 0
        self.package = ""
        self.protocol = (
            self.env.subst("$UPLOAD_PROTOCOL")
            or self.env.GetProjectOption("upload_protocol", None)
            or self.env.BoardConfig().get("upload.protocol")
        )
        self.revision = 0
        self.rom_gitrev = 0
        self.write = False
        self._tried_hosts = set()
        self._load()

    def configure(self) -> None:
        if self.protocol == "picotool":
            self._picotool()
        if not self.write:
            self._generic()

    def finalize(self) -> None:
        if self.write or not self.header.is_file():
            content = (
                "\n".join(
                    [
                        "#pragma once",
                        "",
                        "#include <cstdint>",
                        "#include <string_view>",
                        "",
                        "namespace Kortlinje::board",
                        "{",
                        f"inline constexpr uint8_t revision{{0x{self.revision:X}U}};",
                        f"inline constexpr uint32_t flash_size{{{self.flash_size_bytes}UL}};",
                        f"inline constexpr uint32_t rom_gitrev{{0x{self.rom_gitrev:X}UL}};",
                        f"inline constexpr uint64_t chipid{{0x{self.chip_id:X}ULL}};",
                        f'inline constexpr std::string_view package{{"{self.package}"}};',
                        f'inline constexpr std::string_view type{{"{self.chip_type}"}};',
                        "} // namespace Kortlinje::board",
                    ]
                )
                + "\n"
            )
            if not self.header.is_file() or self.header.read_text(encoding="utf-8") != content:
                self.header.write_text(content, encoding="utf-8")

    def post(self) -> None:
        self.protocol = self.env.subst("$UPLOAD_PROTOCOL") or self.protocol
        if self.protocol == "picotool":
            for node in self.env.Alias("upload"):
                node.get_executor().action_list[0].list.insert(
                    1,
                    self.env.VerboseAction(self._callback, "Detecting device..."),
                )
        elif self.protocol == "espota":
            self.env.AddPreAction(
                "upload",
                self.env.VerboseAction(self._callback, "Detecting device..."),
            )

    def _load(self) -> None:
        if not self.header.is_file():
            return
        text = self.header.read_text(encoding="utf-8")
        if m := re.search(r"\brevision\{0x([0-9A-Fa-f]+)U\}", text):
            self.revision = int(m.group(1), 16)
        if m := re.search(r"\bflash_size\{(\d+)UL\}", text):
            self.flash_size_bytes = int(m.group(1))
        if m := re.search(r"\brom_gitrev\{0x([0-9A-Fa-f]+)UL\}", text):
            self.rom_gitrev = int(m.group(1), 16)
        if m := re.search(r"\bchipid\{0x([0-9A-Fa-f]+)ULL\}", text):
            self.chip_id = int(m.group(1), 16)
        if m := re.search(r'\bpackage\{"([^"]*)"\}', text):
            self.package = m.group(1)
        if m := re.search(r'\btype\{"([^"]*)"\}', text):
            self.chip_type = m.group(1)

    def _callback(self, source: list[typing.Any], target: list[typing.Any], env: Environment) -> None:
        if self.protocol == "picotool":
            self._picotool()
        elif self.protocol == "espota" and not self.write:
            self._espota(self.env.subst("$UPLOAD_PORT"))
        if not self.write:
            self._generic()
        if self.write:
            self.finalize()
            parts = [self.chip_type, f"{self.revision:X}", self.package]
            if self.flash_size_bytes:
                parts.append(f"{self.flash_size_bytes // 2**20}M")
            print(" ".join(filter(None, parts)))

    def _generic(self) -> None:
        secrets = pathlib.Path("secrets.toml")
        data = tomllib.loads(secrets.read_text(encoding="utf-8")) if secrets.is_file() else {}
        hostname = data.get("device", {}).get("hostname") or "kortlinje"
        self._espota(f"{hostname}.local")

    def _espota(self, host: str) -> None:
        host = host.strip()
        if not host or host in self._tried_hosts:
            return
        self._tried_hosts.add(host)
        try:
            req = urllib.request.Request(
                f"http://{host}/rest/ota",
                headers={
                    "Accept": "application/json",
                    "User-Agent": f"Kortlinje/{VERSION} (+https://github.com/VIPnytt/Kortlinje)",
                },
            )
            with urllib.request.urlopen(req, timeout=3) as res:
                data = json.load(res)
            rev = int(data["RP2350"], 16)
            chip_type = "RP2350"
            if (self.chip_type and self.chip_type != chip_type) or (self.revision and self.revision != rev):
                self.chip_id = self.flash_size_bytes = self.rom_gitrev = 0
                self.package = ""
            self.chip_type = chip_type
            self.revision = rev
            self.write = True
        except (http.client.HTTPException, KeyError, OSError, TypeError, ValueError) as err:
            logging.getLogger(__name__).debug("Failed to query OTA device at %s: %s", host, err)

    def _picotool(self) -> None:
        try:
            picotool = subprocess.run(
                [self._cli(), "info", "-d", "--debug"],
                capture_output=True,
                check=False,
                text=True,
                timeout=2,
            )
            type_match = re.search(r"^\s*type:\s+(\S+)", picotool.stdout, re.MULTILINE)
            rev_match = re.search(r"revision:\s+([A-F]\d+)\b", picotool.stdout)
            if not type_match or not rev_match:
                return
            self.chip_type = type_match.group(1).upper()
            self.revision = int(rev_match.group(1), 16)
            if chipid_match := re.search(r"chipid:\s+0x([0-9a-fA-F]+)", picotool.stdout):
                self.chip_id = int(chipid_match.group(1), 16)
            if flash_match := re.search(r"flash size:\s+(\d+)K\b", picotool.stdout):
                self.flash_size_bytes = int(flash_match.group(1)) * 1024
            if pkg_match := re.search(r"package:\s+(\S+)", picotool.stdout):
                self.package = pkg_match.group(1)
            if rom_match := re.search(r"rom gitrev:\s+0x([0-9a-fA-F]+)", picotool.stdout):
                self.rom_gitrev = int(rom_match.group(1), 16)
            self.write = True
        except (OSError, subprocess.SubprocessError, ValueError) as err:
            logging.getLogger(__name__).debug("Failed to query picotool device: %s", err)

    def _cli(self) -> str:
        pkg = "tool-picotool-rp2040-earlephilhower"
        exe = "picotool.exe" if self.env["PLATFORM"] == "win32" else "picotool"
        platform = self.env.PioPlatform()
        if pkg in platform.packages:
            with contextlib.suppress(KeyError, OSError):
                if (pkg_dir := platform.get_package_dir(pkg)) and (pathlib.Path(pkg_dir) / exe).is_file():
                    return str(pathlib.Path(pkg_dir) / exe)
        fallback = pathlib.Path(self.env.subst("$PROJECT_PACKAGES_DIR")) / pkg / exe
        return str(fallback) if fallback.is_file() else exe

    @staticmethod
    def clean() -> None:
        if Board.header.is_file():
            Board.header.unlink()
            print(f"Removing {Board.header}")
