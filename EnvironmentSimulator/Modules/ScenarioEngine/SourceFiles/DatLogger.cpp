// logDat

#include <iostream>
#include <fstream>
#include <cstring>

#include "DatLogger.hpp"

using namespace DatLogger;

void DatLogger::DatLogger::writePackage(CommonPkg package )
{
    if (data_file_.is_open())
    {
        std::cout << "New Package->written package id : " << package.hdr.id << std::endl;
        data_file_.write(reinterpret_cast<char*>(&package.hdr), sizeof(CommonPkgHdr));
        data_file_.write(package.content, package.hdr.content_size);
        data_file_.write(reinterpret_cast<char*>(&package.end.pkg_size), sizeof(CommonPkgEnd));
    }
    else
    {
        std::printf("File is not open:");
    }
}

void DatLogger::DatLogger::logPackage(CommonPkg package )
{
    switch (static_cast<PackageId>(package.hdr.id))
    {
        case PackageId::TIME_SERIES:
        {
            // get content from the old package from file
            Time TimeContent1 = getLatestPackage<Time>(package.hdr.id, package.end.pkg_size, package.hdr.content_size);
            // read content from the new package
            Time TimeContent2 = *reinterpret_cast<Time*>(package.content);
            // compare two content
            if(TimeContent1.time == TimeContent2.time)
            {
                std::cout << "Same as pervious->Skipped Package id: " << package.hdr.id << std::endl;
                break;
            }
            else
            {
                writePackage(package);
                break;
            }
        }

        case PackageId::OBJ_ID:
        {
            // get content from the old package from file
            ObjId objIdContent1 = getLatestPackage<ObjId>(package.hdr.id, package.end.pkg_size, package.hdr.content_size);
            // read content from the new package
            ObjId objIdContent2 = *reinterpret_cast<ObjId*>(package.content);
            // compare two content
            if(objIdContent1.obj_id == objIdContent2.obj_id)
            {
                std::cout << "Same as pervious->Skipped Package id: " << package.hdr.id << std::endl;
                break;
            }
            else
            {
                writePackage(package);
                break;
            }
        }

        case PackageId::POSITIONS:
        {
            // get content from the old package from file
            Pos posContent1 = getLatestPackage<Pos>(package.hdr.id, package.end.pkg_size, package.hdr.content_size);
            // read content from the new package
            Pos posContent2 = *reinterpret_cast<Pos*>(package.content);
            // compare two content
            if (posContent1.h == posContent2.h && posContent1.p == posContent2.p &&
                posContent1.r == posContent2.r && posContent1.x == posContent2.x &&
                posContent1.y == posContent2.y && posContent1.z == posContent2.z)
            {
                std::cout << "Same as pervious->Skipped Package id: " << package.hdr.id << std::endl;
                break;
            }
            else
            {
                writePackage(package);
                break;
            }
        }

        default:
        {
            break;
        }
    }
}

int DatLogger::DatLogger::recordPackage(const std::string& fileName)
{
    std::ifstream file_Read_;
    file_Read_.open(fileName, std::ifstream::binary);
    if (file_Read_.fail())
    {
        std::printf("READ, Cannot open file: %s", fileName.c_str());
        return -1;
    }

    if (file_Read_.is_open())
    {
        std::cout << "File Opened for read" << std::endl;
    }
    while (!file_Read_.eof())
    {
        // read the header for every loop
        CommonPkgHdr  cmnHdrPkgRead;
        file_Read_.read(reinterpret_cast<char*>(&cmnHdrPkgRead), sizeof(CommonPkgHdr));
        switch (static_cast<PackageId>(cmnHdrPkgRead.id))
        {
            case PackageId::HEADER:
            {
                CommonPkg hdrPkgRead;
                hdrPkgRead.hdr = cmnHdrPkgRead;

                DatHdr datHdrRead;
                // Read content -> version
                file_Read_.read(reinterpret_cast<char*>(&datHdrRead.version), sizeof(datHdrRead.version));

                CommonString odrStrRead;
                // Read content -> odr filename size
                file_Read_.read(reinterpret_cast<char*>(&odrStrRead.size), sizeof(odrStrRead.size));

                // Read content -> odr filename string
                odrStrRead.string = new char[odrStrRead.size];
                file_Read_.read(odrStrRead.string, odrStrRead.size);

                CommonString mdlStrRead;
                // Read content -> model filename size
                file_Read_.read(reinterpret_cast<char*>(&mdlStrRead.size), sizeof(mdlStrRead.size));

                // Read content -> model filename string
                mdlStrRead.string = new char[mdlStrRead.size];
                file_Read_.read(mdlStrRead.string, mdlStrRead.size);

                datHdrRead.odrFilename = odrStrRead;
                datHdrRead.modelFilename = mdlStrRead;
                hdrPkgRead.content = reinterpret_cast<char*>(&datHdrRead);

                // Read whole package size
                file_Read_.read(reinterpret_cast<char*>(&hdrPkgRead.end.pkg_size), sizeof(hdrPkgRead.end.pkg_size));
                pkgs_.push_back(hdrPkgRead);
                break;
            }
            case PackageId::TIME_SERIES:
            {
                CommonPkg timePkgRead;
                timePkgRead.hdr = cmnHdrPkgRead;

                Time t;
                file_Read_.read(reinterpret_cast<char*>(&t.time), timePkgRead.hdr.content_size);
                file_Read_.read(reinterpret_cast<char*>(&timePkgRead.end.pkg_size), sizeof(CommonPkgEnd));
                timePkgRead.content = reinterpret_cast<char*>(&t.time);
                pkgs_.push_back(timePkgRead);
                break;
            }

            case PackageId::OBJ_ID:
            {
                CommonPkg objIdPkgRead;
                objIdPkgRead.hdr = cmnHdrPkgRead;

                ObjId objIdRead;
                file_Read_.read(reinterpret_cast<char*>(&objIdRead.obj_id), objIdPkgRead.hdr.content_size);
                file_Read_.read(reinterpret_cast<char*>(&objIdPkgRead.end.pkg_size), sizeof(CommonPkgEnd));
                objIdPkgRead.content = reinterpret_cast<char*>(&objIdRead.obj_id);
                pkgs_.push_back(objIdPkgRead);
                break;
            }

            case PackageId::POSITIONS:
            {

                CommonPkg posPkgRead;
                posPkgRead.hdr = cmnHdrPkgRead;

                Pos posRead;
                file_Read_.read(reinterpret_cast<char*>(&posRead), posPkgRead.hdr.content_size);
                file_Read_.read(reinterpret_cast<char*>(&posPkgRead.end.pkg_size), sizeof(CommonPkgEnd));
                posPkgRead.content = reinterpret_cast<char*>(&posRead);
                pkgs_.push_back(posPkgRead);
                break;
            }
            default:
            {
                std::cout << "Unknown package read->package id :" << std::endl;
                break;
            }
        }
    }
    return 0;
}


