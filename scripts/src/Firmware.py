import json
import pathlib
import typing

from .config.version import VERSION

if typing.TYPE_CHECKING:
    from .Kortlinje import Kortlinje


class Firmware:
    NAME: typing.Final[str] = "Firmware"
    path: pathlib.Path
    project: "Kortlinje"

    def __init__(self, project: "Kortlinje") -> None:
        """Initialize the firmware component with its parent project and component path."""
        self.path = pathlib.Path("firmware")
        self.project = project

    def finalize(self) -> None:
        """Generate the C++ configuration header from TOML options."""
        secrets_lines: list[str] = [
            "#pragma once",
            "",
            "#include <pico/types.h>",
            "#include <string_view>",
        ]
        for section, options in sorted(self.project.toml.items()):
            if not isinstance(options, dict):
                continue
            members: list[str] = []
            for key, value in sorted(options.items()):
                if value is None:
                    continue
                if isinstance(value, bool):
                    cpp_type = "bool"
                    cpp_value = "true" if value else "false"
                elif isinstance(value, int) and value >= 0:
                    cpp_type = "uint"
                    cpp_value = f"{value!s}U"
                elif isinstance(value, int):
                    cpp_type = "int"
                    cpp_value = str(value)
                elif isinstance(value, float):
                    cpp_type = "float"
                    cpp_value = str(value)
                else:
                    cpp_type = "std::string_view"
                    cpp_value = json.dumps(str(value), ensure_ascii=False)
                members.append(f"inline constexpr {cpp_type} {key}{{{cpp_value}}};")
            if members:
                secrets_lines.extend(
                    [
                        "",
                        f"namespace Kortlinje::{section}",
                        "{",
                        *members,
                        f"}} // namespace Kortlinje::{section}",
                    ]
                )
        secrets_content = "\n".join(secrets_lines) + "\n"
        secrets_path = self.path / "include" / "constants" / "secrets.h"
        if not secrets_path.is_file() or secrets_path.read_text(encoding="utf-8") != secrets_content:
            secrets_path.write_text(secrets_content, encoding="utf-8")
        version_path = self.path / "include" / "constants" / "Kortlinje.h"
        version_content = (
            "\n".join(
                [
                    "#pragma once",
                    "",
                    "#include <string_view>",
                    "",
                    "namespace Kortlinje",
                    "{",
                    f'inline constexpr std::string_view version{{"{VERSION}"}};',
                    "} // namespace Kortlinje",
                ]
            )
            + "\n"
        )
        if not version_path.is_file() or version_path.read_text(encoding="utf-8") != version_content:
            version_path.write_text(version_content, encoding="utf-8")

    @staticmethod
    def clean() -> None:
        """Remove generated PIO and configuration header files from the project."""
        firmware = pathlib.Path("firmware")
        for file in (
            firmware / "include" / "constants" / "Kortlinje.h",
            firmware / "include" / "constants" / "secrets.h",
            *(firmware / "src" / "pio").glob("*.pio.h"),
        ):
            if file.is_file():
                file.unlink()
                print(f"Removing {file}")
