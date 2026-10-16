import gzip
import os
import pathlib
import shutil
import subprocess
import typing

import dotenv
import nodejs_wheel

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
        if COMMAND_LINE_TARGETS in [
            ["compiledb"],
        ]:
            self.project.webui = None

    def finalize(self) -> None:
        """
        Synchronize project settings with the web app, build the app, and register OTA commissioning actions.
        """
        options = (
            "HOSTNAME",
            "NAME",
            "TEMPERATURE_UNIT",
        )
        prefixes = (
            "EXTENSION_",
            "IKEA_",
            "MODE_",
        )
        for option, value in self.project.dotenv.items():
            if option in ("OTA_KEY",):
                dotenv.set_key(self.path / ".env", f"VITE_{option}", "true")
            elif option in options or option.startswith(prefixes):
                dotenv.set_key(self.path / ".env", f"VITE_{option}", value)
        config = self.project.env.GetProjectConfig()
        for option in ("board",):
            value = config.get(f"env:{self.project.env['PIOENV']}", option, None)
            if value:
                dotenv.set_key(self.path / ".env", f"VITE_{option.upper()}", value)
        for option in dotenv.dotenv_values(self.path / ".env"):
            if option.startswith("VITE_"):
                _option = option.removeprefix("VITE_")
                if _option not in self.project.dotenv and (_option in options or _option.startswith(prefixes)):
                    dotenv.unset_key(self.path / ".env", option)
        self._npm_build()

    def _npm_build(self) -> None:
        """
        Build the web application and compress its generated index page for packaging.

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
        with gzip.open(self.path / "dist" / "index.html.gz", "wb") as gz:
            gz.write((self.path / "dist" / "index.html").read_bytes())

    @staticmethod
    def clean() -> None:
        """Remove generated web app files and dependencies from the project."""
        for path in ("webui/dist",):
            for data in os.scandir(path):
                if data.is_file():
                    pathlib.Path(data.path).unlink()
                    print(f"Removing {data.path}")
        node_modules = "webui/node_modules"
        if os.path.exists(node_modules):
            shutil.rmtree(node_modules, ignore_errors=True)
            print(f"Removing {node_modules}")
