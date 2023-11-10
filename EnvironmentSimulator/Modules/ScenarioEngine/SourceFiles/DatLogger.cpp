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

#include <iostream>

#include "DatLogger.hpp"

using namespace scenarioengine;

void DatLogger::step(const ObjectState &objState )
// void DatLogger::step()
{
    if (data_file_.is_open())
    {
        PkgTime packageTime;
        packageTime.hdr.id = static_cast<int>(PackageId::TIME_SERIES); // version id
        packageTime.hdr.content_size = sizeof(objState.state_.info.timeStamp);
        packageTime.time = objState.state_.info.timeStamp;
        packageTime.pkg_end.pkg_size = sizeof(packageTime);
        std::cout << "------Time Package->Send----- " << std::endl;
        logPackage(packageTime);
        std::cout << "------------------------- " << std::endl;

        PkgPos packagePos;
        packagePos.hdr.id = static_cast<int>(PackageId::POSITIONS); // version id
        packagePos.pos.x = objState.state_.pos.GetX();
        packagePos.pos.y = objState.state_.pos.GetY();
        packagePos.pos.z = objState.state_.pos.GetZ();
        packagePos.pos.h = objState.state_.pos.GetH();
        packagePos.pos.r = objState.state_.pos.GetR();
        packagePos.pos.p = objState.state_.pos.GetP();
        packagePos.hdr.content_size = sizeof(packagePos.pos);
        packagePos.pkg_end.pkg_size = sizeof(packagePos);
        std::cout << "------Pos package->Send----- " << std::endl;
        logPackage(packagePos);
        std::cout << "------------------------- " << std::endl;

    }
}

void DatLogger::logPackage(PkgVersion package )
{
    if (data_file_.is_open())
    {
        PkgVersion ver = getLatestPackage<PkgVersion>(package.hdr.id, package.pkg_end.pkg_size);
        if (ver.version == package.version)
        {
            std::printf("Package skipped->same as pervious\n");
        }
        else
        {
            std::cout << "New Package->written package id : " << package.hdr.id << std::endl;
            data_file_.write(reinterpret_cast<char*>(&package), sizeof(package));
        }
    }
    else
    {
        std::printf("File is not open:");
    }

}

void DatLogger::logPackage(PkgPos package )
{
    if (data_file_.is_open())
    {

        PkgPos pos = getLatestPackage<PkgPos>(package.hdr.id, package.pkg_end.pkg_size);
        if (pos.pos.h == package.pos.h && pos.pos.id == package.pos.id && pos.pos.p == package.pos.p &&
            pos.pos.r == package.pos.r && pos.pos.x == package.pos.x && pos.pos.y == package.pos.y &&
            pos.pos.z == package.pos.z)
        {
            std::printf("Package skipped->same as pervious\n");
        }
        else
        {
            std::cout << "New Package->written package id : " << package.hdr.id << std::endl;
            data_file_.write(reinterpret_cast<char*>(&package), sizeof(package));
        }
    }
    else
    {
        std::printf("File is not open:");
    }

}

void DatLogger::logPackage(PkgTime package )
{
    if (data_file_.is_open())
    {
        PkgTime time_s = getLatestPackage<PkgTime>(package.hdr.id, package.pkg_end.pkg_size);
        if (time_s.time == package.time && time_s.pkg_end.pkg_size == package.pkg_end.pkg_size)
        {
            std::printf("Package skipped->same as pervious\n");
        }
        else
        {
            std::cout << "New Package->written package id : " << package.hdr.id << std::endl;
            data_file_.write(reinterpret_cast<char*>(&package), sizeof(package));
        }
    }
    else
    {
        std::printf("File is not open:");
    }

}

