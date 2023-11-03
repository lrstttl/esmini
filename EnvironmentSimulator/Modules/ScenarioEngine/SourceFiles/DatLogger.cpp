// logDat

#include <iostream>
#include <fstream>

#include "DatLogger.hpp"

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
        if (pos.pos.h == package.pos.h)
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
        if (time_s.time == package.time)
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
    std::cout << "Complete Package Size: " << sizeof(T) << std::endl;
    if (static_cast<int>(lastPointerPosition) < static_cast<int>(sizeof(T)) || isFirstEntry)
    {
        std::streampos exitPointerPosition = data_file_.tellg();
        std::cout << "Pointer num: " << exitPointerPosition <<" while exit" << std::endl;
        return T();
    }

    // std::cout << "Complete Package Size T: " << sizeof(T) << std::endl;
    while (data_file_.tellg() > 0) {
        data_file_.seekg(-sizeof(T), std::ios::cur);

        // Read the package ID.
        data_file_.read(reinterpret_cast<char*>(&package.hdr.id), sizeof(package.hdr.id));

        // If the package ID is not correct, continue searching.
        if (package.hdr.id != id) {
        continue;
        }

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
        std::cout << "---Package found---- " << std::endl;
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