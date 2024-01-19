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
        self.id = None
        self.content_size = None
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

class PkgModelId(ctypes.Structure):
    _fields_ = [
        ('model_id', ctypes.c_int),
    ]

class PkgObjType(ctypes.Structure):
    _fields_ = [
        ('obj_type', ctypes.c_int),
    ]

class PkgObjCategory(ctypes.Structure):
    _fields_ = [
        ('obj_category', ctypes.c_int),
    ]
class PkgCtrlType(ctypes.Structure):
    _fields_ = [
        ('ctrl_type', ctypes.c_int),
    ]

class PkgWheelAngle(ctypes.Structure):
    _fields_ = [
        ('wheel_angle', ctypes.c_double),
    ]

class PkgWheelRot(ctypes.Structure):
    _fields_ = [
        ('wheel_rot', ctypes.c_double),
    ]

class PkgScaleMode(ctypes.Structure):
    _fields_ = [
        ('scale_mode', ctypes.c_int),
    ]

class PkgVisibilityMode(ctypes.Structure):
    _fields_ = [
        ('visibility_mask', ctypes.c_int),
    ]

class PkgRoadId(ctypes.Structure):
    _fields_ = [
        ('road_id', ctypes.c_int),
    ]

class PkgLaneId(ctypes.Structure):
    _fields_ = [
        ('lane_id', ctypes.c_int),
    ]

class PkgPosOffset(ctypes.Structure):
    _fields_ = [
        ('pos_offset', ctypes.c_double),
    ]

class PkgPosT(ctypes.Structure):
    _fields_ = [
        ('pos_T', ctypes.c_double),
    ]

