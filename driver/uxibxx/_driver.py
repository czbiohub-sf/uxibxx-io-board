from enum import Enum
from typing import List, Union

import serial

from . import types


class UxibxxIoBoard:
    """
    Communications driver class for controlling a UXIBxx I/O board such as
    UXIB-DN12.
    """
    SERIAL_TIMEOUT_S = 1.

    Error = types.UxibxxIoBoardError
    InvalidTerminalNo = types.InvalidTerminalNo
    ResponseTimeout = types.ResponseTimeout
    Unsupported = types.Unsupported
    RemoteError = types.RemoteError
    BadResponse = types.BadResponse

    IoDirection = types.IoDirection

    _direction_codes = [
        (0, IoDirection.INPUT),
        (1, IoDirection.OUTPUT),
        ]

    def __init__(self, ser_port: serial.Serial):
        """
        :param ser_port: a ``serial.Serial`` instance (or other object with a
            compatible interface) that will be used to communicate with the
            hardware
        """
        # TODO add option to verify matching board model and/or ID
        if hasattr(ser_port, 'timeout'):
            ser_port.timeout = self.SERIAL_TIMEOUT_S
        self._ser_port = ser_port
        self._board_model, self._board_id = self._ask("IDN").split(",")
        term_nos = self._get_term_nos()
        self._terminal_capabilities = {
            term_no: self._ask(f"TCP:{term_no}") for term_no in term_nos
            }

    @classmethod
    def from_serial_portname(cls, portname: str):
        """
        Open the specified serial port and initialize a new
        :class:`UxibxxIoBoard`.

        :param portname: Port name or URL to pass to ``serial.Serial()``
        :returns: New :class:`UxibxxIoBoard` instance
        :raises serial.SerialException: If something went wrong opening the
            serial device
        """
        ser = serial.Serial(portname, timeout=cls.SERIAL_TIMEOUT_S)
        return cls(ser)

    def _get_term_nos(self):
        response = self._ask("TLS").split(",")
        try:
            term_nos = [int(x) for x in response]
        except ValueError:
            raise self.BadResponse(response)
        return term_nos

    def _read_response(self):
        response = self._ser_port.readline().decode('ascii')
        if not response.endswith("\n"):
            raise self.ResponseTimeout()
        response = response.strip()
        if response.startswith("ERROR"):
            raise self.RemoteError(response)
        return response

    def _ask(self, cmd: str):
        self._ser_port.write(f"{cmd}?\r".encode('ascii'))
        response = self._read_response()
        if "=" not in response:
            raise self.BadResponse(response)
        return response.rsplit("=", 1)[-1]

    def _tell(self, cmd: str):
        self._ser_port.write(f"{cmd}\r".encode('ascii'))
        response = self._read_response()
        if response != "OK":
            raise self.BadResponse(response)

    def _check_output_ok(self, n: int):
        if n not in self._terminal_capabilities:
            raise self.InvalidTerminalNo(n)
        if n not in self.output_nos:
            raise self.Unsupported(
                f"Terminal {n} does not have output capability")

    def _check_input_ok(self, n: int):
        if n not in self._terminal_capabilities:
            raise self.InvalidTerminalNo(n)
        if n not in self.input_nos:
            raise self.Unsupported(
                f"Terminal {n} does not have input capability")

    def _check_dirchange_ok(self, n: int):
        if n not in self._terminal_capabilities:
            raise self.InvalidTerminalNo(n)
        if n not in self.input_nos or n not in self.output_nos:
            raise self.Unsupported(
                f"Terminal {n} does not support changing I/O direction")

    def get_input(self, n: int):
        """
        Reads out the current input state of the specified terminal.

        Inputs can be *active* or *inactive*; what this means electrically
        is hardware-dependent, but the convention for logic-level inputs
        is that *active* corresponds high logic level.

        :param n: Terminal number
        :returns: `True` if the input is active, otherwise `False`.
        :raises InvalidTerminalNo: if the specified terminal number is invalid
        :raises Unsupported: if the specified terminal does not have input
            capability
        :raises ResponseTimeout,RemoteError,BadResponse: see class descriptions
        """
        if n not in self._terminal_capabilities:
            raise self.InvalidTerminalNo(n)
        if n not in self.input_nos:
            raise self.Unsupported(
                f"Terminal {n} does not have input capability")
        answer = self._ask(f"INP:{n}")
        return bool(int(answer))

    def get_output(self, n: int):
        """
        Reads out the current output state of the specified terminal.

        :param n: Terminal number
        :returns: `True` if the output is active, otherwise `False`.
            See note on :meth:`set_output`.
        :raises InvalidTerminalNo: if the specified terminal number is invalid
        :raises Unsupported: if the specified terminal does not have output
            capability
        :raises ResponseTimeout,RemoteError,BadResponse: see class descriptions
        """
        self._check_output_ok(n)
        answer = self._ask(f"OUT:{n}")
        return bool(int(answer))

    def set_output(self, n: int, on: Union[int, bool]):
        """
        Sets the output state of the specified terminal.

        Outputs can be *active* or *inactive*; what this means electrically
        is hardware-dependent; however, the convention for logic-level outputs
        is that *active* corresponds to logic high, and for power outputs
        *active* means the load is energized.

        :param n: Terminal number
        :param on: New output state. If `False` or zero the output becomes
            inactive. If `True` or nonzero the output becomes active.
        :raises InvalidTerminalNo: if the specified terminal number is invalid
        :raises Unsupported: if the specified terminal does not have output
            capability
        :raises ResponseTimeout,RemoteError,BadResponse: see class descriptions
        """
        self._check_output_ok(n)
        self._tell(f"OUT:{n}={int(bool(on))}")

    def get_direction(self, n: int) -> 'UxibxxIoBoard.IoDirection':
        """
        Reads out the current I/O direction of the specified terminal

        :param n: Terminal number
        :returns: The current I/O direction of the specified terminal
        :raises InvalidTerminalNo: if the specified terminal number is invalid
        :raises ResponseTimeout,RemoteError,BadResponse: see class descriptions
        """
        if n not in self._terminal_capabilities:
            raise self.InvalidTerminalNo(n)
        response = self._ask(f"DIR:{n}")
        try:
            dir_int = int(response)
            return dict(self._direction_codes)[dir_int]
        except (ValueError, KeyError):
            raise self.BadResponse(response)

    def set_direction(
            self, n: int,
            direction: Union[str, IoDirection]  # type: ignore[valid-type]
            ):
        """
        Sets the I/O direction of the specified terminal.

        :param n: Terminal number
        :param direction: I/O direction to set the terminal to
        :raises InvalidTerminalNo: if the specified terminal number is invalid
        :raises Unsupported: if the specified terminal does not support the
            requested mode.
        :raises ResponseTimeout,RemoteError,BadResponse: see class descriptions
        """
        self._check_dirchange_ok(n)
        direction = self.IoDirection(direction)
        dir_code = dict((y, x) for (x, y) in self._direction_codes)[direction]
        self._tell(f"DIR:{n}={dir_code}")

    @property
    def board_model(self) -> str:
        """
        The board model name reported by the hardware, e.g. "UXIB-DN12"
        """
        return self._board_model

    @property
    def board_id(self) -> str:
        """
        The board ID string reported by the hardware, e.g. "4E0101". This is
        the same as the text of the USB serial number descriptor.
        """
        return self._board_id

    @property
    def terminal_nos(self) -> List[int]:
        """
        A list of valid terminal numbers. Terminals are normally (but not
        strictly necessarily) numbered consecutively starting from 1.
        """
        return list(self._terminal_capabilities.keys())

    @property
    def input_nos(self) -> List[int]:
        """
        A list of the terminal numbers for terminals that support being set to
        input mode.
        """
        return [
            term_no for (term_no, caps) in self._terminal_capabilities.items()
            if "I" in caps
            ]

    @property
    def output_nos(self) -> List[int]:
        """
        A list of the terminal numbers for terminals that support being set to
        output mode.
        """
        return [
            term_no for (term_no, caps) in self._terminal_capabilities.items()
            if "O" in caps
            ]
