import typing

if typing.TYPE_CHECKING:
    from ..Kortlinje import Kortlinje


class Weather:
    HOSTS: typing.Final[tuple[str, ...]] = (
        "api.met.no",
        "api.open-meteo.com",
        "wttr.in",
    )
    NAME: typing.Final[str] = "Weather"

    def __init__(self, project: "Kortlinje") -> None:
        """Initialize the weather mode with a project reference."""
        self.project = project

    def configure(self) -> None:
        """
        Normalize coordinates and discover roots for all configured provider hosts.

        Latitude and longitude are formatted as decimal strings with up to four
        decimal places, defaulting to "0" when absent. Invalid numeric strings
        raise ValueError. When the certificate component is enabled, probe every
        host in HOSTS, regardless of the selected weather mode.
        """
        if self.project.certificate:
            for hostname in self.HOSTS:
                self.project.certificate.add_host(hostname)
        options = (
            "LATITUDE",
            "LONGITUDE",
        )
        precision = min(
            4,
            max(
                (
                    len(str(self.project.dotenv[option]).strip().partition(".")[2].rstrip("0"))
                    for option in options
                    if option in self.project.dotenv
                ),
                default=0,
            ),
        )
        for option in options:
            self.project.dotenv[option] = (
                f"{float(self.project.dotenv[option]):.{precision}f}" if option in self.project.dotenv else "0"
            )
