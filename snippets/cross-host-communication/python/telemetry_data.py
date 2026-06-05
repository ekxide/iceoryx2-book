"""Tunnel snippet payload types."""

# snippet:start
import ctypes

class BatteryState(ctypes.Structure):
    """Battery telemetry payload."""

    _fields_ = [("charge_percent", ctypes.c_float)]

    # shared type name; must match across languages
    @staticmethod
    def type_name() -> str:
        return "BatteryState"


class Position(ctypes.Structure):
    """Position telemetry payload."""

    _fields_ = [("x", ctypes.c_float), ("y", ctypes.c_float)]

    # shared type name; must match across languages
    @staticmethod
    def type_name() -> str:
        return "Position"
# snippet:end
