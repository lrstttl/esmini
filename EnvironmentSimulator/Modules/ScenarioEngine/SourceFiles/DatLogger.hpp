/*
 * esmini - Environment Simulator Minimalistic
 * https://github.com/esmini/esmini
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) partners of Simulation Scenarios
 * https://sites.google.com/view/simulationscenarios
 */

#pragma once

#include <fstream>
#include <vector>

#include "CommonMini.hpp"


namespace datLogger
{
    enum class PackageId
    {
        HEADER      = 11,
        TIME_SERIES = 12,
        OBJ_ID      = 13,
        MODEL_ID    = 14,
        POSITIONS   = 15,
        SPEED       = 16,
        OBJ_TYPE    = 17,
        OBJ_CATEGORY = 18,
        CTRL_TYPE   = 19,
        WHEEL_ANGLE = 20,
        WHEEL_ROT   = 21,
        BOUNDING_BOX = 22,
        SCALE_MODE = 23,
        VISIBILITY_MASK = 24,
        NAME = 25,
        ROAD_ID = 26,
        LANE_ID = 27,
        POS_OFFSET = 28,
        POS_T = 29,
        POS_S = 30,
        OBJ_STATUS = 31,
        END_OF_SCENARIO = 32
    };

    // mandatory packages
    struct CommonPkgHdr
    {
        int id;
        int content_size;
    };

    // common package types
    struct CommonString
    {
        int   size;  // size of the string
        char* string;
    };

    // specific packages
    struct DatHdr
    {
        int          version;
        CommonString odrFilename;
        CommonString modelFilename;
    };

    struct Time
    {
        double time = SMALL_NUMBER;
    };

    struct ObjId
    {
        int obj_id = -1;
    };
    struct ModelId
    {
        int model_id = -1;
    };

    struct ObjType
    {
        int obj_type = -1;
    };

    struct ObjCategory
    {
        int obj_category = -1;
    };

    struct CtrlType
    {
        int ctrl_type = -1;
    };

    struct WheelAngle
    {
        double wheel_angle = SMALL_NUMBER;
    };

    struct WheelRot
    {
        double wheel_rot = SMALL_NUMBER;
    };


    struct ScaleMode
    {
        int scale_mode = -1;
    };
    struct VisibilityMask
    {
        int visibility_mask = -1;
    };

    struct RoadId
    {
        int road_id = -LARGE_NUMBER_INT;
    };

    struct LaneId
    {
        int lane_id = -LARGE_NUMBER_INT;
    };

    struct PosOffset
    {
        double offset = SMALL_NUMBER;
    };

    struct PosT
    {
        double t = SMALL_NUMBER;
    };

    struct PosS
    {
        double s = SMALL_NUMBER;
    };

    struct Name
    {
        char* string;
    };

    struct Pos
    {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double h = 0.0;
        double r = 0.0;
        double p = 0.0;
    };

    struct BoundingBox
    {
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;
        float width = 0.0;
        float length = 0.0;
        float height = 0.0;
    };

    struct Speed
    {
        double speed_ = SMALL_NUMBER;
    };

    struct CommonPkg
    {
        CommonPkgHdr hdr;
        char*        content;  // pointer to allocated content
        // CommonPkgEnd  end;
    };

    struct ObjState
    {
        ObjId           obj_id_;
        bool            active = false;
        Speed           speed_;
        Pos             pos_;
        ModelId         modelId_;
        ObjType         objType_;
        ObjCategory     objCategory_;
        CtrlType        ctrlType_;
        WheelAngle      wheelAngle_;
        WheelRot        wheelRot_;
        BoundingBox     boundingBox_;
        ScaleMode       scaleMode_;
        VisibilityMask  visibilityMask_;
        std::string     name_;
        RoadId          roadId_;
        LaneId          laneId_;
        PosOffset       posOffset_;
        PosT            posT;
        PosS            posS;
    };

    struct CompleteObjectState
    {
        Time                  time;
        std::vector<ObjState> obj_states;
    };

    class DatLogger
    {
    private:
        std::fstream data_file_;

