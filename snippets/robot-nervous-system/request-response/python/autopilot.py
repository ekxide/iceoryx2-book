"""Request-response payload types."""

# snippet:start request-type
import ctypes


class Position(ctypes.Structure):
    _fields_ = [
        ("position", ctypes.c_float * 2),
    ]
# snippet:end request-type


# snippet:start response-type
class State(ctypes.Structure):
    _fields_ = [
        ("position", ctypes.c_float * 2),
        ("speed", ctypes.c_float * 2),
    ]
# snippet:end response-type
