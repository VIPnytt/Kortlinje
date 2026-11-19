import contextlib
import datetime
import importlib.resources
import locale
import logging
import pathlib
import typing
import zoneinfo

import tzlocal

if typing.TYPE_CHECKING:
    from ..kortlinje import Kortlinje


class Time:
    project: "Kortlinje"

    def __init__(self, project: "Kortlinje") -> None:
        """Initialize the time component with its associated project.

        Parameters:
            project (Kortlinje): Project whose time configuration is managed.
        """
        self.project = project

    def configure(self) -> None:
        """Configure the project's timezone and clock display settings.

        Missing timezone fields are resolved from ``time.zone`` or the host OS,
        falling back to UTC. Missing or unsupported ``time.format`` values are
        inferred from the host locale.
        """
        time = self.project.toml.setdefault("time", {})
        self._configure_format(time)
        self._configure_zone(time)

    @classmethod
    def _configure_zone(cls, time: dict[str, typing.Any]) -> None:
        """Populate missing ``time.zone`` and ``time.posix`` values in place."""
        zone = time.get("zone")
        posix = time.get("posix")
        if zone and posix:
            return
        if posix:
            time.setdefault("zone", "")
            return
        if zone and (resolved := cls._lookup_zone(zone)):
            time["posix"] = resolved
            return
        if (local_zone := tzlocal.get_localzone_name()) and (resolved := cls._lookup_zone(local_zone)):
            time["zone"] = local_zone
            time["posix"] = resolved
            return
        time["zone"] = "Etc/UTC"
        time["posix"] = "UTC0"

    @classmethod
    def _configure_format(cls, time: dict[str, typing.Any]) -> None:
        """Ensure ``time.format`` is set to ``12`` or ``24`` in place."""
        fmt = time.get("format")
        if isinstance(fmt, int) and fmt in (12, 24):
            return
        if fmt in ("12", "24"):
            time["format"] = int(fmt)
            return
        if fmt is not None:
            logging.getLogger(__name__).warning("time.format %r is unsupported. Valid values are 12 and 24.", fmt)
        time["format"] = cls._detect_format()

    @staticmethod
    def _detect_format() -> int:
        """Infer the 12-hour or 24-hour clock format from the host locale."""
        previous = locale.setlocale(locale.LC_TIME)
        try:
            locale.setlocale(locale.LC_TIME, "")
            return 12 if "11" in datetime.time(23).strftime("%X") else 24
        except locale.Error:
            return 24
        finally:
            locale.setlocale(locale.LC_TIME, previous)

    @classmethod
    def _lookup_zone(cls, iana: str) -> str | None:
        """Resolve an IANA timezone identifier to its embedded POSIX timezone string.

        Parameters:
            iana (str): IANA timezone identifier to resolve.

        Returns:
            str: The embedded POSIX timezone string.
            None: If the timezone cannot be found or contains invalid data.
        """
        parts = iana.split("/") if isinstance(iana, str) else []
        if not parts or any(part in {"", ".", ".."} for part in parts):
            logging.getLogger(__name__).warning("Unknown timezone: %r", iana)
            return None
        data = cls._read_tzif(parts)
        if data and data.startswith(b"TZif") and data.endswith(b"\n"):
            footer = data.rsplit(b"\n", 2)
            if len(footer) == 3 and footer[1]:
                with contextlib.suppress(UnicodeDecodeError):
                    return footer[1].decode("ascii")
        logging.getLogger(__name__).warning("Unknown timezone: %r", iana)
        return None

    @staticmethod
    def _read_tzif(parts: list[str]) -> bytes | None:
        """Read raw TZif bytes from ``zoneinfo.TZPATH`` or the ``tzdata`` package."""
        for directory in zoneinfo.TZPATH:
            zone_file = pathlib.Path(directory, *parts)
            if zone_file.is_file():
                return zone_file.read_bytes()
        with contextlib.suppress(ModuleNotFoundError):
            resource = importlib.resources.files("tzdata.zoneinfo").joinpath(*parts)
            if resource.is_file():
                return resource.read_bytes()
        return None
