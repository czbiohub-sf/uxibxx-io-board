from enum import Enum
from typing import List, Optional, Tuple, Union

import serial
import serial.tools.list_ports

from . import types


class UxibxxIoBoard:
    """
    Communications driver class for controlling a UXIBxx I/O board such as
    UXIB-DN12.
    """
    SERIAL_TIMEOUT_S = 1.
    USB_HW_IDS = {
        (0x4743, 0xB499),
        }

    UxibxxIoBoardError = types.UxibxxIoBoardError
    DeviceNotFound = types.DeviceNotFound
    IdMismatch = types.IdMismatch
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

    def __init__(self, ser_port: serial.Serial,
                 board_model: Optional[str] = None,
                 board_id: Optional[str] = None):
        """
        :param ser_port: a ``serial.Serial`` instance that will be used to
            communicate with the hardware
        :param board_model: If not ``None``, this will be checked against the
            board model string reported by the hardware and :exc:`IdMismatch`
            will be raised in case of a mismatch
        :param board_id: Same as ``board_model`` but for the board ID string
        """
        if hasattr(ser_port, 'timeout'):
            ser_port.timeout = self.SERIAL_TIMEOUT_S
        self._ser_port = ser_port
        portname = self._ser_port.port
        self._board_model, self._board_id = self._ask("IDN").split(",")
        for (desc, expected, actual) in [
                ("board model", board_model, self.board_model),
                ("board ID", board_id, self.board_id),
                ]:
            if expected is None:
                continue
            if expected != actual:
                raise self.IdMismatch(
                    f"Wrong {desc} (expected {expected!r}, "
                    f"board reported {actual!r}) for device on "
                    f"port {portname!r}"
                    )
        term_nos = self._get_term_nos()
        self._terminal_capabilities = {
            term_no: self._ask(f"TCP:{term_no}") for term_no in term_nos
            }

    @classmethod
    def list_connected_devices(
            cls, usb_vidpid: Optional[Tuple[int, int]] = None
            ) -> List[Tuple[str, str]]:
        """
        Get a list of all connected UXIBxx devices. Detection is based on the
        USB vendor ID, product ID and serial number descriptors reported by the
        OS. This method does not attempt to open the devices or verify that
        they are actually accessible.

        Only supported on Windows, macOS and Linux.

        :param usb_vidpid: A tuple ``(vid, pid)`` specifying a particular
            USB vendor and product ID to look for instead of using the default
            list of IDs.
        :returns: A list of tuples ``(portname, board_id)`` where ``portname``
            is a string used by pySerial to identify the port and ``board_id``
            is the board ID string reported by the hardware.
        """
        usb_vidpids = (
            [tuple(usb_vidpid)] if usb_vidpid is not None
            else cls.USB_HW_IDS
            )
        return [
            (info.device, info.serial_number)
            for info in serial.tools.list_ports.comports()
            if (info.vid, info.pid) in usb_vidpids
            ]

    @classmethod
    def _select_and_open(
            cls,
            usb_vidpid: Optional[Tuple[int, int]] = None,
            board_model: Optional[str] = None,
            board_id: Optional[str] = None):
        for portname, board_id_ in cls.list_connected_devices(
                usb_vidpid=usb_vidpid):
            if board_id is not None and board_id != board_id_:
                continue
            return cls.from_serial_portname(
                portname, board_model=board_model, board_id=board_id)
        filter_desc = ", ".join(
            f"{name}={value!r}"
            for name, value in [
                ('usb_vidpid', usb_vidpid),
                ('board_id', board_id)
                ]
            if value is not None
            )
        raise cls.DeviceNotFound(
            "No device(s) found"
            + (f" matching {filter_desc}" if filter_desc else "")
            )

    @classmethod
    def open_first_device(cls, *args, **kwargs) -> 'UxibxxIoBoard':
        """
        Opens the first UXIBxx device found. Intended for convenience in
        situations where only one device is connected.

        :param args: positional arguments to pass to :meth:`__init__`
        :param kwargs: keyword arguments to pass to :meth:`__init__`
        :raises DeviceNotFound: If no matching devices were found
        :raises serial.SerialException: If something went wrong opening the
            serial device
        """
        return cls._select_and_open(*args, **kwargs)

    @classmethod
    def from_board_id(cls, board_id: str, *args, **kwargs) -> 'UxibxxIoBoard':
        """
        Finds and opens the UXIBxx device matching the specified board ID,
        if present.

        :param board_id: board ID string of the hardware to connect to
        :param args: positional arguments to pass to :meth:`__init__`
        :param kwargs: keyword arguments to pass to :meth:`__init__`
        :returns: New :class:`UxibxxIoBoard` instance
        :raises DeviceNotFound: If no matching devices were found
        :raises serial.SerialException: If something went wrong opening the
            serial device
        """
        return cls._select_and_open(board_id=board_id, *args, **kwargs)

    @classmethod
    def from_serial_portname(cls, portname: str, *args, **kwargs):
        """
        Opens and configures the serial port identified by ``portname`` and
        uses it to initialize a new :class:`UxibxxIoBoard` instance.

        :param portname: Port name or URL to pass to ``serial.Serial()``
        :param args: positional arguments to pass to :meth:`__init__`
        :param kwargs: keyword arguments to pass to :meth:`__init__`
        :returns: New :class:`UxibxxIoBoard` instance
        :raises serial.SerialException: If something went wrong opening the
            serial device
        """
        ser = serial.Serial(portname, timeout=cls.SERIAL_TIMEOUT_S)
        return cls(ser, *args, **kwargs)

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
            direction: types._IoDirectionOrLiteral
            ):
        """
        Sets the I/O direction of the specified terminal.
        See :class:`IoDirection` for further explanation.

        :param n: Terminal number
        :param direction: I/O direction to set the terminal to, either a
            :class:`IoDirection` member or the corresponding string literal.
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
        The board model name reported by the hardware, e.g. ``"UXIB-DN12"``
        """
        return self._board_model

    @property
    def board_id(self) -> str:
        """
        The board ID string reported by the hardware, e.g. ``"4E0101"``. This
        is the same as the text of the USB serial number descriptor.
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
        A list of the terminal numbers for terminals that are inputs or support
        being set to input mode.
        """
        return [
            term_no for (term_no, caps) in self._terminal_capabilities.items()
            if "I" in caps
            ]

    @property
    def output_nos(self) -> List[int]:
        """
        A list of the terminal numbers for terminals that are outputs or
        support being set to output mode.
        """
        return [
            term_no for (term_no, caps) in self._terminal_capabilities.items()
            if "O" in caps
            ]
