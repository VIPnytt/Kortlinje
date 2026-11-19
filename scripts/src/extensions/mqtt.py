import typing

if typing.TYPE_CHECKING:
    from ..kortlinje import Kortlinje
else:
    pass


class Mqtt:
    NAME: typing.Literal["MQTT"] = "MQTT"
    project: "Kortlinje"
    auth: str | None

    def __init__(self, project: "Kortlinje") -> None:
        self.auth = None
        self.project = project

    def configure(self) -> None:
        mqtt = self.project.toml.setdefault("mqtt", {})
        for key in ("host", "username", "password"):
            mqtt.setdefault(key, "")
