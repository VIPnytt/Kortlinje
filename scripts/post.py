import typing

from src.components.board import Board

if typing.TYPE_CHECKING:
    from .src.components.types import Environment, Import
else:
    from SCons.Script import Environment, Import


def main() -> None:
    Import("env")
    env: Environment = typing.cast(Environment, globals()["env"])
    Board(env).post()


if __name__ == "SCons.Script":
    main()
