from enum import Enum


class UxibxxIoBoardError(Exception):
    """
    Base class of all UxibxxIoBoard errors
    """
    pass


class RemoteError(UxibxxIoBoardError):
    """
    The UXIBxx board reported an error in response to a command.
    """
    pass


class InvalidTerminalNo(UxibxxIoBoardError, ValueError):
    """
    The specified terminal number doesn't exist for the connected hardware.
    """
    pass


class ResponseTimeout(UxibxxIoBoardError):
    """
    A reponse line was not received within the prescribed time limit.
    """
    pass


class BadResponse(UxibxxIoBoardError):
    """
    The response from the hardware didn't align with the expected format.

    The most common situation where this is encountered is when accidentally
    connecting to a device that is not a UXIBxx board or is running an
    incompatible firmware.
    """
    pass


class Unsupported(UxibxxIoBoardError):
    """
    The specified terminal doesn't support the requested functionality -- for
    example, when attempting to set an output-only terminal to input mode.
    """
    pass


class IoDirection(Enum):
    """
    Identifies whether a given terminal logically acts as an "input" or
    an "output". The exact consequences in terms of electrical behavior depend
    on the hardware.

    For UXIB-DN12, terminals 13 and 14 (broken out on the extra headers on the
    bottoom of the board) are set to high-impedance in input mode and
    push-pull in output mode. Terminals 1-12 are open-collector outputs and
    only support output mode.
    """

    # Terminal acts as an input
    INPUT = "in"

    # Terminal acts as an output
    OUTPUT = "out"
