import os
import shutil
import typing

if typing.TYPE_CHECKING:
    from .Kortlinje import Kortlinje


class Tools:
    NAME: typing.Final[str] = "Tools"
    project: "Kortlinje"

    def __init__(self, project: "Kortlinje") -> None:
        """Initialize the tools with a project instance.

        Parameters:
            project (Kortlinje): Project instance associated with the tools.
        """
        self.project = project

    @staticmethod
    def clean() -> None:
        """Remove the tools package's Python cache directory when it exists."""
        path = "tools/__pycache__"
        if os.path.exists(path):
            shutil.rmtree(path, ignore_errors=True)
            print(f"Removing {path}")