int DatLogger::DatLogger::init(std::string fileName, int ver, std::string odrName, std::string modelName)
{
    std::ofstream data_file(fileName, std::ios::binary);
    data_file_.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    if (data_file_.fail())
    {
        std::printf("Cannot open file: %s", fileName.c_str());
        return -1;
    }
    if (data_file_.is_open()) {
        std::cout << "File opened successfully." << std::endl;
    }

    CommonPkgHdr cmnHdr;
    cmnHdr.id = static_cast<int>(PackageId::HEADER);

    CommonString odrStr;
    odrStr.size = odrName.size() + 1;
    odrStr.string = new char[odrStr.size];
    strncpy(odrStr.string, odrName.c_str(), odrStr.size);
    CommonString mdlStr;
    mdlStr.size = modelName.size() + 1;
    mdlStr.string = new char[mdlStr.size];
    strncpy(mdlStr.string, modelName.c_str(), mdlStr.size);

    cmnHdr.content_size = sizeof(odrStr.size) + odrStr.size + mdlStr.size + sizeof(mdlStr.size);

    DatHdr datHdr;
    datHdr.version = ver;
    datHdr.odrFilename = odrStr;
    datHdr.modelFilename = mdlStr;

    CommonPkg hdrPkg;
    hdrPkg.hdr = cmnHdr;
    hdrPkg.content = reinterpret_cast<char*>(&datHdr);
    hdrPkg.end.pkg_size = sizeof(CommonPkgHdr) + cmnHdr.content_size + sizeof(CommonPkgEnd);

    // write common header

    // file_.write(reinterpret_cast<char*>(&hdrPkg.hdr), sizeof(hdrPkg.hdr));
    data_file_.write(reinterpret_cast<char*>(&hdrPkg.hdr), sizeof(CommonPkgHdr));

    // write header package

    // write content -> version
    data_file_.write(reinterpret_cast<char*>(&datHdr.version), sizeof(datHdr.version));

    // write content -> odr filename size
    data_file_.write(reinterpret_cast<char*>(&datHdr.odrFilename.size), sizeof(datHdr.odrFilename.size));

    // write actual odr filename string
    data_file_.write(odrStr.string, datHdr.odrFilename.size);

    // write content -> model filename size
    data_file_.write(reinterpret_cast<char*>(&datHdr.modelFilename.size), sizeof(datHdr.modelFilename.size));

    // write actual model filename string
    data_file_.write(mdlStr.string, datHdr.modelFilename.size);

    // write packag
    data_file_.write(reinterpret_cast<char*>(&hdrPkg.end.pkg_size), sizeof(hdrPkg.end));

    return 0;
}

