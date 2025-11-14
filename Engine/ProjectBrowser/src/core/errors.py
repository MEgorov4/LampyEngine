__all__ = [
    "ProjectError",
    "ProjectNameError",
    "JsonProjectError",
    "StructureProjectError",
    "FileExtensionError",
    "FileIOProjectError",
    "PermissionDeniedError",
]


class ProjectError(Exception):
    def __init__(self, message: str, fatal: bool = True) -> None:
        super().__init__(message)
        self.user_message = message
        self.fatal = fatal


class ProjectNameError(ProjectError):
    pass


class JsonProjectError(ProjectError):
    pass


class StructureProjectError(ProjectError):
    pass


class FileExtensionError(ProjectError):
    pass


class FileIOProjectError(ProjectError):
    pass


class PermissionDeniedError(ProjectError):
    def __init__(self, message: str, fatal: bool = False) -> None:
        super().__init__(message, fatal=fatal)
