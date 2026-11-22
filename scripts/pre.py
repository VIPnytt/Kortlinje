import importlib
import importlib.util
import os
import pathlib
import subprocess
import sys
import tomllib
import typing

if typing.TYPE_CHECKING:
    from .src.components.types import COMMAND_LINE_TARGETS, Environment, Import
else:
    from SCons.Script import COMMAND_LINE_TARGETS, Environment, Import


def main() -> None:
    """
    Run the Kortlinje build workflow for the current SCons environment.

    The function synchronizes the scripts dependencies, cleans the project for
    clean targets, and otherwise executes the Kortlinje workflow. It returns
    without action for the ``erase``, ``menuconfig``, and ``size`` targets.
    """
    if not {
        "erase",
        "size",
    }.isdisjoint(COMMAND_LINE_TARGETS):
        return
    requires = tomllib.loads(pathlib.Path("pyproject.toml").read_text(encoding="utf-8"))["project"]["requires-python"]
    if sys.version_info < tuple(map(int, requires.removeprefix(">=").split("."))):
        sys.exit(f"Python {sys.version.split()[0]} is incompatible (requires {requires}).")
    Import("env")
    env: Environment = typing.cast(Environment, globals()["env"])
    if importlib.util.find_spec("uv") is None:
        if importlib.util.find_spec("pip") is None:
            subprocess.run([sys.executable, "-m", "ensurepip"], check=True)
        subprocess.run([sys.executable, "-m", "pip", "install", "uv"], check=True)
    subprocess.run(
        [
            sys.executable,
            "-m",
            "uv",
            "sync",
            "--active",
            "--frozen",
            "--inexact",
            "--no-python-downloads",
            "--only-group",
            "scripts",
            "--python",
            f"{sys.version_info.major}.{sys.version_info.minor}",
        ],
        env={**os.environ, "VIRTUAL_ENV": sys.prefix} if sys.prefix != sys.base_prefix else None,
        check=True,
    )
    importlib.invalidate_caches()
    from src.kortlinje import Kortlinje

    if env.IsCleanTarget():
        Kortlinje.clean()
        return
    Kortlinje(env).run()


if __name__ == "SCons.Script":
    main()
