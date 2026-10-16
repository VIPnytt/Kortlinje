import os
import re
import shutil
import typing
import unicodedata

import dotenv

from .components.Dependency import Dependency
from .components.Time import Time
from .config.version import VERSION
from .devices.IkeaKortlinje import IkeaKortlinje
from .extensions.Ota import Ota
from .extensions.WebUI import WebUI
from .Firmware import Firmware
from .modes.Weather import Weather

if typing.TYPE_CHECKING:
    from .components.Types import Environment
else:
    from SCons.Script import Environment


class Kortlinje:
    dependency: Dependency | None = None
    dotenv: dict[str, str]
    env: Environment
    firmware: Firmware | None = None
    ota: Ota | None = None
    time: Time | None = None
    weather: Weather | None = None
    webui: WebUI | None = None

    def __init__(self, env: Environment) -> None:
        """
        Initialize the project orchestrator with the provided build environment.

        Parameters:
            env (Environment): SCons build environment used to configure project components.
        """
        self.env = env
        self.dotenv = {key: (value if value is not None else "") for key, value in dotenv.dotenv_values(".env").items()}
        self.dependency = Dependency(self)
        self.firmware = Firmware(self)
        self.ota = Ota(self)
        self.time = Time(self)
        self.weather = Weather(self)
        self.webui = WebUI(self)

    def run(self) -> None:
        """
        Execute the complete Kortlinje build lifecycle.
        """
        print(f"Kortlinje {VERSION}")
        self.initialize()
        self.configure()
        self.finalize()

    def initialize(self) -> None:
        """
        Initialize project settings and enabled components.

        Determines default application and host names, normalizes configured hostnames, and initializes the available project components.
        """
        if "NAME" not in self.dotenv:
            self.dotenv["NAME"] = IkeaKortlinje.NAME
        if "HOSTNAME" in self.dotenv:
            self.dotenv["HOSTNAME"] = self.dotenv["HOSTNAME"].lower()
        else:
            self.dotenv["HOSTNAME"] = re.sub(
                r"[^a-z0-9]+",
                "-",
                unicodedata.normalize("NFKD", self.dotenv["NAME"]).encode("ascii", "ignore").decode("ascii").lower(),
            ).strip("-")
        if self.dependency:
            self.dependency.initialize()
        if self.webui:
            self.webui.initialize()

    def configure(self) -> None:
        """Configure the enabled OTA, time, and weather components."""
        if self.ota:
            self.ota.configure()
        if self.time:
            self.time.configure()
        if self.weather:
            self.weather.configure()

    def finalize(self) -> None:
        if self.firmware:
            self.firmware.finalize()
        if self.ota:
            self.ota.finalize()
        if self.webui:
            self.webui.finalize()

    @staticmethod
    def clean() -> None:
        """Remove generated files and directories from the project workspace."""
        WebUI.clean()
        for path in (
            "logs",
            "scripts/__pycache__",
        ):
            if os.path.exists(path):
                shutil.rmtree(path, ignore_errors=True)
                print(f"Removing {path}")
