import os
import pathlib
import re
import shutil
import tomllib
import typing
import unicodedata

from .components.certificate import Certificate
from .components.Dependency import Dependency
from .components.Time import Time
from .config.version import VERSION
from .extensions.Ota import Ota
from .extensions.WebUI import WebUI
from .Firmware import Firmware
from .modes.Weather import Weather

if typing.TYPE_CHECKING:
    from .components.Types import Environment
else:
    from SCons.Script import Environment


class Kortlinje:
    certificate: Certificate | None = None
    dependency: Dependency | None = None
    env: Environment
    firmware: Firmware | None = None
    ota: Ota | None = None
    time: Time | None = None
    toml: dict[str, typing.Any]
    weather: Weather | None = None
    webui: WebUI | None = None

    def __init__(self, env: Environment) -> None:
        """
        Initialize the project orchestrator with the provided build environment.

        Parameters:
            env (Environment): SCons build environment used to configure project components.
        """
        self.toml = tomllib.loads(pathlib.Path("secrets.toml").read_text(encoding="utf-8"))
        self.env = env
        self.certificate = Certificate(self)
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
        device = self.toml.setdefault("device", {})
        name = device.setdefault("name", "Kortlinje")
        device["hostname"] = (
            device["hostname"].lower()
            if "hostname" in device
            else re.sub(
                r"[^a-z0-9]+",
                "-",
                unicodedata.normalize("NFKD", name).encode("ascii", "ignore").decode("ascii").lower(),
            ).strip("-")
        )
        if self.certificate:
            self.certificate.initialize()
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
        """Write the certificate bundle and finalize enabled firmware, OTA, and Web UI components."""
        if self.certificate:
            self.certificate.finalize()
        if self.firmware:
            self.firmware.finalize()
        if self.ota:
            self.ota.finalize()
        if self.webui:
            self.webui.finalize()

    @staticmethod
    def clean() -> None:
        """Remove generated files and directories from the project workspace."""
        Certificate.clean()
        Firmware.clean()
        WebUI.clean()
        for path in (
            "logs",
            "scripts/__pycache__",
        ):
            if os.path.exists(path):
                shutil.rmtree(path, ignore_errors=True)
                print(f"Removing {path}")
