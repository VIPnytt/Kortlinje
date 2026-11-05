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
        Configure weather settings and normalize location-related options.

        Weather is disabled unless the weather mode option is set to ``"true"``.
        Latitude and longitude are formatted as decimal strings with up to four
        decimal places when provided.
        """
        for option in (
            "LATITUDE",
            "LONGITUDE",
        ):
            if option in self.project.dotenv:
                self.project.dotenv[option] = f"{float(self.project.dotenv[option]):.4f}".rstrip("0").rstrip(".")
        if self.project.certificate:
            for hostname in self.HOSTS:
                self.project.certificate.add_host(hostname)