class PkgPosS(ctypes.Structure):
    _fields_ = [
        ('pos_s', ctypes.c_double),
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

class objectState():
    def __init__(self):
        self.obj_id = None
        self.obj_active = None
        self.speed = None
        self.pos = None
        self.name = None

class CompleteObjectState():
    def __init__(self):
        self.time = None
        self.objectState_ = []

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
        self.pkgs = []
        self.get_all_pkg()
        self.CompleteObjectState_ = CompleteObjectState()
        self.InitiateStates()
        self.labels = [field[0] for field in ObjectStateStructDat._fields_]

        if (self.version.value != VERSION):
            print('Version mismatch. {} is version {} while supported version is: {}'.format(
                filename, self.version.value, VERSION)
            )
            exit(-1)

    def get_header_line(self):
        return 'Version: {}, OpenDRIVE: {}, 3DModel: {}'.format(
                self.version,
                self.odr_filename,
                self.model_filename
            )
    def get_all_pkg(self):
        stat = os.stat(self.file.name)
        while True:
            if self.file.tell() == stat.st_size:
                break # reach end of file
            header_buffer = self.file.read(ctypes.sizeof(CommonPkgHdr))

            header = CommonPkgHdr.from_buffer_copy(header_buffer)
            pkg = CommonPkg()
            pkg.id = header.id
            pkg.content_size = header.content_size

            if header.id == PkgId.HEADER.value:
                version_buffer = self.file.read(ctypes.sizeof(ctypes.c_int))
                self.version = ctypes.c_int.from_buffer_copy(version_buffer)
                odr_size_buffer = self.file.read(ctypes.sizeof(ctypes.c_int))
                odr_size = ctypes.c_int.from_buffer_copy(odr_size_buffer)
                odr_filename_bytes = self.file.read(odr_size.value)
                self.odr_filename = odr_filename_bytes[:odr_size.value].decode('utf-8')

                mdl_size_buffer = self.file.read(ctypes.sizeof(ctypes.c_int))
                mdl_size = ctypes.c_int.from_buffer_copy(mdl_size_buffer)
                mdl_filename_bytes = self.file.read(mdl_size.value)
                self.mdl_filename = mdl_filename_bytes[:mdl_size.value].decode('utf-8')

            elif header.id == PkgId.TIME_SERIES.value:
                time_buffer = self.file.read(header.content_size)
                t = PkgTime.from_buffer_copy(time_buffer)
                pkg.content = t
                self.pkgs.append(pkg)
            elif header.id == PkgId.OBJ_ID.value:
                obj_id_buffer = self.file.read(header.content_size)
                obj_id = PkgObjId.from_buffer_copy(obj_id_buffer)
                pkg.content = obj_id
                self.pkgs.append(pkg)
            elif header.id == PkgId.OBJ_ADDED.value:
                print (" obj added ")
                self.pkgs.append(pkg)
            elif header.id == PkgId.SPEED.value:
                speed_buffer = self.file.read(header.content_size)
                speed = PkgSpeed.from_buffer_copy(speed_buffer)
                pkg.content = speed
                self.pkgs.append(pkg)
            elif header.id == PkgId.POSITIONS.value:
                pos_buffer = self.file.read(header.content_size)
                pos = PkgPositions.from_buffer_copy(pos_buffer)
                pkg.content = pos
                self.pkgs.append(pkg)
            elif header.id == PkgId.NAME.value:
                name_buffer = self.file.read(header.content_size)
                name = name_buffer[:mdl_size.value].decode('utf-8')
                pkg.content = name
                self.pkgs.append(pkg)
            elif header.id == PkgId.MODEL_ID.value:
                model_id_buffer = self.file.read(header.content_size)
                model_id = PkgModelId.from_buffer_copy(model_id_buffer)
                pkg.content = model_id
                self.pkgs.append(pkg)
            elif header.id == PkgId.OBJ_TYPE.value:
                obj_type_buffer = self.file.read(header.content_size)
                obj_type = PkgObjType.from_buffer_copy(obj_type_buffer)
                pkg.content = obj_type
                self.pkgs.append(pkg)
            elif header.id == PkgId.OBJ_CATEGORY.value:
                obj_category_buffer = self.file.read(header.content_size)
                obj_category = PkgObjCategory.from_buffer_copy(obj_category_buffer)
                pkg.content = obj_category
                self.pkgs.append(pkg)
            elif header.id == PkgId.CTRL_TYPE.value:
                ctrl_type_buffer = self.file.read(header.content_size)
                ctrl_type = PkgCtrlType.from_buffer_copy(ctrl_type_buffer)
                pkg.content = ctrl_type
                self.pkgs.append(pkg)
            elif header.id == PkgId.WHEEL_ANGLE.value:
                wheel_angle_buffer = self.file.read(header.content_size)
                wheel_angle = PkgWheelAngle.from_buffer_copy(wheel_angle_buffer)
                pkg.content = wheel_angle
                self.pkgs.append(pkg)
            elif header.id == PkgId.WHEEL_ROT.value:
                wheel_rot_buffer = self.file.read(header.content_size)
                wheel_rot = PkgWheelRot.from_buffer_copy(wheel_rot_buffer)
                pkg.content = wheel_rot
                self.pkgs.append(pkg)
            elif header.id == PkgId.BOUNDING_BOX.value:
                bb_buffer = self.file.read(header.content_size)
                bb = PkgBB.from_buffer_copy(bb_buffer)
                pkg.content = bb
                self.pkgs.append(pkg)
            elif header.id == PkgId.SCALE_MODE.value:
                scale_mode_buffer = self.file.read(header.content_size)
                scale_mode = PkgScaleMode.from_buffer_copy(scale_mode_buffer)
                pkg.content = scale_mode
                self.pkgs.append(pkg)
            elif header.id == PkgId.VISIBILITY_MASK.value:
                visibility_mask_buffer = self.file.read(header.content_size)
                visibility_mask = PkgVisibilityMode.from_buffer_copy(visibility_mask_buffer)
                pkg.content = visibility_mask
                self.pkgs.append(pkg)
            elif header.id == PkgId.ROAD_ID.value:
                road_id_buffer = self.file.read(header.content_size)
                road_id = PkgRoadId.from_buffer_copy(road_id_buffer)
                pkg.content = road_id
                self.pkgs.append(pkg)
            elif header.id == PkgId.LANE_ID.value:
                lane_id_buffer = self.file.read(header.content_size)
                lane_id = PkgLaneId.from_buffer_copy(lane_id_buffer)
                pkg.content = lane_id
                self.pkgs.append(pkg)
            elif header.id == PkgId.POS_OFFSET.value:
                pos_offset_buffer = self.file.read(header.content_size)
                pos_offset = PkgPosOffset.from_buffer_copy(pos_offset_buffer)
                pkg.content = pos_offset
                self.pkgs.append(pkg)
            elif header.id == PkgId.POS_S.value:
                pos_s_buffer = self.file.read(header.content_size)
                pos_s = PkgPosOffset.from_buffer_copy(pos_s_buffer)
                pkg.content = pos_s
                self.pkgs.append(pkg)
            elif header.id == PkgId.POS_T.value:
                pos_t_buffer = self.file.read(header.content_size)
                pos_t = PkgPosOffset.from_buffer_copy(pos_t_buffer)
                pkg.content = pos_t
                self.pkgs.append(pkg)
            elif header.id == PkgId.END_OF_SCENARIO.value:
                print (" End of scenario ")
                self.pkgs.append(pkg)
            else:
                print("unknown pkg")
                ignore_buffer = self.file.read(header.content_size) # ignore it

        self.file.close()

    def InitiateStates(self):
        first_time_frame = False
        new_obj = False
        objectState_ = objectState()
        for pkg in self.pkgs:
            if pkg.id == PkgId.TIME_SERIES.value:
                if first_time_frame == True:
                    break
                self.CompleteObjectState_.time = pkg.content
                first_time_frame = True
            elif pkg.id == PkgId.OBJ_ID.value:
                if new_obj == True:
                    self.CompleteObjectState_.objectState_ .append(objectState_) # append for each object
                    objectState_ = objectState()
                    new_obj = False
                objectState_.obj_id = pkg.content
                new_obj = True
            elif pkg.id == PkgId.OBJ_ADDED.value:
                objectState_.obj_active = True
            elif pkg.id == PkgId.SPEED.value:
                objectState_.speed = pkg.content
            elif pkg.id == PkgId.POSITIONS.value:
                objectState_.pos = pkg.content
            elif pkg.id == PkgId.NAME.value:
                objectState_.name = pkg.content
        self.CompleteObjectState_.objectState_ .append(objectState_)

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
    # dat.print_csv(args.extended, args.file_refs)
    # dat.close()