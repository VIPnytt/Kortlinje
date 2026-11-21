import typing

if typing.TYPE_CHECKING:
    from ..kortlinje import Kortlinje
else:
    pass


class Scd4x:
    NAME: typing.Literal["SCD4x"] = "SCD4x"
    project: "Kortlinje"

    def __init__(self, project: "Kortlinje") -> None:
        self.project = project

    def configure(self) -> None:
        """Default the optional SCD4x I2C pins to -1, which disables the sensor."""
        pin = self.project.toml.setdefault("pin", {})
        for key in ("co2_scl", "co2_sda"):
            pin.setdefault(key, -1)
