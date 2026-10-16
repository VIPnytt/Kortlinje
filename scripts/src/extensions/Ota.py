import hashlib
import typing

if typing.TYPE_CHECKING:
    from ..components.Types import COMMAND_LINE_TARGETS
    from ..Kortlinje import Kortlinje
else:
    from SCons.Script import COMMAND_LINE_TARGETS


class Ota:
    NAME: typing.Final[str] = "OTA"
    project: "Kortlinje"
    auth: str | None

    def __init__(self, project: "Kortlinje") -> None:
        """Initialize the OTA manager with a project and no authentication key."""
        self.auth = None
        self.project = project

    def configure(self) -> None:
        """
        Configure the OTA authentication key for project use.

        Stores the original key for upload authentication and replaces the project key with its SHA-256 hexadecimal digest when it is not already a 64-character hash.
        """
        if "OTA_KEY" in self.project.dotenv and len(self.project.dotenv["OTA_KEY"]) != 64:
            self.auth = self.project.dotenv["OTA_KEY"]
            self.project.dotenv["OTA_KEY"] = hashlib.sha256(self.project.dotenv["OTA_KEY"].encode()).hexdigest()

    def finalize(self) -> None:
        """
        Configure OTA upload settings, including the protocol, host-based port, and authentication flags.
        """
        protocol = self.project.env.GetProjectOption("upload_protocol", None)
        if protocol != "espota" and any(target.endswith("ota") for target in COMMAND_LINE_TARGETS):
            protocol = "espota"
            self.project.env.Replace(UPLOAD_PROTOCOL=protocol)
        if (
            protocol == "espota"
            and not self.project.env.GetProjectOption("upload_flags", None)
            and not self.project.env.GetProjectOption("upload_port", None)
        ):
            self.project.env.Replace(UPLOAD_PORT=f"{self.project.dotenv['HOSTNAME']}.local")
            if self.auth:
                self.project.env.Append(
                    UPLOAD_FLAGS=[
                        f"-a={self.auth}",
                    ]
                )