int DatLogger::init(std::string fileName, std::string odr_filename, std::string model_filename)
{
    std::ofstream data_file(fileName, std::ios::binary);
    data_file_.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    if (data_file_.is_open()) {
        std::cout << "File opened successfully." << std::endl;
    }
    if (data_file_.fail())
    {
        std::printf("Cannot open file: %s", fileName.c_str());
        return -1;
    }

    // Write status to file - for later replay
    PkgVersion packageVersion;
    packageVersion.hdr.id = static_cast<int>(PackageId::VERSION); // version id
    packageVersion.hdr.content_size = sizeof(int);
    packageVersion.version = DAT_FILE_FORMAT_VERSION;
    packageVersion.pkg_end.pkg_size = sizeof(packageVersion.hdr.id) + sizeof(packageVersion.hdr.content_size) + packageVersion.hdr.content_size + sizeof(packageVersion.pkg_end);
    std::cout << "------Version Package->Send----- " << std::endl;
    std::cout << "Package id Size: " << sizeof(packageVersion.hdr.id) << std::endl;
    std::cout << "Package size Size: " << sizeof(packageVersion.hdr.content_size) << std::endl;
    std::cout << "Package content Size: " << packageVersion.hdr.content_size << std::endl;
    std::cout << "Package size end: " << sizeof(packageVersion.pkg_end) << std::endl;
    std::cout << "Complete Package Size: " << packageVersion.pkg_end.pkg_size << std::endl;
    data_file_.write(reinterpret_cast<char*>(&packageVersion), packageVersion.pkg_end.pkg_size);
    std::cout << "------------------------- " << std::endl;

    pkgOdrFilename packageOdr;
    packageOdr.hdr.id = static_cast<unsigned int>(PackageId::ODR_FILENAME);
    packageOdr.hdr.content_size = odr_filename.size() + 1;
    packageOdr.odr_filename = new char[packageOdr.hdr.content_size];
    strncpy(packageOdr.odr_filename, odr_filename.c_str(), packageOdr.hdr.content_size);
    packageOdr.pkg_end.pkg_size = sizeof(packageOdr.hdr.id) + sizeof(packageOdr.hdr.content_size) + packageOdr.hdr.content_size + sizeof(packageOdr.pkg_end);
    std::cout << "------ODR filename Package->Send----- " << std::endl;
    std::cout << "Package id Size: " << sizeof(packageOdr.hdr.id) << std::endl;
    std::cout << "Package size Size: " << sizeof(packageOdr.hdr.content_size) << std::endl;
    std::cout << "Package content Size: " << packageOdr.hdr.content_size << std::endl;
    std::cout << "Package size end: " << sizeof(packageOdr.pkg_end) << std::endl;
    std::cout << "Complete Package Size: " << packageOdr.pkg_end.pkg_size << std::endl;
    data_file_.write(reinterpret_cast<char*>(&packageOdr.hdr), sizeof(packageOdr.hdr));
    data_file_.write(packageOdr.odr_filename, packageOdr.hdr.content_size);
    data_file_.write(reinterpret_cast<char*>(&packageOdr.pkg_end), sizeof(packageOdr.pkg_end));
    std::cout << "------------------------- " << std::endl;

    pkgModelFilename packageModelname;
    packageModelname.hdr.id = static_cast<unsigned int>(PackageId::MODEL_FILENAME);
    packageModelname.hdr.content_size = model_filename.size() + 1;
    packageModelname.model_filename = new char[model_filename.size()];
    strncpy(packageModelname.model_filename, model_filename.c_str(), packageModelname.hdr.content_size);
    packageModelname.pkg_end.pkg_size = sizeof(packageModelname.hdr.id) + sizeof(packageModelname.hdr.content_size) + packageModelname.hdr.content_size + sizeof(packageModelname.pkg_end);
    std::cout << "------Model filename Package->Send----- " << std::endl;
    std::cout << "Package id Size: " << sizeof(packageModelname.hdr.id) << std::endl;
    std::cout << "Package size Size: " << sizeof(packageModelname.hdr.content_size) << std::endl;
    std::cout << "Package content Size: " << packageModelname.hdr.content_size << std::endl;
    std::cout << "Package size end: " << sizeof(packageModelname.pkg_end) << std::endl;
    std::cout << "Complete Package Size: " << packageModelname.pkg_end.pkg_size << std::endl;
    data_file_.write(reinterpret_cast<char*>(&packageModelname.hdr), sizeof(packageModelname.hdr));
    data_file_.write(packageModelname.model_filename, packageModelname.hdr.content_size);
    data_file_.write(reinterpret_cast<char*>(&packageModelname.pkg_end), sizeof(packageModelname.pkg_end));
    std::cout << "------------------------- " << std::endl;

    return 0;
}

