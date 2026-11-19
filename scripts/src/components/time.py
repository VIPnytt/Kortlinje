import datetime
import importlib.util
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
        """
        Configure the project's timezone and clock display settings.

        The timezone is stored in IANA and POSIX formats. The clock display preference is
        set from ``time.format`` when provided, or inferred from the system locale.
        Unsupported clock format values are ignored.
        """
        time = self.project.toml.setdefault("time", {})
        time["timezone"], time["posix"] = self._get_zone()
        fmt = time.get("format")
        if str(fmt) in ("12", "24"):
            time["format"] = int(fmt)
        else:
            if fmt is not None:
                logging.getLogger(__name__).warning("time.format %r is unsupported. Valid values are 12 and 24.", fmt)
            previous = locale.setlocale(locale.LC_TIME)
            try:
                locale.setlocale(locale.LC_TIME, "")
                time["format"] = 12 if datetime.time(23).strftime("%X").startswith("11") else 24
            finally:
                locale.setlocale(locale.LC_TIME, previous)

    def _get_zone(self) -> tuple[str, str]:
        """
        Resolve the configured or system timezone for the project.

        Returns:
            tuple[str, str]: The IANA timezone name and its corresponding POSIX timezone string.
        """
        time = self.project.toml.get("time", {})
        if (iana := time.get("timezone")) and (posix := self._lookup_zone(iana)):
            return iana, posix
        if (iana := tzlocal.get_localzone_name()) and (posix := self._lookup_zone(iana)):
            return iana, posix
        return "Etc/UTC", "UTC0"

    @staticmethod
    def _lookup_zone(iana: str) -> str | None:
        """Resolve an IANA timezone identifier to its embedded POSIX timezone string.

        Parameters:
            iana (str): IANA timezone identifier to resolve.

        Returns:
            str: The embedded POSIX timezone string.
            None: If the timezone cannot be found or contains invalid data.
        """
        paths = [pathlib.Path(path) for path in zoneinfo.TZPATH]
        spec = importlib.util.find_spec("tzdata")
        if spec and spec.submodule_search_locations:
            paths.extend(pathlib.Path(path) / "zoneinfo" for path in spec.submodule_search_locations)
        for path in dict.fromkeys(paths):
            zone_file = path / iana
            if zone_file.is_file():
                zone = zone_file.read_bytes()
                if zone.startswith(b"TZif") and zone.endswith(b"\n"):
                    posix = zone.rsplit(b"\n", 2)[-2].decode("ascii", errors="strict")
                    if posix:
                        return posix
        logging.getLogger(__name__).warning("Unknown timezone: %r", iana)
        return None
