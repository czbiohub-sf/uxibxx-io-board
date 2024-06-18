``uxibxx`` module
=========================

Driver class
------------
.. autoclass:: uxibxx.UxibxxIoBoard
   :members: __init__, list_connected_devices, open_first_device, from_serial_portname, get_direction, set_direction, get_input, get_output, set_output, board_model, board_id, terminal_nos, input_nos, output_nos
   :member-order: bysource

Enums
-----
.. autoclass:: uxibxx.UxibxxIoBoard.IoDirection
   :members:

Exceptions
----------
.. autoclass:: uxibxx.UxibxxIoBoard.UxibxxIoBoardError
.. autoclass:: uxibxx.UxibxxIoBoard.DeviceNotFound
   :show-inheritance: True
.. autoclass:: uxibxx.UxibxxIoBoard.IdMismatch
   :show-inheritance: True
.. autoclass:: uxibxx.UxibxxIoBoard.InvalidTerminalNo
   :show-inheritance: True
.. autoclass:: uxibxx.UxibxxIoBoard.Unsupported
   :show-inheritance: True
.. autoclass:: uxibxx.UxibxxIoBoard.RemoteError
   :show-inheritance: True
.. autoclass:: uxibxx.UxibxxIoBoard.ResponseTimeout
   :show-inheritance: True
.. autoclass:: uxibxx.UxibxxIoBoard.BadResponse
   :show-inheritance: True

Usage example
-------------
.. literalinclude:: usage_example.py
   :language: python
