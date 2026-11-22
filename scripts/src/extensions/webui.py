import gzip
import json
import pathlib
import shutil
import subprocess
import typing

import nodejs_wheel

from ..config.version import VERSION

if typing.TYPE_CHECKING:
    from ..components.types import COMMAND_LINE_TARGETS
    from ..kortlinje import Kortlinje
else:
    from SCons.Script import COMMAND_LINE_TARGETS


class WebUI:
    NAME: typing.Literal["Web UI"] = "Web UI"
    path: typing.Final[pathlib.Path] = pathlib.Path("webui")
    project: "Kortlinje"

    def __init__(self, project: "Kortlinje") -> None:
        """Initialize the web application extension for a project.

        Parameters:
            project (Kortlinje): The project associated with the web application.
        """
        self.project = project

    def initialize(self) -> None:
        """
        Disable the web app for command-line targets that do not require it.
        """
        if COMMAND_LINE_TARGETS == ["compiledb"]:
            self.project.webui = None

    def finalize(self) -> None:
        """
        Synchronize project settings with the web app, build the app, and register dependencies.
        """
        device = self.project.toml.get("device", {})
        secrets_lines: list[str] = []
        for key, value in {
            "ARDUINO_VARIANT": self.project.env.BoardConfig().get("build.variant"),
            "DEVICE_HOSTNAME": device.get("hostname"),
            "DEVICE_NAME": device.get("name"),
            "DEVICE_TEMPERATURE_UNIT": device.get("temperature_unit"),
        }.items():
            if isinstance(value, bool):
                ts_type = "boolean"
            elif isinstance(value, int | float):
                ts_type = "number"
            else:
                ts_type = "string"
                value = str(value)
            secrets_lines.append(f"export const {key}: {ts_type} = {json.dumps(value, ensure_ascii=False)};")

        constants = self.path / "src" / "constants"
        constants.mkdir(parents=True, exist_ok=True)
        for file, content in {
            constants / "secrets.ts": "\n".join(secrets_lines) + "\n",
            constants / "version.ts": f"export const VERSION: string = {json.dumps(VERSION, ensure_ascii=False)};\n",
        }.items():
            if not file.is_file() or file.read_text(encoding="utf-8") != content:
                file.write_text(content, encoding="utf-8")

        self._npm_build()

    def _npm(self, *args: str) -> None:
        """Run an npm command via the bundled Node.js wheel, falling back to system ``npm``."""
        try:
            nodejs_wheel.npm(list(args), cwd=self.path, check=True)
        except FileNotFoundError:
            subprocess.run([shutil.which("npm") or "npm", *args], cwd=self.path, check=True)

    def _npm_build(self) -> None:
        """
        Install web dependencies when missing or stale, build the web app, and compress its assets.
        """
        lock = self.path / "package-lock.json"
        installed = self.path / "node_modules" / ".package-lock.json"
        if not installed.is_file() or (lock.is_file() and lock.stat().st_mtime > installed.stat().st_mtime):
            self._npm("ci" if lock.is_file() else "install")
        self._npm("run", "build")
        webui_obj = self.project.env.File("${BUILD_DIR}/src/extensions/WebUiExtension.cpp.o")
        for file in (self.path / "dist").iterdir():
            if file.is_file() and file.suffix != ".gz":
                compressed = gzip.compress(file.read_bytes(), mtime=0)
                gz_file = file.with_name(f"{file.name}.gz")
                if not gz_file.is_file() or gz_file.read_bytes() != compressed:
                    gz_file.write_bytes(compressed)
                self.project.env.Depends(webui_obj, self.project.env.File(f"#{gz_file.as_posix()}"))

    @staticmethod
    def clean() -> None:
        """Remove generated web app files and dependencies from the project."""
        for file in (
            WebUI.path / "src" / "constants" / "secrets.ts",
            WebUI.path / "src" / "constants" / "version.ts",
        ):
            if file.is_file():
                file.unlink()
                print(f"Removing {file}")
        if (dist := WebUI.path / "dist").is_dir():
            for file in dist.iterdir():
                if file.is_file():
                    file.unlink()
                    print(f"Removing {file}")
        if (node_modules := WebUI.path / "node_modules").exists():
            shutil.rmtree(node_modules, ignore_errors=True)
            print(f"Removing {node_modules}")
