import subprocess
import sys
import typing

if typing.TYPE_CHECKING:
    from .src.components.types import COMMAND_LINE_TARGETS, Environment, Import
else:
    from SCons.Script import COMMAND_LINE_TARGETS, Environment, Import


def main() -> None:
    """
    Run the Kortlinje build workflow for the current SCons environment.

    The function installs the locked scripts dependencies into the running
    interpreter, cleans the project for clean targets, and otherwise executes
    the Kortlinje workflow. It returns without action for the ``erase``,
    ``menuconfig``, and ``size`` targets.
    """
    if not {
        "erase",
        "size",
    }.isdisjoint(COMMAND_LINE_TARGETS):
        return

    if sys.version_info < (3, 13):
        sys.exit(
            f"Kortlinje requires Python 3.13 or newer, but PlatformIO runs on Python {sys.version.split()[0]}.\n"
            "Install Python 3.13+, disable the `platformio-ide.useBuiltinPython` setting in VS Code, "
            f"delete {sys.prefix} and restart VS Code to let PlatformIO recreate it."
        )

    Import("env")
    env: Environment = typing.cast(Environment, globals()["env"])

    # Install into PlatformIO's own interpreter rather than syncing a project environment: `uv sync`
    # recreates the environment when its Python doesn't satisfy `requires-python`, wiping PlatformIO.
    uv = subprocess.run(
        [
            sys.executable,
            "-m",
            "uv",
            "export",
            "--frozen",
            "--only-group",
            "scripts",
            "--no-annotate",
            "--no-emit-project",
            "--no-hashes",
            "--no-header",
        ],
        capture_output=True,
        check=False,
        text=True,
    )
    if uv.returncode:
        pip = subprocess.run(
            [sys.executable, "-m", "pip", "install", "uv"],
            stderr=subprocess.DEVNULL,
            check=False,
        )
        if pip.returncode:
            subprocess.run([sys.executable, "-m", "ensurepip"], check=True)
            subprocess.run(pip.args, check=True)
        uv = subprocess.run(uv.args, capture_output=True, check=True, text=True)
    subprocess.run(
        [sys.executable, "-m", "uv", "pip", "install", "--quiet", "--python", sys.executable, "--requirements", "-"],
        check=True,
        input=uv.stdout,
        text=True,
    )

    from src.kortlinje import Kortlinje

    if env.IsCleanTarget():
        Kortlinje.clean()
        return

    Kortlinje(env).run()


if __name__ == "SCons.Script":
    main()
