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
    };
    #pragma pack(push, 4)
    typedef struct
    {
        int id;
        int size;
    } CommonPackageHeader;

    typedef struct
    {
        int end_of_package;
    } CommonPackageEnd;

    typedef struct
    {
        CommonPackageHeader hdr;
        unsigned int version;
        CommonPackageEnd package_end;
    } PackageStructVersion;

    typedef struct
    {
        int   id;
        double x;
        double y;
        double z;
        double h;
        double r;
        double p;
    } PackageStructPosition;


    typedef struct
    {
        CommonPackageHeader hdr;
        PackageStructPosition pos;
        CommonPackageEnd package_end;
    } PackageStructPos;

    typedef struct
    {
        CommonPackageHeader hdr;
        double time;
        CommonPackageEnd package_end;
    } PackageStructTime;

    #pragma pack(pop)

    class ObjectState; // Forward declaration of class ObjectState

    class DatLogger {
    private:
        std::fstream data_file_;

    public:
        DatLogger() = default;
        ~DatLogger() {
            data_file_.flush();
            data_file_.close();
        }
        bool isFirstEntry = true;
        bool notFirstEnd = false;

        int init(std::string fileName);

        // Log a common package
        void logPackage(PackageStructVersion package);
        void logPackage(PackageStructPos package );
        void logPackage(PackageStructTime package );
        void step(const ObjectState &objState);
        // void step();
        template <typename T>
        T getLatestPackage(const int id);

    };

}