    public:
        DatLogger() = default;
        ~DatLogger()
        {
            totalPkgProcessed += 2;
            totalPkgReceived  += 2;
            WriteTime(simTimeTemp);

            CommonPkgHdr pkg;
            pkg.id = static_cast<int>(PackageId::END_OF_SCENARIO);
            pkg.content_size = 0;
            data_file_.write(reinterpret_cast<char*>(&pkg), sizeof(CommonPkgHdr));

            data_file_.flush();
            data_file_.close();
            if (display_print)
            {
                std::cout << "LOG Summary: " << std::endl;
                std::cout << "Total Package Received: " << totalPkgReceived << std::endl;
                std::cout << "Total Package logged: " << totalPkgProcessed << std::endl;
                std::cout << "Total Package Skipped: " << totalPkgSkipped << std::endl;
            }
            if (display_print)
            {
                std::cout << "Record Summary: " << std::endl;
                std::cout << "Total Package Received: " << totalPkgReceived << std::endl;
                std::cout << "Total Package Recorded: " << totalPkgProcessed << std::endl;
                std::cout << "Total Package Skipped: " << totalPkgSkipped << std::endl;
            }

            std::cout << "Total Package Received: " << totalPkgReceived << std::endl;
            std::cout << "Total Package logged: " << totalPkgProcessed << std::endl;
            std::cout << "Total Package Skipped: " << totalPkgSkipped << std::endl;

            std::cout << "LOG Summary: " << std::endl;
            std::cout << "Total time Package " << timePkgs << std::endl;
            std::cout << "Total id Package: " << objIdPkgs << std::endl;
            std::cout << "Total model id:: " << modelIdPkg << std::endl;
            std::cout << "Total pos Package: " << posPkgs << std::endl;
            std::cout << "Total speed Package: " << speedPkgs << std::endl;


            std::cout << "Total objTypePkg " << objTypePkg << std::endl;
            std::cout << "Total objCatePkg: " << objCatePkg << std::endl;
            std::cout << "Total ctrlTypPkg: " << ctrlTypPkg << std::endl;

            std::cout << "Total wheelAnPkg: " << wheelAnPkg << std::endl;
            std::cout << "Total wheelRoPkg: " << wheelRoPkg << std::endl;
            std::cout << "Total boundinPkg: " << boundinPkg << std::endl;
            std::cout << "Total scaleMoPkg: " << scaleMoPkg << std::endl;
            std::cout << "Total visibilPkg: " << visibilPkg << std::endl;
            std::cout << "Total namePkg: " << namePkg << std::endl;

            std::cout << "Total raodIdPkg: " << roadIdPkg << std::endl;
            std::cout << "Total laneIdPkg: " << laneIdPkg << std::endl;
            std::cout << "Total posOffPkg: " << posOffPkg << std::endl;
            std::cout << "Total posTPkg: " << posTPkg << std::endl;
            std::cout << "Total posSPkg: " << posSPkg << std::endl;

            std::cout << "File closed successfully in destructure" << std::endl;
        }

        int posPkgs   = 0;
        int speedPkgs = 0;
        int timePkgs  = 0;
        int objIdPkgs = 0;
        int modelIdPkg = 0;
        int objTypePkg = 0;
        int objCatePkg = 0;
        int ctrlTypPkg = 0;
        int wheelAnPkg = 0;
        int wheelRoPkg = 0;
        int boundinPkg = 0;
        int scaleMoPkg = 0;
        int visibilPkg = 0;
        int namePkg = 0;
        int roadIdPkg = 0;
        int laneIdPkg = 0;
        int posOffPkg = 0;
        int posTPkg = 0;
        int posSPkg = 0;


        bool isFirstEntry      = true;
        bool notFirstEnd       = false;
        bool display_print     = false;
        int  totalPkgProcessed = 0;
        int  totalPkgReceived  = 0;
        int  totalPkgSkipped   = 0;
        bool TimePkgAdded = false;
        bool ObjIdPkgAdded = false;

        double simTimeTemp = SMALL_NUMBER;

        CompleteObjectState completeObjectState;

        int  init(std::string fileName, int ver, std::string odrName, std::string modelName);
        void deleteObjState(int objId);


        void writePackage(CommonPkg package);  // will just write package
        void WriteManPkg(int obj_id);
        int  AddObject(int obj_id);
        int  deleteObject();

        int         WriteObjSpeed(int obj_id, double speed);
        int         WriteTime(double t);
        int         WriteObjPos(int obj_id, double x, double y, double z, double h, double p, double r);
        int         WriteObjId(int obj_id);
        int         WriteModelId(int obj_id, int model_id);
        int         WriteObjType(int obj_id, int obj_type);
        int         WriteObjCategory(int obj_id, int obj_category);
        int         WriteCtrlType(int obj_id, int ctrl_type);

        int         WriteWheelAngle(int obj_id, double angle);
        int         WriteWheelRot(int obj_id, double rot);
        int         WriteBB(int obj_id, float x, float y, float z, float length, float width, float height);
        int         WriteScaleMode(int obj_id, int mode);
        int         WriteVisiblityMask(int obj_id, int mask);
        int         WriteName(int obj_id, std::string name);
        int         WriteRoadId(int obj_id, int road_id);
        int         WriteLaneId(int obj_id, int lane_id);
        int         WritePosOffset(int obj_id, double pos_offset);
        int         WritePosT(int obj_id, double t);
        int         WritePosS(int obj_id, double s);

        std::string pkgIdTostring(PackageId id);
    };

}  // namespace datLogger