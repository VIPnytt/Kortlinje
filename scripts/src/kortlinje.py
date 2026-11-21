import os
import pathlib
import re
import shutil
import tomllib
import typing
import unicodedata

from .components.board import Board
from .components.certificate import Certificate
from .components.dependency import Dependency
from .components.time import Time
from .config.pins import PINS
from .config.version import VERSION
from .extensions.mqtt import Mqtt
from .extensions.ota import Ota
from .extensions.scd4x import Scd4x
from .extensions.webui import WebUI
from .firmware import Firmware
from .modes.weather import Weather

if typing.TYPE_CHECKING:
    from .components.types import Environment
else:
    from SCons.Script import Environment


class Kortlinje:
    board: Board
    certificate: Certificate | None = None
    dependency: Dependency
    env: Environment
    firmware: Firmware
    mqtt: Mqtt
    ota: Ota
    scd4x: Scd4x
    time: Time
    toml: dict[str, typing.Any]
    weather: Weather
    webui: WebUI | None = None

    def __init__(self, env: Environment) -> None:
        """
        Initialize the project orchestrator with the provided build environment.

        Parameters:
            env (Environment): SCons build environment used to configure project components.
        """
        secrets = pathlib.Path("secrets.toml")
        self.toml = tomllib.loads(secrets.read_text(encoding="utf-8")) if secrets.is_file() else {}
        self.env = env
        self.board = Board(self.env)
        self.certificate = Certificate(self)
        self.dependency = Dependency(self)
        self.firmware = Firmware(self)
        self.mqtt = Mqtt(self)
        self.ota = Ota(self)
        self.scd4x = Scd4x(self)
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

        Determines default application and host names, normalizes configured hostnames, defaults pins and Wi-Fi credentials, and initializes the available project components.
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
        pin = self.toml.setdefault("pin", {})
        for key, value in PINS.items():
            pin.setdefault(key, value)
        wifi = self.toml.setdefault("wifi", {})
        for key in ("psk", "ssid"):
            wifi.setdefault(key, "")
        if self.certificate:
            self.certificate.initialize()
        self.dependency.initialize()
        if self.webui:
            self.webui.initialize()

    def configure(self) -> None:
        """Configure the enabled OTA, SCD4x, time, and weather components."""
        self.board.configure()
        self.mqtt.configure()
        self.ota.configure()
        self.scd4x.configure()
        self.time.configure()
        self.weather.configure()

    def finalize(self) -> None:
        """Write the certificate bundle and finalize enabled firmware, OTA, and Web UI components."""
        self.board.finalize()
        if self.certificate:
            self.certificate.finalize()
        self.firmware.finalize()
        self.ota.finalize()
        if self.webui:
            self.webui.finalize()

    @staticmethod
    def clean() -> None:
        """Remove generated files and directories from the project workspace."""
        Board.clean()
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
