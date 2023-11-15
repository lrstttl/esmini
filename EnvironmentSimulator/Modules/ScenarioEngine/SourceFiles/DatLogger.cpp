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

void DatLogger::DatLogger::logPackage(CommonPkg package, const int object_id)
{
    switch (static_cast<PackageId>(package.hdr.id))
    {
        case PackageId::TIME_SERIES:
        {
            // Always write time package
            writePackage(package);
            break;
        }

        case PackageId::OBJ_ID:
        {
            // Always write obj id package
            writePackage(package);
            break;
        }

        case PackageId::POSITIONS:
        {
            // get content from the old package from file
            Pos posContent1 = getLatestPackage<Pos>(package.hdr.id, package.end.pkg_size, package.hdr.content_size, object_id);
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

        case PackageId::SPEED:
        {
            // get content from the old package from file
            Speed speedContent1 = getLatestPackage<Speed>(package.hdr.id, package.end.pkg_size, package.hdr.content_size, object_id);
            // read content from the new package
            Speed speedContent2 = *reinterpret_cast<Speed*>(package.content);
            // compare two content
            if (speedContent1.speed_ == speedContent2.speed_)
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
T DatLogger::DatLogger::getLatestPackage(const int id, const unsigned long long pkgSize, const int contentSize, const int objId) {
    // Open the file in read mode.
    if (!data_file_.is_open()) {
        std::cerr << "Error opening file: " << std::endl;
        return T();
    }

    // Get the current file pointer position.
    std::streampos currentFilePointerPosition = data_file_.tellg();

    if(display_print){
    std::cout << "Searching package with id-> " << id << " package size-> "<< pkgSize << " content size-> " << contentSize << " with ID: "<< objId <<std::endl;
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
    int pkgSizeTemp = 0; // pointer size to move forward
    int lastReadObjIdPkgPtrPos = 0; // to jump to last read obj id package pointer
    int ptrsToMove = 0; // pointer size to move backward
    int currentObjID; // to jump to last read obj id package pointer
    bool firstTimeObjIdPkgFound = false;

    // Find the object id package, searching backward from the end of the file.
    while (data_file_.tellg() > 0) {
        data_file_.seekg(-(ptrsToMove + sizeof(CommonPkgEnd)), std::ios::cur);

        // Check if the seekg() function was successful.
        if (data_file_.tellg() == std::streampos(-1))
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
            std::cout << "Pointer num: " << data_file_.tellg() <<" dynamic" << std::endl;
        }

        // stage 1-> Find the object id package.

        CommonPkgEnd pkgEnd;
        //Read the overall size package.
        data_file_.read(reinterpret_cast<char*>(&pkgEnd.pkg_size), sizeof(CommonPkgEnd));

        // seek to start of package
        data_file_.seekg(-pkgEnd.pkg_size, std::ios::cur);

        // Read the header
        CommonPkgHdr cmnPkgHdr_;
        data_file_.read(reinterpret_cast<char*>(&cmnPkgHdr_), sizeof(cmnPkgHdr_));

        // continue till object id package found
        if (cmnPkgHdr_.id != static_cast<int>(PackageId::OBJ_ID))
        {
            // current pointer is end of content size field
            ptrsToMove = sizeof(cmnPkgHdr_);
            continue;
        }
        else
        {   // first object id package not interested.
            if (!firstTimeObjIdPkgFound)
            {
                firstTimeObjIdPkgFound = true;
                // current pointer is end of content size field
                ptrsToMove = sizeof(cmnPkgHdr_);
                continue;
            }

            data_file_.read(reinterpret_cast<char*>(&currentObjID), cmnPkgHdr_.content_size);
            if (currentObjID == objId)
            {  // stop searching.
                lastReadObjIdPkgPtrPos = data_file_.tellg() - sizeof(cmnPkgHdr_) - cmnPkgHdr_.content_size;
                if(display_print)
                {
                    std::cout << "Found object id: " << currentObjID << std::endl;
                    std::cout << "Started look for package, Entering stage 2 "<< std::endl;
                }
            }
            else
            {
                // current pointer is end of content field
                ptrsToMove = sizeof(cmnPkgHdr_) + cmnPkgHdr_.content_size;
                continue;
            }
        }


        ptrsToMove = 0;
        // stage 2-> Find the package
        // object id found now look for package forward.Always content field end will be current pointer position for stage2
        data_file_.seekg(sizeof(CommonPkgEnd), std::ios::cur);
        if(display_print)
        {
            std::cout << "Pointer number in stage 2: " << data_file_.tellg() << std::endl;
        }

        while (!data_file_.eof() || data_file_.tellg() == std::streampos(lastReadObjIdPkgPtrPos))
        {
            // read header
            data_file_.read(reinterpret_cast<char*>(&cmnPkgHdr_), sizeof(cmnPkgHdr_));
            // skip package content.
            data_file_.seekg(cmnPkgHdr_.content_size, std::ios::cur);
            // read overall size
            data_file_.read(reinterpret_cast<char*>(&pkgEnd.pkg_size), sizeof(CommonPkgEnd));

            // go back to stage1.
            if (cmnPkgHdr_.id == static_cast<int>(PackageId::OBJ_ID))
            {
                // Jump to last known obj id package.
                data_file_.seekg(lastReadObjIdPkgPtrPos, std::ios::beg);
                break;
            }

            // If the package ID or content size or package size is not match, continue searching.
            if (cmnPkgHdr_.id != id || cmnPkgHdr_.content_size != contentSize || cmnPkgHdr_.content_size != contentSize)
            {
                continue;
            }
            // go back to content field start
            data_file_.seekg(-(sizeof(CommonPkgEnd) + cmnPkgHdr_.content_size), std::ios::cur);
            // read content
            data_file_.read(reinterpret_cast<char*>(&package), cmnPkgHdr_.content_size);

            // Move the data_file_ pointer back to the original position.
            data_file_.seekg(currentFilePointerPosition, std::ios::beg);

            // Read the package data.
            if(display_print)
            {
                std::cout << "---Package found Successful---- " << std::endl;
                std::cout << "Package id: " <<cmnPkgHdr_.id << std::endl;
                std::cout << "Package content size: " << cmnPkgHdr_.content_size << std::endl;
                std::cout << "Complete Package size: " << pkgEnd.pkg_size << std::endl;
                std::cout << "Pointer num: " << data_file_.tellg() <<" while exit" << std::endl;
            }

            return package;
        }

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

///// main

#include <cstring>
#include <iostream>
#include <chrono>

#include "DatLogger.hpp"

using namespace std::chrono;
using namespace DatLogger;

int main(int, char**){

    std::string fileName = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name = "e6mini.osgb";
    enum class Mode
    {
        WRITE = 0, // simple write
        READ = 1, // simple read
        LOG = 2,  // log packages
        RECORD =3, // record all packages in vector
        REPLAY = 4, // method to fetch package
        BOTH_SIMPLE = 5,
        BOTH = 6,
        LOG_PKG = 7,
        SIMPLE = 8 // isolated problem checking
    } ;
    Mode mode;
    mode = Mode::LOG;
    bool display_print = true;
    int version_ = 2;
    double current_time  = 1.09;

    DatLogger::DatLogger logger;
    if (mode == Mode::LOG)
    {
        // logger.init(fileName);

        double x = 1.0;
        double y = 2.0;
        double z = 3.0;
        double h = 4.0;
        double r = 5.0;
        double p = 6.0;

        float bb = 1.0;
        float bb1 = 2.0;
        int object_id = 1;
        double speed = 1.0;

        logger.init(fileName, version_, odrFileName, model_Name);

        for (int i = 0; i < 6 ; i++)
        {
            std::cout << "Iteration: " << i << std::endl;
            if (i > 0)
            {
                logger.isFirstEntry = false;
            }

            if (i == 4 || i == 5)
            {
                h = 6.0;
            }

            auto now = high_resolution_clock::now();
            typedef std::chrono::duration<double> duration_type;
            duration_type time_since_epoch = duration_cast<duration_type>(now.time_since_epoch());
            double current_time = time_since_epoch.count();

            // time package
            CommonPkg timePkg;

            timePkg.hdr.id = static_cast<int>(PackageId::TIME_SERIES);
            timePkg.hdr.content_size = sizeof(current_time);
            timePkg.content = reinterpret_cast<char*>(&current_time);
            timePkg.end.pkg_size = sizeof(CommonPkgHdr) + timePkg.hdr.content_size + sizeof(CommonPkgEnd);
            logger.logPackage(timePkg, object_id);

            // Obj id package
            CommonPkg pkgObjId;
            pkgObjId.hdr.id = static_cast<int>(PackageId::OBJ_ID);
            pkgObjId.hdr.content_size = sizeof(object_id);
            pkgObjId.content = reinterpret_cast<char*>(&object_id);
            pkgObjId.end.pkg_size = sizeof(CommonPkgHdr) + pkgObjId.hdr.content_size + sizeof(CommonPkgEnd);
            logger.logPackage(pkgObjId, object_id);

            // Pos package
            Pos pos_;
            CommonPkg packagePos;
            packagePos.hdr.id = static_cast<int>(PackageId::POSITIONS);
            pos_.x = x;
            pos_.y = y;
            pos_.z = z;
            pos_.h = h;
            pos_.r = r;
            pos_.p = p;
            packagePos.hdr.content_size = sizeof(pos_);
            packagePos.content = reinterpret_cast<char*>(&pos_);
            packagePos.end.pkg_size = sizeof(CommonPkgHdr) + packagePos.hdr.content_size + sizeof(CommonPkgEnd);
            logger.logPackage(packagePos, object_id);

            // Speed package
            std::cout << "speed: " << speed << std::endl;
            CommonPkg pkgSpeed;
            pkgSpeed.hdr.id = static_cast<int>(PackageId::SPEED);
            pkgSpeed.hdr.content_size = sizeof(speed);
            pkgSpeed.content = reinterpret_cast<char*>(&speed);
            pkgSpeed.end.pkg_size = sizeof(CommonPkgHdr) + pkgSpeed.hdr.content_size + sizeof(CommonPkgEnd);
            logger.logPackage(pkgSpeed, object_id);

            if (i != 3 )
            {
                speed += 1.0;
            }

            std::cout << "------------------------------------------------------" << i << std::endl;
        }

    }
    else if (mode == Mode::RECORD)
    {
        logger.recordPackage(fileName);
    }
    else if (mode == Mode::REPLAY)
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

        CommonPkg hdrPkgRead;

        // Read common header
        file_Read_.read(reinterpret_cast<char*>(&hdrPkgRead.hdr), sizeof(CommonPkgHdr));
        // hdrPkgRead.content = new char[hdrPkgRead.hdr.content_size];


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

        if(display_print){
        std::cout << "------Hdr Package->Read from file----- " << std::endl;
        std::cout << "Package id number: " << hdrPkgRead.hdr.id << std::endl;
        std::cout << "Package content size : " << hdrPkgRead.hdr.content_size << std::endl;
        std::cout << "Version : " << datHdrRead.version << std::endl;
        std::cout << "Odr content Size: " << odrStrRead.size << std::endl;
        std::cout << "Odr file name: " << odrStrRead.string << std::endl;
        std::cout << "Model content Size: " << mdlStrRead.size << std::endl;
        std::cout << "Model file name: " << mdlStrRead.string << std::endl;
        std::cout << "Package size end: " << hdrPkgRead.end.pkg_size << std::endl;
        std::cout << "------------------------- " << std::endl;
        }

        logger.pkgs_.push_back(hdrPkgRead);
        // Read from package
        DatHdr* datHdrPkg = reinterpret_cast<DatHdr*>(hdrPkgRead.content);
        int version = datHdrPkg->version;
        CommonString odrFilename = datHdrPkg->odrFilename;
        CommonString modelFilename = datHdrPkg->modelFilename;

        if(display_print){
        std::cout << "------Hdr Package->Read from Vector----- " << std::endl;
        std::cout << "Package id number: " << hdrPkgRead.hdr.id << std::endl;
        std::cout << "Package content size : " << hdrPkgRead.hdr.content_size << std::endl;
        std::cout << "Version : " << version << std::endl;
        std::cout << "Odr content Size: " << odrFilename.size << std::endl;
        std::cout << "Odr file name: " << odrFilename.string << std::endl;
        std::cout << "Model content Size: " << modelFilename.size << std::endl;
        std::cout << "Model file name: " << modelFilename.string << std::endl;
        std::cout << "Package size end: " << hdrPkgRead.end.pkg_size << std::endl;
        std::cout << "------------------------- " << std::endl;
        }

        // read time package

        CommonPkg timePkgRead;

        file_Read_.read(reinterpret_cast<char*>(&timePkgRead.hdr), sizeof(CommonPkgHdr));
        Time t;
        file_Read_.read(reinterpret_cast<char*>(&t.time), sizeof(CommonPkgHdr));
        file_Read_.read(reinterpret_cast<char*>(&timePkgRead.end.pkg_size), sizeof(CommonPkgEnd));
        timePkgRead.content = reinterpret_cast<char*>(&t.time);
        logger.pkgs_.push_back(timePkgRead);

        Time t_;
        t_.time = *reinterpret_cast<double*>(timePkgRead.content);

        if(display_print){
        std::cout << "------Time Package->Read from vector----- " << std::endl;
        std::cout << "Package id number: " << timePkgRead.hdr.id << std::endl;
        std::cout << "Package content size : " << timePkgRead.hdr.content_size << std::endl;
        std::cout << "Version : " << t_.time << std::endl;
        std::cout << "Package size end: " << timePkgRead.end.pkg_size << std::endl;
        std::cout << "------------------------- " << std::endl;
        }


    }
    else if (mode == Mode::WRITE)
    {
        std::ofstream file_;
        file_.open(fileName, std::ofstream::binary);
        if (file_.fail())
        {
            std::printf("Cannot open file: %s", fileName.c_str());
            return -1;
        }

        if (file_.is_open())
        {
            std::cout << "File Opened " << std::endl;
        }


        CommonPkgHdr cmnHdr;
        cmnHdr.id = static_cast<int>(PackageId::HEADER);

        CommonString odrStr;
        odrStr.size = odrFileName.size() + 1;
        odrStr.string = new char[odrStr.size];
        strncpy(odrStr.string, odrFileName.c_str(), odrStr.size);
        CommonString mdlStr;
        mdlStr.size = model_Name.size() + 1;
        mdlStr.string = new char[mdlStr.size];
        strncpy(mdlStr.string, model_Name.c_str(), mdlStr.size);

        cmnHdr.content_size = sizeof(odrStr.size) + odrStr.size + mdlStr.size + sizeof(mdlStr.size);

        DatHdr datHdr;
        datHdr.version = version_;
        datHdr.odrFilename = odrStr;
        datHdr.modelFilename = mdlStr;

        CommonPkg hdrPkg;
        hdrPkg.hdr = cmnHdr;
        hdrPkg.content = reinterpret_cast<char*>(&datHdr);
        hdrPkg.end.pkg_size = sizeof(CommonPkgHdr) + cmnHdr.content_size + sizeof(CommonPkgEnd);

        // write common header

        // file_.write(reinterpret_cast<char*>(&hdrPkg.hdr), sizeof(hdrPkg.hdr));
        file_.write(reinterpret_cast<char*>(&hdrPkg.hdr), sizeof(CommonPkgHdr));

        // write header package

        // write content -> version
        file_.write(reinterpret_cast<char*>(&datHdr.version), sizeof(datHdr.version));

        // write content -> odr filename size
        file_.write(reinterpret_cast<char*>(&datHdr.odrFilename.size), sizeof(datHdr.odrFilename.size));

        // write actual odr filename string
        file_.write(odrStr.string, datHdr.odrFilename.size);

        // write content -> model filename size
        file_.write(reinterpret_cast<char*>(&datHdr.modelFilename.size), sizeof(datHdr.modelFilename.size));

        // write actual model filename string
        file_.write(mdlStr.string, datHdr.modelFilename.size);

// #if 0
        // write packag
        file_.write(reinterpret_cast<char*>(&hdrPkg.end.pkg_size), sizeof(hdrPkg.end));
// #endif
        file_.flush();
        file_.close();

        if(display_print){
        std::cout << "------Hdr Package->Send----- " << std::endl;
        std::cout << "Package id number: " << hdrPkg.hdr.id << std::endl;
        std::cout << "Package content size : " << hdrPkg.hdr.content_size << std::endl;
        std::cout << "Version : " << datHdr.version << std::endl;
        std::cout << "Odr content Size: " << odrStr.size << std::endl;
        std::cout << "Odr file name: " << odrFileName.c_str() << std::endl;
        std::cout << "Model content Size: " << mdlStr.size << std::endl;
        std::cout << "Model file name: " << model_Name.c_str() << std::endl;
        std::cout << "Package size end: " << hdrPkg.end.pkg_size << std::endl;
        std::cout << "------------------------- " << std::endl;
        }

        if (!file_.is_open())
        {
            std::cout << "File closed " << std::endl;
        }
    }
    else if (mode == Mode::READ)
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

        CommonPkg hdrPkgRead;

        // Read common header
        file_Read_.read(reinterpret_cast<char*>(&hdrPkgRead.hdr), sizeof(CommonPkgHdr));
        // hdrPkgRead.content = new char[hdrPkgRead.hdr.content_size];


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

        // Read whole package size
        file_Read_.read(reinterpret_cast<char*>(&hdrPkgRead.end.pkg_size), sizeof(hdrPkgRead.end.pkg_size));

        if(display_print){
        std::cout << "------Hdr Package->Read----- " << std::endl;
        std::cout << "Package id number: " << hdrPkgRead.hdr.id << std::endl;
        std::cout << "Package content size : " << hdrPkgRead.hdr.content_size << std::endl;
        std::cout << "Version : " << datHdrRead.version << std::endl;
        std::cout << "Odr content Size: " << odrStrRead.size << std::endl;
        std::cout << "Odr file name: " << odrStrRead.string << std::endl;
        std::cout << "Model content Size: " << mdlStrRead.size << std::endl;
        std::cout << "Model file name: " << mdlStrRead.string << std::endl;
        std::cout << "Package size end: " << hdrPkgRead.end.pkg_size << std::endl;
        std::cout << "------------------------- " << std::endl;
        }


        // read time package
        CommonPkg timePkgRead;

        file_Read_.read(reinterpret_cast<char*>(&timePkgRead.hdr), sizeof(CommonPkgHdr));
        Time t;
        file_Read_.read(reinterpret_cast<char*>(&t.time), timePkgRead.hdr.content_size);
        file_Read_.read(reinterpret_cast<char*>(&timePkgRead.end.pkg_size), sizeof(CommonPkgEnd));


        if(display_print){
        std::cout << "------Time Package->Read----- " << std::endl;
        std::cout << "Package id number: " << timePkgRead.hdr.id << std::endl;
        std::cout << "Package content size : " << timePkgRead.hdr.content_size << std::endl;
        std::cout << "Time : " << t.time << std::endl;
        std::cout << "Package size end: " << timePkgRead.end.pkg_size << std::endl;
        std::cout << "------------------------- " << std::endl;
        }

        file_Read_.close();
        if (!file_Read_.is_open())
        {
            std::cout << "File closed After read" << std::endl;
        }
    }
    else if ( mode == Mode::BOTH_SIMPLE)
    {
        std::string fileName = "sim_both.dat";
        std::fstream data_file_both;
        std::ofstream data_file(fileName, std::ios::binary);
        data_file_both.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
        if (data_file_both.fail())
        {
            std::printf("Cannot open file: %s", fileName.c_str());
            return -1;
        }
        if (data_file_both.is_open()) {
            std::cout << "File opened successfully for both ." << std::endl;
        }

        char *stringTmp;
        int size = odrFileName.size() + 1;
        stringTmp = new char[size];
        strncpy(stringTmp, odrFileName.c_str(), size);
        std::cout << "Wrote content: " << odrFileName.c_str()<< std::endl;
        data_file_both.write(stringTmp, size);

        data_file_both.seekg(0, std::ios::beg);

        char *stringRead;
        int sizeRead = odrFileName.size();
        stringRead = new char[sizeRead];
        data_file_both.read(stringRead, sizeRead);
        std::cout << "Read content: " << stringRead<< std::endl;

        data_file_both.flush();
        data_file_both.close();

        if (!data_file_both.is_open())
        {
            std::cout << "File closed after read and write" << std::endl;
        }

    }
    else if (mode == Mode::LOG_PKG)
    {
        std::ofstream file_;
        std::string fileName = "sim_pkg.dat";
        file_.open(fileName, std::ofstream::binary);
        if (file_.fail())
        {
            std::printf("Cannot open file: %s", fileName.c_str());
            return -1;
        }

        if (file_.is_open())
        {
            std::cout << "File Opened " << std::endl;
        }


        typedef struct
        {
            int id;
            int size;

        } ModelHdr;

        typedef struct
        {
            ModelHdr hdr;
            char *stringTmp;
            int pkg_size;

        } ModelPkg;

        ModelPkg pkg;


        // char *stringTmp;
        pkg.hdr.id = 1;
        // int size_all;
        pkg.hdr.size = 12;
        // int size = odr_fileName.size() + 1;
        pkg.pkg_size = pkg.hdr.size + sizeof(ModelHdr) + sizeof(pkg.pkg_size);
        pkg.stringTmp = new char[pkg.hdr.size];
        file_.write(reinterpret_cast<char*>(&pkg.hdr.id), sizeof(int));
        file_.write(reinterpret_cast<char*>(&pkg.hdr.size), sizeof(int));
        strncpy(pkg.stringTmp, odrFileName.c_str(), pkg.hdr.size);
        file_.write(pkg.stringTmp, pkg.hdr.size);
        file_.write(reinterpret_cast<char*>(&pkg.pkg_size), sizeof(pkg.pkg_size));
        file_.flush();
        file_.close();

        if (!file_.is_open())
        {
            std::cout << "File closed " << std::endl;
        }
    }

    else if (mode == Mode::SIMPLE)
    {
        std::ofstream file_;
        std::string fileName = "sim.dat";
        file_.open(fileName, std::ofstream::binary);
        if (file_.fail())
        {
            std::printf("Cannot open file: %s", fileName.c_str());
            return -1;
        }

        if (file_.is_open())
        {
            std::cout << "File Opened " << std::endl;
        }


        CommonPkg timePkg;

        timePkg.hdr.id = static_cast<int>(PackageId::TIME_SERIES);
        timePkg.hdr.content_size = sizeof(current_time);
        timePkg.content = reinterpret_cast<char*>(&current_time);
        timePkg.end.pkg_size = sizeof(CommonPkgHdr) + timePkg.hdr.content_size + sizeof(CommonPkgEnd);

        file_.write(reinterpret_cast<char*>(&timePkg.hdr), sizeof(CommonPkgHdr));
        file_.write(timePkg.content, timePkg.hdr.content_size);
        file_.write(reinterpret_cast<char*>(&timePkg.end.pkg_size), sizeof(CommonPkgEnd));

        file_.flush();
        file_.close();

        if (!file_.is_open())
        {
            std::cout << "File closed " << std::endl;
        }
    }

}
