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
        PackageStructTime packageTime;
        packageTime.hdr.id = static_cast<int>(PackageId::TIME_SERIES); // version id
        packageTime.hdr.size = sizeof(objState.state_.info.timeStamp);
        packageTime.time = objState.state_.info.timeStamp;
        packageTime.package_end.end_of_package = sizeof(packageTime);
        std::cout << "------Time Package->Send----- " << std::endl;
        // // // std::cout << "Complete Package Size: " << sizeof(packageVersion) << std::endl;
        // // // std::cout << "Package id Size: " << sizeof(packageVersion.hdr.id) << std::endl;
        // // // std::cout << "Package size Size: " << sizeof(packageVersion.hdr.size) << std::endl;
        // // // std::cout << "Package content Size: " << sizeof(packageVersion.version) << std::endl;
        // // // std::cout << "------------------------- " << std::endl;
        logPackage(packageTime);
        std::cout << "------------------------- " << std::endl;

        PackageStructPos packagePos;
        packagePos.hdr.id = static_cast<int>(PackageId::POSITIONS); // version id
        packagePos.pos.id = objState.state_.info.id;
        packagePos.pos.x = objState.state_.pos.GetX();
        packagePos.pos.y = objState.state_.pos.GetY();
        packagePos.pos.z = objState.state_.pos.GetZ();
        packagePos.pos.h = objState.state_.pos.GetH();
        packagePos.pos.r = objState.state_.pos.GetR();
        packagePos.pos.p = objState.state_.pos.GetP();
        packagePos.hdr.size = sizeof(packagePos.pos);
        packagePos.package_end.end_of_package = sizeof(packagePos);
        std::cout << "------Pos package->Send----- " << std::endl;
        // std::cout << "Complete Package Size: " << sizeof(packagePos) << std::endl;
        // std::cout << "Package id Size: " << sizeof(packagePos.hdr.id) << std::endl;
        // std::cout << "Package size Size: " << sizeof(packagePos.hdr.size) << std::endl;
        // std::cout << "Package content Size: " << sizeof(packagePos.pos) << std::endl;
        // std::cout << "------------------------- " << std::endl;
        logPackage(packagePos);
        std::cout << "------------------------- " << std::endl;

    }
}

