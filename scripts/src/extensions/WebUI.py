import gzip
import json
import pathlib
import shutil
import subprocess
import typing

import nodejs_wheel

from ..config.version import VERSION

if typing.TYPE_CHECKING:
    from ..components.Types import COMMAND_LINE_TARGETS
    from ..Kortlinje import Kortlinje
else:
    from SCons.Script import COMMAND_LINE_TARGETS


class WebUI:
    NAME: typing.Final[str] = "Web UI"
    path: pathlib.Path
    project: "Kortlinje"

    def __init__(self, project: "Kortlinje") -> None:
        """Initialize the web application extension for a project.

        Parameters:
            project (Kortlinje): The project associated with the web application.
        """
        self.path = pathlib.Path("webui")
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
            "OTA_PASSWORD": self.project.toml.get("ota", {}).get("password") != "",
        }.items():
            if isinstance(value, bool):
                ts_type = "boolean"
                ts_value = "true" if value else "false"
            elif isinstance(value, int):
                ts_type = "number"
                ts_value = int(value)
            elif isinstance(value, float):
                ts_type = "number"
                ts_value = float(value)
            else:
                ts_type = "string"
                ts_value = json.dumps(str(value), ensure_ascii=False)
            secrets_lines.append(f"export const {key}: {ts_type} = {ts_value};")
        secrets_content = "\n".join(secrets_lines) + "\n"
        secrets_path = self.path / "src" / "constants" / "secrets.ts"
        secrets_path.parent.mkdir(parents=True, exist_ok=True)
        if not secrets_path.is_file() or secrets_path.read_text(encoding="utf-8") != secrets_content:
            secrets_path.write_text(secrets_content, encoding="utf-8")
        version_path = self.path / "src" / "constants" / "version.ts"
        version_path.parent.mkdir(parents=True, exist_ok=True)
        version_content = f'export const VERSION: string = "{VERSION}";\n'
        if not version_path.is_file() or version_path.read_text(encoding="utf-8") != version_content:
            version_path.write_text(version_content, encoding="utf-8")
        self._npm_build()

    def _npm_build(self) -> None:
        """
        Build the web application and compress its generated assets for packaging.

        The build retries after installing dependencies when the initial build fails and falls back to the system `npm` executable when the bundled npm tool is unavailable.
        """
        build_cmd = ["run", "build"]
        install_cmd = ["install"]
        try:
            if nodejs_wheel.npm(build_cmd, stderr=subprocess.DEVNULL, cwd=self.path, check=False):
                nodejs_wheel.npm(install_cmd, cwd=self.path, check=True)
                nodejs_wheel.npm(build_cmd, cwd=self.path, check=True)
        except FileNotFoundError:
            if subprocess.run(["npm", *build_cmd], stderr=subprocess.DEVNULL, cwd=self.path, check=False).returncode:
                subprocess.run(["npm", *install_cmd], cwd=self.path, check=True)
                subprocess.run(["npm", *build_cmd], cwd=self.path, check=True)
        for file in (self.path / "dist").iterdir():
            if file.is_file() and file.suffix != ".gz":
                (file.parent / f"{file.name}.gz").write_bytes(gzip.compress(file.read_bytes(), mtime=0))
                self.project.env.Depends(
                    self.project.env.File("${BUILD_DIR}/src/extensions/WebUiExtension.cpp.o"),
                    self.project.env.File(f"#{file.as_posix()}"),
                )

    @staticmethod
    def clean() -> None:
        """Remove generated web app files and dependencies from the project."""
        webui = pathlib.Path("webui")
        for file in (
            webui / "src" / "constants" / "secrets.ts",
            webui / "src" / "constants" / "version.ts",
        ):
            if file.is_file():
                file.unlink()
                print(f"Removing {file}")
        if (dist := webui / "dist").is_dir():
            for file in dist.iterdir():
                if file.is_file():
                    file.unlink()
                    print(f"Removing {file}")
        if (node_modules := webui / "node_modules").exists():
            shutil.rmtree(node_modules, ignore_errors=True)
            print(f"Removing {node_modules}")
