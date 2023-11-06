import argparse
import ctypes
from enum import Enum, auto
import os

class CommonPackageHeader(ctypes.Structure):
    _fields_ = [
        ('id', ctypes.c_int),
        ('size', ctypes.c_int),
    ]

class PackageStructOdrFilename(ctypes.Structure):
    _fields_ = [
        ('hdr', CommonPackageHeader),
        ('odr_filename', ctypes.c_char * 17),  # Adjust the size as needed
    ]

class PackageStructVersion(ctypes.Structure):
    _fields_ = [
        ('hdr', CommonPackageHeader),
        ('version', ctypes.c_uint),
        ('Overall_Size', ctypes.c_int),
    ]

class PackageStructTime(ctypes.Structure):
    _fields_ = [
        ('hdr', CommonPackageHeader),
        ('time', ctypes.c_float),
        ('Overall_Size', ctypes.c_int),
    ]

class PackageStructPositions(ctypes.Structure):
    _fields_ = [
        ('x', ctypes.c_double),
        ('y', ctypes.c_double),
        ('z', ctypes.c_double),
        ('h', ctypes.c_double),
        ('r', ctypes.c_double),
        ('p', ctypes.c_double),
    ]

class PackageStructId(ctypes.Structure):
    _fields_ = [
        ('id', ctypes.c_uint),
    ]

class PackageStructPos(ctypes.Structure):
    _fields_ = [
        ('hdr', CommonPackageHeader),
        ('pos', ctypes.c_double),
        ('Overall_Size', ctypes.c_int),

    ]

class OSCBoundingBox(ctypes.Structure):
    _fields_ = [
        ('x_', ctypes.c_float),
        ('y_', ctypes.c_float),
        ('z_', ctypes.c_float),
        ('width_', ctypes.c_float),
        ('length_', ctypes.c_float),
        ('height_', ctypes.c_float),
    ]

class PackageStructOSCBoundingBox(ctypes.Structure):
    _fields_ = [
        ('hdr', CommonPackageHeader),
        ('bb_', OSCBoundingBox),
    ]

class PackageId(Enum):
    VERSION = 11
    POSITIONS   = 12
    TIME_SERIES = 13
    ODR_FILENAME = 14
    OSC_BOUNDING_BOX = 15


dat = open('build/sim.dat', 'rb')
count = 1

while True:
    header_buffer = dat.read(ctypes.sizeof(CommonPackageHeader))

    if len(header_buffer) < ctypes.sizeof(CommonPackageHeader):
        break

    header = CommonPackageHeader.from_buffer_copy(header_buffer)

    print(f"Package number {count}")
    print(f"Package ID: {header.id}")
    print(f"Package Size: {header.size}")

    if header.id == PackageId.VERSION.value:
        # Read the version package data
        version_buffer = dat.read(header.size)
        version = ctypes.c_uint.from_buffer_copy(version_buffer)
        print(f"Version: {version.value}")
        version_buffer_end = dat.read(4)
        version_end = ctypes.c_uint.from_buffer_copy(version_buffer_end)
        print(f"Version Overall_Size: {version_end.value}")
    elif header.id == PackageId.TIME_SERIES.value:
        # Read the time series package data
        time_buffer = dat.read(header.size)
        time_series = ctypes.c_double.from_buffer_copy(time_buffer)
        print(f"Time Series: {time_series.value}")
        time_buffer_end = dat.read(4)
        time_series_end = ctypes.c_uint.from_buffer_copy(time_buffer_end)
        print(f"Time Overall_Size: {time_series_end.value}")
    elif header.id == PackageId.ODR_FILENAME.value:
        # Read the odr filename package data
        print(f"PackageStructOdrFilename Size: {ctypes.sizeof(PackageStructOdrFilename)}")
        print(f"CommonPackageHeader Size: {ctypes.sizeof(CommonPackageHeader)}")
        filename_buffer = dat.read(36)
        # odr_filename = PackageStructOdrFilename.from_buffer_copy(filename_buffer)
        # print(f"ODR Filename: {odr_filename.odr_filename.decode('utf-8')}")
    elif header.id == PackageId.POSITIONS.value:
        # Read the position package data
        pos_buffer_id = dat.read(4)
        positions_buffer_id = ctypes.c_uint.from_buffer_copy(pos_buffer_id)
        pos_buffer = dat.read(header.size - 4)
        positions = PackageStructPositions.from_buffer_copy(pos_buffer)
        pos_buffer_end = dat.read(4)
        positions_end = ctypes.c_uint.from_buffer_copy(pos_buffer_end)
        print(f"ID: {positions_buffer_id.value}")
        print(f"Position X: {positions.x}")
        print(f"Position Y: {positions.y}")
        print(f"Position Z: {positions.z}")
        print(f"Heading: {positions.h}")
        print(f"Roll: {positions.r}")
        print(f"Pitch: {positions.p}")
        print(f"Pos Overall_Size: {positions_end.value}")
    elif header.id == PackageId.OSC_BOUNDING_BOX.value:
        # Read the OSC bounding box package data
        osc_bbox_buffer = dat.read(header.size)
        osc_bbox = OSCBoundingBox.from_buffer_copy(osc_bbox_buffer)
        print(f"Center X: {osc_bbox.x_}")
        print(f"Center Y: {osc_bbox.y_}")
        print(f"Center Z: {osc_bbox.z_}")
        print(f"Dimensions Width: {osc_bbox.width_}")
        print(f"Dimensions Length: {osc_bbox.length_}")
        print(f"Dimensions Height: {osc_bbox.height_}")



    count += 1
    print(f"--------------------------")

dat.close()