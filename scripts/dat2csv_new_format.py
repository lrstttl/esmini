import argparse
import ctypes
from enum import Enum, auto
import os

NAME_LEN = 32
VERSION = 2



class CommonPkgHdr(ctypes.Structure):
    _fields_ = [
        ('id', ctypes.c_int),
        ('content_size', ctypes.c_int),
    ]

class CommonPkg():
    def __init__(self):
        self.hdr = None
        self.content = None

class PkgTime(ctypes.Structure):
    _fields_ = [
        ('time', ctypes.c_double),
    ]

class PkgObjId(ctypes.Structure):
    _fields_ = [
        ('id', ctypes.c_uint),
    ]

class PkgSpeed(ctypes.Structure):
    _fields_ = [
        ('speed', ctypes.c_double),
    ]


class PkgPositions(ctypes.Structure):
    _fields_ = [
        ('x', ctypes.c_double),
        ('y', ctypes.c_double),
        ('z', ctypes.c_double),
        ('h', ctypes.c_double),
        ('r', ctypes.c_double),
        ('p', ctypes.c_double),
    ]

class PkgBB(ctypes.Structure):
    _fields_ = [
        ('x', ctypes.c_float),
        ('y', ctypes.c_float),
        ('z', ctypes.c_float),
        ('width', ctypes.c_float),
        ('length', ctypes.c_float),
        ('height', ctypes.c_float),
    ]

class ObjectStateStructDat(ctypes.Structure):
    _fields_ = [

        # ObjectInfoStruct
        ("id", ctypes.c_int),
        ("model_id", ctypes.c_int),
        ("obj_type", ctypes.c_int),
        ("obj_category", ctypes.c_int),
        ("ctrl_type", ctypes.c_int),
        ("time", ctypes.c_float),
        ('name', ctypes.c_char * NAME_LEN),
        ("speed", ctypes.c_float),
        ("wheel_angle", ctypes.c_float),
        ("wheel_rot", ctypes.c_float),
        ("centerOffsetX", ctypes.c_float),
        ("centerOffsetY", ctypes.c_float),
        ("centerOffsetZ", ctypes.c_float),
        ("width", ctypes.c_float),
        ("length", ctypes.c_float),
        ("height", ctypes.c_float),
        ("scaleMode", ctypes.c_int),
        ("visibilityMask", ctypes.c_int),

        # ObjectPositionStruct
        ("x", ctypes.c_float),
        ("y", ctypes.c_float),
        ("z", ctypes.c_float),
        ("h", ctypes.c_float),
        ("p", ctypes.c_float),
        ("r", ctypes.c_float),
        ("roadId", ctypes.c_int),
        ("laneId", ctypes.c_int),
        ("offset", ctypes.c_float),
        ("t", ctypes.c_float),
        ("s", ctypes.c_float),
    ]

class PkgId(Enum):
    HEADER      = 11
    TIME_SERIES = 12
    OBJ_ID      = 13
    MODEL_ID    = 14
    POSITIONS   = 15
    SPEED       = 16
    OBJ_TYPE    = 17
    OBJ_CATEGORY = 18
    CTRL_TYPE   = 19
    WHEEL_ANGLE = 20
    WHEEL_ROT   = 21
    BOUNDING_BOX = 22
    SCALE_MODE = 23
    VISIBILITY_MASK = 24
    NAME = 25
    ROAD_ID = 26
    LANE_ID = 27
    POS_OFFSET = 28
    POS_T = 29
    POS_S = 30
    OBJ_DELETED = 31
    OBJ_ADDED = 32
    END_OF_SCENARIO = 33

class ObjState:
    def __init__(self, obj_id: PkgObjId, active: bool, speed: PkgSpeed, pos: PkgPositions, model_id: int, obj_type: int, obj_category: int,
                 ctrl_type: int, wheel_angle: float, wheel_rot: float, bounding_box: PkgBB, scale_mode: int,
                 visibility_mask: int, name: str, road_id: int, lane_id: int, pos_offset: float, pos_t: float, pos_s: float) -> None:
        self.obj_id = obj_id
        self.active = active
        self.speed = speed
        self.pos = pos
        self.model_id = model_id
        self.obj_type = obj_type
        self.obj_category = obj_category
        self.ctrl_type = ctrl_type
        self.wheel_angle = wheel_angle
        self.wheel_rot = wheel_rot
        self.bounding_box = bounding_box
        self.scale_mode = scale_mode
        self.visibility_mask = visibility_mask
        self.name = name
        self.road_id = road_id
        self.lane_id = lane_id
        self.pos_offset = pos_offset
        self.pos_t = pos_t
        self.pos_s = pos_s

