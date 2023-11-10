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
#include "ScenarioGateway.hpp"

#include <fstream>
#include <vector>

#define DAT_FILE_FORMAT_VERSION 2

namespace scenarioengine
{
    enum class PackageId {
        VERSION = 11,
        POSITIONS   = 12,
        TIME_SERIES = 13,
        ODR_FILENAME = 14,
        OSC_BOUNDING_BOX = 15,
        MODEL_FILENAME = 16,
    };

    typedef struct
    {
        int id;
        int content_size;
    } CommonPkgHdr;

    typedef struct
    {
        int pkg_size;
    } CommonPkgEnd;

    typedef struct
    {
        CommonPkgHdr hdr;
        unsigned int version;
        CommonPkgEnd pkg_end;
    } PkgVersion;

    typedef struct
    {
        CommonPkgHdr hdr;
        char *odr_filename;
        CommonPkgEnd pkg_end;
    } pkgOdrFilename;

    typedef struct
    {
        CommonPkgHdr hdr;
        char *model_filename;
        CommonPkgEnd pkg_end;
    } pkgModelFilename;

    typedef struct
    {
        CommonPkgHdr hdr;
        double time;
        CommonPkgEnd pkg_end;
    } PkgTime;

    typedef struct
    {
        CommonPkgHdr hdr;
        double Obj_id;
        CommonPkgEnd pkg_end;
    } PkgObjId;

    typedef struct
    {
        double x;
        double y;
        double z;
        double h;
        double r;
        double p;
    } Positions;

    typedef struct
    {
        CommonPkgHdr hdr;
        Positions pos;
        CommonPkgEnd pkg_end;
    } PkgPos;


    class ObjectState; // Forward declaration of class ObjectState

    class DatLogger {
    private:
        std::fstream data_file_;

    public:
        DatLogger() = default;
        ~DatLogger() {
            data_file_.flush();
            data_file_.close();
            std::cout << "---------------------------------file closed successfully in destructure" << std::endl;
        }
        bool isFirstEntry = true;
        bool notFirstEnd = false;

        int init(std::string fileName, std::string odr_filename, std::string model_filename);

        // Log a common package
        void logPackage(PkgVersion package);
        void logPackage(PkgPos package );
        void logPackage(PkgTime package );
        void step(const ObjectState &objState);
        // void step();
        template <typename T>
        T getLatestPackage(const int id, const int size);

    };

}