void DatLogger::logPackage(PackageStructVersion package )
{
    if (data_file_.is_open())
    {
        PackageStructVersion ver = getLatestPackage<PackageStructVersion>(package.hdr.id);
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

void DatLogger::logPackage(PackageStructPos package )
{
    if (data_file_.is_open())
    {

        PackageStructPos pos = getLatestPackage<PackageStructPos>(package.hdr.id);
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

void DatLogger::logPackage(PackageStructTime package )
{
    if (data_file_.is_open())
    {
        PackageStructTime time_s = getLatestPackage<PackageStructTime>(package.hdr.id);
        if (time_s.time == package.time && time_s.package_end.end_of_package == package.package_end.end_of_package)
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

int DatLogger::init(std::string fileName)
{
    std::ofstream data_file(fileName, std::ios::binary);
    data_file_.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    if (data_file_.fail())
    {
        std::printf("Cannot open file: %s", fileName.c_str());
        return -1;
    }

    // Write status to file - for later replay
    PackageStructVersion packageVersion;
    packageVersion.hdr.id = static_cast<int>(PackageId::VERSION); // version id
    packageVersion.hdr.size = sizeof(int);
    packageVersion.version = DAT_FILE_FORMAT_VERSION;
    packageVersion.package_end.end_of_package = sizeof(packageVersion);
    std::cout << "------Version Package->Send----- " << std::endl;
    // std::cout << "Complete Package Size: " << sizeof(packageVersion) << std::endl;
    // std::cout << "Package id Size: " << sizeof(packageVersion.hdr.id) << std::endl;
    // std::cout << "Package size Size: " << sizeof(packageVersion.hdr.size) << std::endl;
    // std::cout << "Package content Size: " << sizeof(packageVersion.version) << std::endl;
    // std::cout << "Package size end: " << sizeof(packageVersion.package_end) << std::endl;
    // std::cout << "------------------------- " << std::endl;
    data_file_.write(reinterpret_cast<char*>(&packageVersion), sizeof(packageVersion));
    std::cout << "------------------------- " << std::endl;

    return 0;
}

template <typename T>
T DatLogger::getLatestPackage(const int id) {
    // Open the file in read mode.
    if (!data_file_.is_open()) {
        std::cerr << "Error opening file: " << std::endl;
        return T();
    }

    // Get the current file pointer position.
    std::streampos currentFilePointerPosition = data_file_.tellg();
    std::cout << "Pointer num: " << currentFilePointerPosition <<" while entering" << std::endl;

    // Seek to the end of the file.
    data_file_.seekg(0, std::ios::end);
    std::streampos lastPointerPosition = data_file_.tellg();
    std::cout << "Pointer num: " << lastPointerPosition <<" last position" << std::endl;

    // Find the package with the specified ID, searching backward from the end of the file.
    T package;
    std::cout << "Given Package Size: " << sizeof(T) << std::endl;
    if (static_cast<int>(lastPointerPosition) < static_cast<int>(sizeof(T)))
    {
        data_file_.seekg(currentFilePointerPosition, std::ios::beg);
        std::streampos exitPointerPosition = data_file_.tellg();
        std::cout << "Pointer num: " << exitPointerPosition <<" while exit" << std::endl;
        return T();
    }
    int searchingPackSize = 0;
    // std::cout << "Complete Package Size T: " << sizeof(T) << std::endl;
    while (data_file_.tellg() > 0) {

        data_file_.seekg(-4, std::ios::cur);
        searchingPackSize += 4;
        std::cout << "Searching package size: " << searchingPackSize << " dynamic"<<std::endl;
        std::streampos dynamic_position = data_file_.tellg();

        // Check if the seekg() function was successful.
        if (dynamic_position == std::streampos(-1)) {
            std::cout << "pointer did not moved" << std::endl;
        // Handle the error.
        }

        std::cout << "Pointer num: " << dynamic_position <<" dynamic" << std::endl;

        // check furthur in case last package
        if (data_file_.tellg() != 0)
        {
            // continue searching for two package end Read the pervious package end.
            data_file_.read(reinterpret_cast<char*>(&package.package_end.end_of_package), 4);
            std::cout << "Pointer num: " << data_file_.tellg() <<" read" << std::endl;
            // // move back again
            data_file_.seekg(-4, std::ios::cur);
            std::cout << "Pointer num: " << data_file_.tellg()<<" corrected" << std::endl;

            // continue searching till same size package as given package
            if (package.package_end.end_of_package != sizeof(T)) {
                searchingPackSize = 0;
            continue;
            }

            std::cout << "---Package size matches with given package---- " << std::endl;

        }
        data_file_.seekg(-(sizeof(T)-4), std::ios::cur);

        std::cout << "Pointer Num after matching size: " << data_file_.tellg() << std::endl;

        // Read the package ID.
        data_file_.read(reinterpret_cast<char*>(&package.hdr.id), sizeof(package.hdr.id));

        // If the package ID is not correct, continue searching.
        if (package.hdr.id != id) {
        continue;
        }

        std::cout << "---Package id matches with id package---- " << std::endl;

        // Read the package size.
        data_file_.read(reinterpret_cast<char*>(&package.hdr.size), sizeof(package.hdr.size));

        // If the package size is invalid, return an empty package.
        if (package.hdr.size <= 0) {
        data_file_.seekg(0, std::ios::end);
        // Move the file pointer back to the original position.
        data_file_.seekg(currentFilePointerPosition, std::ios::beg);
        return T();
        }

        // id and size confirmed now Seek to the before id and size to read complete package.
        data_file_.seekg(-(sizeof(package.hdr.id) + sizeof(package.hdr.size)), std::ios::cur);
        data_file_.read(reinterpret_cast<char*>(&package), sizeof(package));

        // Read the package data.
        std::cout << "---Package found with same id and size as given---- " << std::endl;
        std::cout << "Complete Package Size: " << sizeof(package) << std::endl;
        // std::cout << "Package id Size: " << sizeof(package.hdr.id) << std::endl;
        // std::cout << "Package size Size: " << sizeof(package.hdr.size) << std::endl;
        std::cout << "Package id: " << package.hdr.id << std::endl;
        std::cout << "Package content size: " << package.hdr.size << std::endl;
        // std::cout << "Complete Package content: " << package << std::endl;

        // Move the data_file_ pointer back to the original position.
        data_file_.seekg(currentFilePointerPosition, std::ios::beg);
        // Found the package.

        std::streampos exitPointerPosition = data_file_.tellg();
        std::cout << "Pointer num: " << exitPointerPosition <<" while exit" << std::endl;
        return package;
        break;
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