class CompleteObjectState:
    def __init__(self, time: int, obj_states: list) -> None:
        self.time = time
        self.obj_states = obj_states


class DATFile():
    def __init__(self, filename):
        if not os.path.isfile(filename):
            print('ERROR: dat-file not found: {}'.format(filename))
            return
        try:
            self.file = open(filename, 'rb')
        except OSError:
            print('ERROR: Could not open file {} for reading'.format(filename))
            raise

        self.filename = filename
        self.version = []
        self.odr_filename = []
        self.model_filename = []
        self.labels = [field[0] for field in ObjectStateStructDat._fields_]
        self.pkgs = []

        # if (self.version != VERSION):
        #     print('Version mismatch. {} is version {} while supported version is: {}'.format(
        #         filename, self.version, VERSION)
        #     )
        #     exit(-1)

    def get_header_line(self):
        return 'Version: {}, OpenDRIVE: {}, 3DModel: {}'.format(
                self.version,
                self.odr_filename,
                self.model_filename
            )
    def get_all_pkg(self):
        file_read = open(self.filename, 'rb')
        stat = os.stat(file_read.name)
        while True:
            if file_read.tell() == stat.st_size:
                break # reach end of file
            header_buffer = file_read.read(ctypes.sizeof(CommonPkgHdr))

            header = CommonPkgHdr.from_buffer_copy(header_buffer)
            pkg = CommonPkg()
            pkg.hdr.id = header.id
            pkg.hdr.content_size = header.content_size

            if header.id == PkgId.HEADER.value:
                version_buffer = file_read.read(ctypes.sizeof(ctypes.c_int))
                self.version = ctypes.c_int.from_buffer_copy(version_buffer)
                odr_size_buffer = file_read.read(ctypes.sizeof(ctypes.c_int))
                odr_size = ctypes.c_int.from_buffer_copy(odr_size_buffer)
                odr_filename_bytes = file_read.read(odr_size.value)
                self.odr_filename = odr_filename_bytes[:odr_size.value].decode('utf-8')

                mdl_size_buffer = file_read.read(ctypes.sizeof(ctypes.c_int))
                mdl_size = ctypes.c_int.from_buffer_copy(mdl_size_buffer)
                mdl_filename_bytes = file_read.read(mdl_size.value)
                self.mdl_filename = mdl_filename_bytes[:mdl_size.value].decode('utf-8')
            elif header.id == PkgId.TIME_SERIES.value:

                time_buffer = file_read.read(header.content_size)
                t = PkgTime()
                t = PkgTime.from_buffer_copy(time_buffer)
                pkg.content = t
                # pkg.content = ctypes.byref(t)
                # pkg.content = ctypes.cast(t.time, ctypes.byref(ctypes.c_char))
                # pkg.content = ctypes.c_char_p(ctypes.byref(t.time))
                # Add the time package to the list
                self.pkgs.append(pkg)
            elif header.id == PkgId.OBJ_ID.value:
                obj_id_buffer = file_read.read(header.content_size)
                time_series = PkgObjId.from_buffer_copy(obj_id_buffer)
            elif header.id == PkgId.OBJ_ADDED.value:
                print (" obj added ")
            elif header.id == PkgId.SPEED.value:
                speed_buffer = file_read.read(header.content_size)
                speed = PkgSpeed.from_buffer_copy(speed_buffer)
            elif header.id == PkgId.POSITIONS.value:
                pos_buffer = file_read.read(header.content_size)
                pos = PkgPositions.from_buffer_copy(pos_buffer)
        file_read.close()

if __name__ == "__main__":
    # # Create the parser
    # parser = argparse.ArgumentParser(description='Read and print .dat file')

    # # Add the arguments
    # parser.add_argument('filename', help='dat filename')
    # parser.add_argument('--extended', '-e', action='store_true', help='add road coordinates')
    # parser.add_argument('--file_refs', '-r', action='store_true', help='include odr and model file references')

    # # Execute the parse_args() method
    # args = parser.parse_args()

    # dat = DATFile(args.filename)
    dat = DATFile('sim.dat')
    dat.get_all_pkg()
    # dat.print_csv(args.extended, args.file_refs)
    # dat.close()