template <typename T>
T DatLogger::DatLogger::getLatestPackage(const int id, const unsigned long long pkgSize, const int contentSize) {
    // Open the file in read mode.
    if (!data_file_.is_open()) {
        std::cerr << "Error opening file: " << std::endl;
        return T();
    }

    // Get the current file pointer position.
    std::streampos currentFilePointerPosition = data_file_.tellg();

    if(display_print){
    std::cout << "Searching package with id-> " << id << " package size-> "<< pkgSize << " content size-> " << contentSize << std::endl;
    std::cout << "Pointer num: " << currentFilePointerPosition <<" while entering" << std::endl;
    }


    // exit if first entry.
    T package;
    if (isFirstEntry)
    {
        data_file_.seekg(currentFilePointerPosition, std::ios::beg);
        if(display_print){
        std::cout << "Pointer num: " << data_file_.tellg() <<" while exit" << std::endl;
        std::cout << "First entry with id: " << id << " in file"<< std::endl;
        }
        return T();
    }
    int pkgSizeTemp = 0;
    // Find the package with the specified ID, searching backward from the end of the file.
    while (data_file_.tellg() > 0) {
        data_file_.seekg(-(pkgSizeTemp + sizeof(CommonPkgEnd)), std::ios::cur);
        std::streampos dynamic_position = data_file_.tellg();

        // Check if the seekg() function was successful.
        if (dynamic_position == std::streampos(-1))
        {
            if(display_print)
            {
                std::cout << "pointer did not moved, Searching unsuccessful" << std::endl;
            }
            data_file_.seekg(currentFilePointerPosition, std::ios::beg);
            return T();
        }

        if(display_print)
        {
            std::cout << "Pointer num: " << dynamic_position <<" dynamic" << std::endl;
        }

        CommonPkgEnd pkgEnd;
        //Read the pervious package end.
        data_file_.read(reinterpret_cast<char*>(&pkgEnd.pkg_size), sizeof(CommonPkgEnd));
        // use it next iteration till it find same pkg size.
        pkgSizeTemp = pkgEnd.pkg_size;
        if(display_print)
        {
            std::cout << "Pointer num: " << data_file_.tellg() <<" after overall size read" << std::endl;
            std::cout << "overall size: " << pkgEnd.pkg_size <<" after overall size read" << std::endl;
        }

        // continue searching till same size package as given package
        if (pkgEnd.pkg_size != pkgSize)
        {
            continue;
        }
        // This variable no mare valid from here.
        pkgSizeTemp = 0;

        if(display_print)
        {
            std::cout << "---Package size matches with given package---- " << std::endl;
            std::cout << "Pointer Num after matching size: " << data_file_.tellg() << std::endl;
        }

        // go to start of of package
        data_file_.seekg(-(pkgSize), std::ios::cur);

        // Read the package ID.

        CommonPkgHdr cmnPkgHdr_;
        data_file_.read(reinterpret_cast<char*>(&cmnPkgHdr_.id), sizeof(cmnPkgHdr_.id));

        // If the package ID is not correct, continue searching.
        if (cmnPkgHdr_.id != id)
        {
            // reset the pointer to sync while,  since next iteration pointer shall be moved
            data_file_.seekg(-sizeof(cmnPkgHdr_.id), std::ios::cur);
            if(display_print)
            {
                std::cout << "Pointer num: " << data_file_.tellg()<<" corrected" << std::endl;
            }
            continue;
        }
        if(display_print)
        {
            std::cout << "---Package id matches with given id package---- " << std::endl;
        }

        // Read the package size.
        data_file_.read(reinterpret_cast<char*>(&cmnPkgHdr_.content_size), sizeof(cmnPkgHdr_.content_size));

        // If the package size is invalid, continue searching or return an empty package.?TODO
        if (cmnPkgHdr_.content_size != contentSize)
        {
            // Move the file pointer back to the original position.
            data_file_.seekg(-sizeof(cmnPkgHdr_.content_size), std::ios::cur);
            continue;
        }

        // id size, overall size confirmed now read package content.
        data_file_.read(reinterpret_cast<char*>(&package), contentSize);

        // Read the package data.
        if(display_print)
        {
        std::cout << "---Package found Successful---- " << std::endl;
        std::cout << "Package id: " <<cmnPkgHdr_.id << std::endl;
        std::cout << "Package content size: " << cmnPkgHdr_.content_size << std::endl;
        std::cout << "Complete Package size: " << pkgEnd.pkg_size << std::endl;
        }

        // Move the data_file_ pointer back to the original position.
        data_file_.seekg(currentFilePointerPosition, std::ios::beg);

        if(display_print)
        {
            std::cout << "Pointer num: " << data_file_.tellg() <<" while exit" << std::endl;
        }
        return package;
    }

    std::streampos beforeSettingExitPointerPosition = data_file_.tellg();
    // If we reach here, the package with the specified ID was not found.
    // Move the file pointer back to the original position.
    data_file_.seekg(currentFilePointerPosition, std::ios::beg);
    if(display_print)
    {
        std::cout << "Pointer num: " << beforeSettingExitPointerPosition <<" before setting to pointer last " << std::endl;
        std::cout << "Pointer num: " << data_file_.tellg() <<" while exit" << std::endl;
        std::cout << "No package found with id: " << id << " in file"<< std::endl;
    }
    return T();
}