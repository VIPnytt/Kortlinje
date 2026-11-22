import contextlib
import http
import http.client
import logging
import typing
import urllib.parse

import packaging.version

from ..config.version import VERSION

if typing.TYPE_CHECKING:
    from ..kortlinje import Kortlinje
    from .types import COMMAND_LINE_TARGETS
else:
    from SCons.Script import COMMAND_LINE_TARGETS


class Dependency:
    project: "Kortlinje"

    def __init__(self, project: "Kortlinje") -> None:
        """Initialize the dependency with its associated Kortlinje project."""
        self.project = project

    def initialize(self) -> None:
        """
        Check GitHub for a newer Kortlinje release and display its release notes URL when available.

        The update check is skipped for the ``compiledb`` command-line target.
        """
        if COMMAND_LINE_TARGETS == ["compiledb"]:
            return
        try:
            with contextlib.closing(http.client.HTTPSConnection("github.com", timeout=5)) as conn:
                conn.request(
                    "HEAD",
                    "/VIPnytt/Kortlinje/releases/latest",
                    headers={"User-Agent": f"Kortlinje/{VERSION} (+https://github.com/VIPnytt/Kortlinje)"},
                )
                response = conn.getresponse()
                if response.status != http.HTTPStatus.FOUND or not (location := response.getheader("Location")):
                    return
            _, _, tag = urllib.parse.urlparse(location).path.partition("/releases/tag/")
            tag = tag.strip("/")
            if not tag:
                return
            local = packaging.version.parse(VERSION)
            latest = packaging.version.parse(urllib.parse.unquote(tag))
            if latest > local:
                print(
                    f"\033[93m[notice] A new release of Kortlinje is available: {local.public} -> {latest.public}\033[0m"
                )
                print(f"Release notes: https://github.com/VIPnytt/Kortlinje/releases/tag/{tag}")
        except (OSError, http.client.HTTPException, packaging.version.InvalidVersion) as e:
            logging.getLogger(__name__).debug("Update check failed: %s", e)
