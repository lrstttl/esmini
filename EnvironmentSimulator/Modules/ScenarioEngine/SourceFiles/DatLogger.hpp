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
    enum class PackageId {
        HEADER      = 11,
        TIME_SERIES = 12,
        OBJ_ID      = 13,
        POSITIONS   = 14,
        SPEED       = 15,
    };

    // mandatory packages
    typedef struct
    {
        int id;
        int content_size;
    } CommonPkgHdr;


    // common package types
    typedef struct
    {
        int size;  // size of the string
        char *string;
    } CommonString;

    // specific packages
    typedef struct
    {
        int version;
        CommonString odrFilename;
        CommonString modelFilename;
    } DatHdr;

    typedef struct
    {
        double time;
    } Time;

    typedef struct
    {
        double speed_ = 0.0;
    } Speed;

    typedef struct
    {
        int obj_id = -1;
    } ObjId;

    typedef struct
    {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double h = 0.0;
        double r = 0.0;
        double p = 0.0;
    } Pos;

    typedef struct
    {
        CommonPkgHdr  hdr;
        char*         content;   // pointer to allocated content
        // CommonPkgEnd  end;
    } CommonPkg;



    typedef struct
    {
        ObjId obj_id_;
        Speed speed_;
        Pos   pos_;
    } ObjState;

    typedef struct
    {
        Time time;
        std::vector<ObjState> obj_states;

    } CompleteObjectState;


    class ObjectState;
    class DatLogger {
    private:
        std::fstream data_file_;
    public:
        DatLogger() = default;
        ~DatLogger() {
            data_file_.flush();
            data_file_.close();
            if (display_print)
            {
                std::cout << "LOG Summary: "<<std::endl;
                std::cout << "Total Package Received: " << totalPkgReceived <<std::endl;
                std::cout << "Total Package logged: " << totalPkgProcessed <<std::endl;
                std::cout << "Total Package Skipped: " << totalPkgSkipped <<std::endl;
            }
            if (display_print)
            {
                std::cout << "Record Summary: "<<std::endl;
                std::cout << "Total Package Received: " << totalPkgReceived <<std::endl;
                std::cout << "Total Package Recorded: " << totalPkgProcessed <<std::endl;
                std::cout << "Total Package Skipped: " << totalPkgSkipped <<std::endl;
            }

            std::cout << "LOG Summary: "<<std::endl;
            std::cout << "Total pos Package: " << posPkgs <<std::endl;
            std::cout << "Total time Package " << timePkgs <<std::endl;
            std::cout << "Total id Package: " << objIdPkgs <<std::endl;
            std::cout << "Total speed Package: " << speedPkgs <<std::endl;

            std::cout << "File closed successfully in destructure" << std::endl;
        }

        int posPkgs = 0;
        int speedPkgs = 0;
        int timePkgs = 0;
        int objIdPkgs = 0;

        bool isFirstEntry = true;
        bool notFirstEnd = false;
        bool display_print = false;
        int totalPkgProcessed = 0;
        int totalPkgReceived = 0;
        int totalPkgSkipped = 0;


        CompleteObjectState completeObjectState;

        int init(std::string fileName, int ver, std::string odrName, std::string modelName);
        void deleteObjState(int objId);

        void writePackage(CommonPkg package ); // will just write package

        int WriteObjSpeed(int obj_id, double speed);
        int WriteTime(double t);
        int WriteObjPos(int obj_id, double x, double y, double z, double h, double p, double r);
        int WriteObjId(int obj_id);
        std::string pkgIdTostring(PackageId id);

    };

}