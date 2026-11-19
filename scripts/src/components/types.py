import collections.abc
import pathlib
import typing

BUILD_TARGETS = globals().get("BUILD_TARGETS", [])
COMMAND_LINE_TARGETS = globals().get("COMMAND_LINE_TARGETS", [])


class Action:
    """SCons executable action wrapper."""

    list: list[typing.Any]


class Executor:
    """SCons target action executor."""

    action_list: list[Action]


class BoardConfig:
    def get(self, key: str, default: typing.Any = None) -> typing.Any:
        """Declare board configuration lookup; this placeholder raises NotImplementedError."""
        raise NotImplementedError

    def update(self, key: str, value: typing.Any) -> None:
        raise NotImplementedError


class PioPlatform:
    """PlatformIO platform manager interface."""

    packages: dict[str, dict[str, typing.Any]]

    def get_package_dir(self, name: str) -> str | None:
        """Return the filesystem directory path for an installed PlatformIO package.

        Parameters:
            name (str): The package identifier (e.g., 'tool-picotool-rp2040-earlephilhower').

        Returns:
            str | None: The resolved package directory path, or None if unavailable.
        """
        raise NotImplementedError


class ProjectConfig:
    def get(self, section: str, option: str, default: typing.Any = None) -> typing.Any:
        """Declare project option lookup within a section; this placeholder raises NotImplementedError."""
        raise NotImplementedError


class Node:
    """SCons directed acyclic graph (DAG) filesystem node."""

    def exists(self) -> bool:
        """Check whether the node exists on the filesystem."""
        raise NotImplementedError

    def get_executor(self) -> Executor:
        """Return the action executor bound to this node."""
        raise NotImplementedError

    def get_path(self) -> str:
        """Return the path string representation of the node."""
        raise NotImplementedError


class Environment:
    def __getitem__(self, key: str) -> str:
        """Retrieve an environment value by key.

        Returns:
            str: The value associated with the key
        """
        raise NotImplementedError

    def AddPostAction(
        self,
        target: str | Node | collections.abc.Sequence[str | Node],
        action: str | Action | collections.abc.Callable[[list[str], list[str], "Environment"], int | None],
    ) -> None:
        """Register an action to run after the specified build target completes.

        Parameters:
            target (str | Node | collections.abc.Sequence[str | Node]): The build target associated with the action.
            action (collections.abc.Callable[[list[str], list[str], Environment], int | None]): A callback receiving the target name, command arguments, and environment.
        """
        raise NotImplementedError

    def AddPreAction(
        self,
        target: str | Node | collections.abc.Sequence[str | Node],
        action: str | Action | collections.abc.Callable[[list[str], list[str], "Environment"], int | None],
    ) -> None:
        """Register an action to run before the specified build target executes.

        Parameters:
            target (str | Node | collections.abc.Sequence[str | Node]): The build target associated with the action.
            action (str | Action | collections.abc.Callable[[list[str], list[str], Environment], int | None]): A command, SCons Action, or callback receiving the target name, command arguments, and environment.
        """
        raise NotImplementedError

    def Alias(
        self,
        alias: str,
        source: str | Node | collections.abc.Sequence[str | Node] | None = None,
        action: typing.Any = None,
    ) -> list[Node]:
        """Create or lookup one or more SCons Alias nodes.

        Parameters:
            alias (str): The alias target name (e.g., 'upload').
            source (str | Node | collections.abc.Sequence[str | Node] | None): Optional prerequisite targets.
            action (typing.Any): Optional action(s) attached to the alias.

        Returns:
            list[Node]: The list of Alias nodes matching the identifier.
        """
        raise NotImplementedError

    def Append(self, **kwargs: typing.Any) -> None:
        """Appends values to environment variables.

        Parameters:
            **kwargs (typing.Any): Variable names mapped to values to append.
        """
        raise NotImplementedError

    def BoardConfig(self) -> BoardConfig:
        raise NotImplementedError

    def Depends(
        self,
        target: str | Node | collections.abc.Sequence[str | Node],
        dependency: str | Node | collections.abc.Sequence[str | Node],
    ) -> list[Node]:
        """Explicitly declare a dependency between build targets and prerequisites.

        Parameters:
            target (str | Node | collections.abc.Sequence[str | Node]): Target node(s) to invalidate when dependencies change.
            dependency (str | Node | collections.abc.Sequence[str | Node]): Prerequisite file node(s).

        Returns:
            list[Node]: The list of target nodes associated with this dependency.
        """
        raise NotImplementedError

    def Dir(self, name: str | pathlib.Path) -> Node:
        """Bind or lookup a Directory node in SCons's filesystem graph.

        Parameters:
            name (str | pathlib.Path): Relative or project-root anchored ('#') directory path.

        Returns:
            Node: The resolved directory node.
        """
        raise NotImplementedError

    def File(self, name: str | pathlib.Path) -> Node:
        """Bind or lookup a File node in SCons's filesystem graph.

        Parameters:
            name (str | pathlib.Path): Relative or project-root anchored ('#') file path.

        Returns:
            Node: The resolved file node.
        """
        raise NotImplementedError

    def GetProjectConfig(self) -> ProjectConfig:
        raise NotImplementedError

    def GetProjectOption(self, option: str, default: typing.Any = None) -> typing.Any:
        raise NotImplementedError

    def IsCleanTarget(self) -> bool:
        raise NotImplementedError

    def PioPlatform(self) -> PioPlatform:
        """Return the active PlatformIO platform instance.

        Returns:
            PioPlatform: The platform instance for the current build environment.
        """
        raise NotImplementedError

    def Replace(self, **kwargs: typing.Any) -> None:
        raise NotImplementedError

    def StringifyMacro(self, value: str) -> str:
        raise NotImplementedError

    def subst(self, key: str) -> str:
        raise NotImplementedError

    def VerboseAction(
        self,
        action: collections.abc.Callable[[list[typing.Any], list[typing.Any], "Environment"], typing.Any] | str,
        actstr: str,
    ) -> typing.Any:
        """Wrap an SCons action with a custom summary message when verbose mode is off.

        Parameters:
            action (collections.abc.Callable | str): The callback or shell command to execute.
            actstr (str): The status string displayed in the terminal.

        Returns:
            typing.Any: The wrapped SCons Action object.
        """
        raise NotImplementedError


def Import(*vars: str) -> None:
    raise NotImplementedError
