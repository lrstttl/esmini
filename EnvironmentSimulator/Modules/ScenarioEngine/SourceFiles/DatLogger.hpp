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

    typedef struct
    {
        unsigned long long pkg_size;  // avoid padding for 64 bit alignment
    } CommonPkgEnd;

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
        double speed_;
    } Speed;

    typedef struct
    {
        int obj_id;
    } ObjId;

    typedef struct
    {
        double x;
        double y;
        double z;
        double h;
        double r;
        double p;
    } Pos;

    typedef struct
    {
        CommonPkgHdr  hdr;
        char*         content;   // pointer to allocated content
        CommonPkgEnd  end;
    } CommonPkg;

    typedef struct
    {
        double       time_;
        char*         pkg;
    } ObjectStateWithPkg;

    typedef struct
    {
        int       id;
        std::vector<ObjectStateWithPkg> pkgs;
    } ObjectStateWithObjId;

    typedef struct
    {
        double       sim_time;
        std::vector<ObjectStateWithObjId> obj_states;
    } ScenarioState;

    enum class Mode
    {
        WRITE = 0, // simple write
        READ = 1, // simple read
        LOG = 2,  // log packages
        RECORD = 3, // record all packages in vector
        REPLAY = 4, // method to fetch package
        BOTH_SIMPLE = 5,
        BOTH = 6,
        LOG_PKG = 7,
        SIMPLE = 8 // isolated problem checking
    } ;

    class ObjectState;
    class DatLogger {
    private:
        std::fstream data_file_;
    public:
        DatLogger() = default;
        ~DatLogger() {
            data_file_.flush();
            data_file_.close();
            if (system_mode == Mode::LOG)
            {
                std::cout << "LOG Summary: "<<std::endl;
                std::cout << "Total Package Received: " << totalPkgReceived <<std::endl;
                std::cout << "Total Package logged: " << totalPkgProcessed <<std::endl;
                std::cout << "Total Package Skipped: " << totalPkgSkipped <<std::endl;
            }
            if (system_mode == Mode::RECORD)
            {
                std::cout << "Record Summary: "<<std::endl;
                std::cout << "Total Package Received: " << totalPkgReceived <<std::endl;
                std::cout << "Total Package Recorded: " << totalPkgProcessed <<std::endl;
                std::cout << "Total Package Skipped: " << totalPkgSkipped <<std::endl;
            }

            std::cout << "File closed successfully in destructure" << std::endl;
        }

        void step(int obj_no);


        bool isFirstEntry = true;
        bool notFirstEnd = false;
        bool display_print = false;
        int totalPkgProcessed = 0;
        int totalPkgReceived = 0;
        int totalPkgSkipped = 0;
        std::vector<CommonPkg> pkgs_;
        ScenarioState objectStates_;

        Mode system_mode;
        void initiateStates(double time_frame);
        void updateStates(double time_frame);
        PackageId readPkgHdr(char* package );

        int init(std::string fileName, int ver, std::string odrName, std::string modelName);

        void logPackage(CommonPkg package, const int object_id); // check package can be logged or not
        void writePackage(CommonPkg package ); // will just write package
        int recordPackage(const std::string& fileName); // check package can be recorded or not
        std::vector<int> getObjIdPkgIndexBtwTime( double t); // till next time forward
        int getPkgCntBtwObj( int idx); // till next time forward
        double getTimeFromPkgIdx( int idx);
        std::string pkgIdTostring(PackageId id);
        double getTimeFromCnt(int count); // give time for the time
        void deleteObjState(int objId); // delete the object state for given object id from the current object state
        void addObjState(int objId, double t); // add the object state for given object id from the current object state
        int searchAndReplacePkg(int idx1, int idx2, int idx3, double time);
        int getNextTimeIdx(double t, int idxDir);
        int getIdxFromTime(double t);
        bool isObjAvailable(int Idx);  // check in current state
        bool isObjAvailable(int idx, std::vector<int> Indices);  // check in the object in the given new time
        template <typename T>
        T getLatestPackage(const int id, const unsigned long long pkgSize, const int contentSize, const int obj_id);

    };

}