from enum import Enum
from typing import Union

import serial


class FbibxxIoBoardError(Exception):
    pass


class RemoteError(FbibxxIoBoardError):
    pass


class InvalidTerminalNo(FbibxxIoBoardError):
    pass


class ResponseTimeout(FbibxxIoBoardError):
    pass


class BadResponse(FbibxxIoBoardError):
    pass


class Unsupported(FbibxxIoBoardError):
    pass


class IoDirection(Enum):
    INPUT = "in"
    OUTPUT = "out"


class FbibxxIoBoard:
    SERIAL_TIMEOUT_S = 1.

    Error = FbibxxIoBoardError
    InvalidTerminalNo = InvalidTerminalNo
    ResponseTimeout = ResponseTimeout
    Unsupported = Unsupported
    RemoteError = RemoteError
    BadResponse = BadResponse
    IoDirection = IoDirection

    _direction_codes = [
        (0, IoDirection.INPUT),
        (1, IoDirection.OUTPUT),
        ]

    def __init__(self, ser_port: serial.Serial):
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
        ser = serial.Serial(portname, timeout=cls.SERIAL_TIMEOUT_S)
        return cls(ser)

    def _get_term_nos(self):
        response = self._ask("TLS").split(",")
        try:
            term_nos = [int(x) for x in response]
        except ValueError:
            raise BadResponse(response)
        return term_nos

    def _read_response(self):
        response = self._ser_port.readline().decode('ascii')
        if not response.endswith("\n"):
            raise self.ResponseTimeout()
        response = response.strip()
        if response.startswith("ERROR"):
            raise RemoteError(response)
        return response

    def _ask(self, cmd: str):
        self._ser_port.write(f"{cmd}?\r".encode('ascii'))
        response = self._read_response()
        if "=" not in response:
            raise BadResponse(response)
        return response.rsplit("=", 1)[-1]

    def _tell(self, cmd: str):
        self._ser_port.write(f"{cmd}\r".encode('ascii'))
        response = self._read_response()
        if response != "OK":
            raise BadResponse(response)

    def _check_output_ok(self, n: int):
        if n not in self._terminal_capabilities:
            raise InvalidTerminalNo(n)
        if n not in self.output_nos:
            raise Unsupported(f"Terminal {n} does not have output capability")

    def _check_input_ok(self, n: int):
        if n not in self._terminal_capabilities:
            raise InvalidTerminalNo(n)
        if n not in self.input_nos:
            raise Unsupported(f"Terminal {n} does not have input capability")

    def _check_dirchange_ok(self, n: int):
        if n not in self._terminal_capabilities:
            raise InvalidTerminalNo(n)
        if n not in self.input_nos or n not in self.output_nos:
            raise Unsupported(
                f"Terminal {n} does not support changing I/O direction")

    def get_input(self, n: int):
        if n not in self._terminal_capabilities:
            raise InvalidTerminalNo(n)
        if n not in self.input_nos:
            raise Unsupported(f"Terminal {n} does not have input capability")
        answer = self._ask(f"INP:{n}")
        return(bool(int(answer)))

    def get_output(self, n: int):
        self._check_output_ok(n)
        answer = self._ask(f"OUT:{n}")
        return(bool(int(answer)))

    def set_output(self, n: int, on: Union[int, bool]):
        self._check_output_ok(n)
        self._tell(f"OUT:{n}={int(bool(on))}")

    def get_direction(self, n: int):
        if n not in self._terminal_capabilities:
            raise InvalidTerminalNo(n)
        response = self._ask(f"DIR:{n}")
        try:
            dir_int = int(response)
            return dict(self._direction_codes)[dir_int]
        except (ValueError, KeyError):
            raise BadResponse(response)

    def set_direction(
            self, n: int,
            direction: Union[str, IoDirection]  # type: ignore[valid-type]
            ):
        self._check_dirchange_ok(n)
        direction = self.IoDirection(direction)
        dir_code = dict((y, x) for (x, y) in self._direction_codes)[direction]
        self._tell(f"DIR:{n}={dir_code}")

    @property
    def board_model(self):
        return self._board_model

    @property
    def board_id(self):
        return self._board_id

    @property
    def terminal_nos(self):
        return list(self._terminal_capabilities.keys())

    @property
    def input_nos(self):
        return [
            term_no for (term_no, caps) in self._terminal_capabilities.items()
            if "I" in caps
            ]

    @property
    def output_nos(self):
        return [
            term_no for (term_no, caps) in self._terminal_capabilities.items()
            if "O" in caps
            ]


if __name__ == "__main__":
    board = FbibxxIoBoard.from_serial_portname("/dev/ttyACM0")
    print("model:", board.board_model)
    print("id:", board.board_id)
    print("terminals:", board.terminal_nos)
    print("inputs:", board.input_nos)
    print("outputs:", board.output_nos)

    print("setting terminal 13 to input")
    board.set_direction(13, board.IoDirection.OUTPUT)
    print("setting terminal 14 to output")
    board.set_direction(14, board.IoDirection.OUTPUT)
    print("wiggling")
    while True:
        board.set_output(14, True)
        board.set_output(14, False)