template <typename T>
T DatLogger::getLatestPackage(const int id, const int size) {
    // Open the file in read mode.
    if (!data_file_.is_open()) {
        std::cerr << "Error opening file: " << std::endl;
        return T();
    }

    std::cout << "Searching package with id: " << id << " and size: "<< size << std::endl;
    // Get the current file pointer position.
    std::streampos currentFilePointerPosition = data_file_.tellg();
    std::cout << "Pointer num: " << currentFilePointerPosition <<" while entering" << std::endl;

    // Seek to the end of the file.
    data_file_.seekg(0, std::ios::end);
    std::streampos lastPointerPosition = data_file_.tellg();
    std::cout << "Pointer num: " << lastPointerPosition <<" last position" << std::endl;

    // exit if first entry or given package size is greater than last pointer number.
    T package;
    if (static_cast<int>(lastPointerPosition) < size)
    {
        data_file_.seekg(currentFilePointerPosition, std::ios::beg);
        std::streampos exitPointerPosition = data_file_.tellg();
        std::cout << "Pointer num: " << exitPointerPosition <<" while exit" << std::endl;
        std::cout << "No package found with id: " << id << " in file"<< std::endl;
        return T();
    }

    int pkgSizeTemp = 0;
    // Find the package with the specified ID, searching backward from the end of the file.
    while (data_file_.tellg() >= 0) {

        if ((pkgSizeTemp + sizeof(CommonPkgEnd)) > data_file_.tellg())
        {
            data_file_.seekg(currentFilePointerPosition, std::ios::beg);
            std::cout << "Pointer num: " << data_file_.tellg() <<" while exit" << std::endl;
            std::cout << "Reached last package, No package found with id: " << id << " in file"<< std::endl;
            return T();
        }
        //Move backward
        data_file_.seekg(-(pkgSizeTemp + sizeof(CommonPkgEnd)), std::ios::cur);

        // Check if the seekg() function was successful.
        if (data_file_.tellg() == std::streampos(-1)) {
            std::cout << "pointer did not moved, Searching unsuccessful" << std::endl;
            data_file_.seekg(currentFilePointerPosition, std::ios::beg);
            return T();
        }

        std::cout << "Pointer num: " << data_file_.tellg() <<" dynamic" << std::endl;

        //Read the pervious package end.
        data_file_.read(reinterpret_cast<char*>(&package.pkg_end.pkg_size), sizeof(CommonPkgEnd));
        std::cout << "Pointer num: " << data_file_.tellg() <<" after overall size read" << std::endl;
        std::cout << "overall size: " << package.pkg_end.pkg__size <<" after overall size read" << std::endl;

        // continue searching till same size package as given package
        if (package.pkg_end.pkg__size != size)
        {
            // set to pervious package
            pkgSizeTemp = package.pkg_end.pkg__size;
            continue;
        }

        std::cout << "---Package size matches with given package---- " << std::endl;
        std::cout << "Pointer Num after matching size: " << data_file_.tellg() << std::endl;

        // go to start of of package
        data_file_.seekg(-(size), std::ios::cur);

        // Read the package ID.
        data_file_.read(reinterpret_cast<char*>(&package.hdr.id), sizeof(package.hdr.id));

        // If the package ID is not correct, continue searching.
        if (package.hdr.id != id) {
        // reset the pointer to sync while,  since next iteration pointer shall be moved
        data_file_.seekg(-sizeof(package.hdr.id), std::ios::cur);
        std::cout << "Pointer num: " << data_file_.tellg()<<" corrected" << std::endl;
        continue;
        }

        std::cout << "---Package id matches with given id package---- " << std::endl;

        // Read the package size.
        data_file_.read(reinterpret_cast<char*>(&package.hdr.content_size), sizeof(package.hdr.content_size));

        // If the package size is invalid, return an empty package.
        if (package.hdr.content_size <= 0) {
        data_file_.seekg(0, std::ios::end);
        // Move the file pointer back to the original position.
        data_file_.seekg(currentFilePointerPosition, std::ios::beg);
        return T();
        }

        // id size, overall size confirmed now Seek to the before id and size to read complete package.
        data_file_.seekg(-(sizeof(CommonPkgHdr)), std::ios::cur);
        data_file_.read(reinterpret_cast<char*>(&package), package.pkg_end.pkg__size);

        // Read the package data.
        std::cout << "---Package found Successful---- " << std::endl;
        std::cout << "Complete Package Size: " << sizeof(package) << std::endl;
        std::cout << "Package id: " << package.hdr.id << std::endl;
        std::cout << "Package content size: " << package.hdr.content_size << std::endl;
        std::cout << "Complete Package size: " << package.pkg_end.pkg__size << std::endl;

        // Move the data_file_ pointer back to the original position.
        data_file_.seekg(currentFilePointerPosition, std::ios::beg);

        std::streampos exitPointerPosition = data_file_.tellg();
        std::cout << "Pointer num: " << exitPointerPosition <<" while exit" << std::endl;
        return package;
    }

    std::streampos beforeSettingExitPointerPosition = data_file_.tellg();
    std::cout << "Pointer num: " << beforeSettingExitPointerPosition <<" before setting to pointer last " << std::endl;

    // If we reach here, the package with the specified ID was not found.
    // Move the file pointer back to the original position.
    data_file_.seekg(currentFilePointerPosition, std::ios::beg);

    std::streampos exitPointerPosition = data_file_.tellg();
    std::cout << "Pointer num: " << exitPointerPosition <<" while exit" << std::endl;

    std::cout << "No package found with id: " << id << " in file"<< std::endl;
    return T();
}