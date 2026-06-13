"""Publish-subscribe payload type."""

# snippet:start payload
import ctypes


class Distance(ctypes.Structure):
    _fields_ = [
        ("distance_in_meters", ctypes.c_double),
        ("some_other_property", ctypes.c_float),
    ]

    def __str__(self) -> str:
        return (
            f"Distance {{ distance_in_meters: {self.distance_in_meters}, "
            f"some_other_property: {self.some_other_property} }}"
        )
# snippet:end payload
