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

namespace datLogger
{
    enum class PackageId
    {
        HEADER      = 11,
        MODEL_ID    = 12,
        TIME_SERIES = 13,
        OBJ_ID      = 14,
        POSITIONS   = 15,
        SPEED       = 16,
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
        double time;
    };

    struct Speed
    {
        double speed_ = 0.0;
    };

    struct ObjId
    {
        int obj_id = -1;
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

    struct CommonPkg
    {
        CommonPkgHdr hdr;
        char*        content;  // pointer to allocated content
        // CommonPkgEnd  end;
    };

    struct ObjState
    {
        ObjId obj_id_;
        Speed speed_;
        Pos   pos_;
        int   model_id;
    };

    struct CompleteObjectState
    {
        Time                  time;
        std::vector<ObjState> obj_states;
    };

    class ObjectState;
    class DatLogger
    {
    private:
        std::fstream data_file_;

    public:
        DatLogger() = default;
        ~DatLogger()
        {
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

            std::cout << "LOG Summary: " << std::endl;
            std::cout << "Total pos Package: " << posPkgs << std::endl;
            std::cout << "Total time Package " << timePkgs << std::endl;
            std::cout << "Total id Package: " << objIdPkgs << std::endl;
            std::cout << "Total speed Package: " << speedPkgs << std::endl;

            std::cout << "File closed successfully in destructure" << std::endl;
        }

        int posPkgs   = 0;
        int speedPkgs = 0;
        int timePkgs  = 0;
        int objIdPkgs = 0;

        bool isFirstEntry      = true;
        bool notFirstEnd       = false;
        bool display_print     = false;
        int  totalPkgProcessed = 0;
        int  totalPkgReceived  = 0;
        int  totalPkgSkipped   = 0;

        CompleteObjectState completeObjectState;

        int  init(std::string fileName, int ver, std::string odrName, std::string modelName);
        void deleteObjState(int objId);

        void writePackage(CommonPkg package);  // will just write package

        int         WriteObjSpeed(int obj_id, double speed);
        int         WriteTime(double t);
        int         WriteObjPos(int obj_id, double x, double y, double z, double h, double p, double r);
        int         WriteObjId(int obj_id);
        int         WriteModelId(int obj_id, int model_id);

        std::string pkgIdTostring(PackageId id);
    };

}  // namespace datLogger