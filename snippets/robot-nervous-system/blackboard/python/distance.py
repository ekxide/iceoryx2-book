"""Blackboard example payload type."""

import ctypes


class Distance(ctypes.Structure):
    _fields_ = [
        ("distance_in_meters", ctypes.c_double),
        ("some_other_property", ctypes.c_float),
    ]
