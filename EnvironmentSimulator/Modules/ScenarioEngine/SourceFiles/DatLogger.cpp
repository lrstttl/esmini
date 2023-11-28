// logDat

#include <iostream>
#include <fstream>
#include <cstring>
#include <chrono>
#include <cmath>

#include "DatLogger.hpp"
#include "CommonMini.hpp"

using namespace datLogger;

int DatLogger::WriteObjSpeed(int obj_id, double speed)
{
    totalPkgReceived += 1;
    for (size_t i = 0; i < completeObjectState.obj_states.size(); i++)
    {
        if (completeObjectState.obj_states[i].obj_id_.obj_id != obj_id)
        {
            continue;
        }
        if (fabs(completeObjectState.obj_states[i].speed_.speed_ - speed) > SMALL_NUMBER)
        {
            speedPkgs += 1;
            // create pkg
            CommonPkg pkg;
            pkg.hdr.id           = static_cast<int>(PackageId::SPEED);
            pkg.hdr.content_size = sizeof(speed);
            pkg.content          = reinterpret_cast<char*>(&speed);
            writePackage(pkg);
            completeObjectState.obj_states[i].speed_.speed_ = speed;
            break;
        }
        else
        {
            totalPkgSkipped += 1;
            break;
        }
    }
    return 0;
}

int DatLogger::WriteModelId(int obj_id, int model_id)
{
    totalPkgReceived += 1;

    if (data_file_.is_open())
    {
        for (size_t i = 0; i < completeObjectState.obj_states.size(); i++)
        {
            if (completeObjectState.obj_states[i].obj_id_.obj_id != obj_id)
            {
                continue;
            }
            if (completeObjectState.obj_states[i].model_id != model_id)
            {
                // create pkg
                CommonPkg pkg;
                pkg.hdr.id           = static_cast<int>(PackageId::MODEL_ID);
                pkg.hdr.content_size = sizeof(model_id);
                pkg.content          = reinterpret_cast<char*>(&model_id);
                writePackage(pkg);
                completeObjectState.obj_states[i].model_id = model_id;
                break;
            }
            else
            {
                totalPkgSkipped += 1;
            }
        }
    }
    return 0;
}

int DatLogger::WriteTime(double t)
{
    totalPkgReceived += 1;
    timePkgs += 1;

    if (data_file_.is_open())
    {
        if (fabs(completeObjectState.time.time - t) > SMALL_NUMBER)
        {
            // create pkg
            CommonPkg pkg;
            pkg.hdr.id           = static_cast<int>(PackageId::TIME_SERIES);
            pkg.hdr.content_size = sizeof(t);
            pkg.content          = reinterpret_cast<char*>(&t);
            writePackage(pkg);
            completeObjectState.time.time = t;
        }
        else
        {
            totalPkgSkipped += 1;
        }
    }

    return 0;
}

int DatLogger::WriteObjId(int obj_id)
{
    if (data_file_.is_open())
    {
        totalPkgReceived += 1;
        objIdPkgs += 1;

        // create pkg
        CommonPkg pkg;
        pkg.hdr.id           = static_cast<int>(PackageId::OBJ_ID);
        pkg.hdr.content_size = sizeof(obj_id);
        pkg.content          = reinterpret_cast<char*>(&obj_id);
        writePackage(pkg);

        if (completeObjectState.obj_states.size() == 0)  // first time
        {
            ObjState objState_;
            objState_.obj_id_.obj_id = obj_id;
            completeObjectState.obj_states.push_back(objState_);
        }
        else
        {
            for (size_t i = 0; i < completeObjectState.obj_states.size(); i++)
            {
                if (completeObjectState.obj_states[i].obj_id_.obj_id == obj_id)
                {
                    break;  // object already available
                }
                if (i == completeObjectState.obj_states.size() - 1)  // reached last iteration so add object
                {
                    ObjState objState_;
                    objState_.obj_id_.obj_id = obj_id;
                    completeObjectState.obj_states.push_back(objState_);
                }
            }
        }
    }
    return 0;
}

int DatLogger::WriteObjPos(int obj_id, double x, double y, double z, double h, double p, double r)
{
    totalPkgReceived += 1;
    if (data_file_.is_open())
    {
        for (size_t i = 0; i < completeObjectState.obj_states.size(); i++)
        {
            if (completeObjectState.obj_states[i].obj_id_.obj_id != obj_id)
            {
                continue;
            }
            if (completeObjectState.obj_states[i].pos_.h != h || completeObjectState.obj_states[i].pos_.p != p ||
                completeObjectState.obj_states[i].pos_.r != r || completeObjectState.obj_states[i].pos_.x != x ||
                completeObjectState.obj_states[i].pos_.y != y || completeObjectState.obj_states[i].pos_.z != z)
            {
                posPkgs += 1;
                // create pkg
                CommonPkg pkg;
                Pos       pos_;
                pos_.x = x;
                pos_.y = y;
                pos_.z = z;
                pos_.h = h;
                pos_.r = r;
                pos_.p = p;

                pkg.hdr.id           = static_cast<int>(PackageId::POSITIONS);
                pkg.hdr.content_size = sizeof(pos_);
                pkg.content          = reinterpret_cast<char*>(&pos_);
                writePackage(pkg);
                completeObjectState.obj_states[i].pos_ = pos_;
                break;
            }
            else
            {
                totalPkgSkipped += 1;
                break;
            }
        }
    }
    return 0;
}

void DatLogger::writePackage(CommonPkg package)
{
    if (data_file_.is_open())
    {
        if (display_print)
        {
            std::cout << "New Package->written package name : " << pkgIdTostring(static_cast<PackageId>(package.hdr.id)) << std::endl;
        }
        data_file_.write(reinterpret_cast<char*>(&package.hdr), sizeof(CommonPkgHdr));
        data_file_.write(package.content, package.hdr.content_size);
        totalPkgProcessed += 1;
    }
    else
    {
        std::printf("File is not open:");
    }
}

void DatLogger::deleteObjState(int objId)
{
    for (size_t i = 0; i < completeObjectState.obj_states.size(); i++)  // loop current state object id to find the object id
    {
        if (completeObjectState.obj_states[i].obj_id_.obj_id == objId)  // found object id
        {                                                               // delete now
            completeObjectState.obj_states.erase(completeObjectState.obj_states.begin() + static_cast<int>(i));
        }
    }
}

int DatLogger::init(std::string fileName, int ver, std::string odrName, std::string modelName)
{
    totalPkgReceived += 1;
    std::ofstream data_file(fileName, std::ios::binary);
    data_file_.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    if (data_file_.fail())
    {
        std::printf("Cannot open file: %s", fileName.c_str());
        return -1;
    }
    if (data_file_.is_open())
    {
        std::cout << "File opened successfully." << std::endl;
    }

    CommonPkgHdr cmnHdr;
    cmnHdr.id = static_cast<int>(PackageId::HEADER);

    CommonString odrStr;
    odrStr.size   = static_cast<int>(odrName.size() + 1);
    odrStr.string = new char[odrStr.size];
    StrCopy(odrStr.string, odrName.c_str(), static_cast<size_t>(odrStr.size));
    CommonString mdlStr;
    mdlStr.size   = static_cast<int>(modelName.size() + 1);
    mdlStr.string = new char[mdlStr.size];
    StrCopy(mdlStr.string, modelName.c_str(), static_cast<size_t>(mdlStr.size));

    cmnHdr.content_size = static_cast<int>(sizeof(odrStr.size)) + odrStr.size + mdlStr.size + static_cast<int>(sizeof(mdlStr.size));

    DatHdr datHdr;
    datHdr.version       = ver;
    datHdr.odrFilename   = odrStr;
    datHdr.modelFilename = mdlStr;

    CommonPkg hdrPkg;
    hdrPkg.hdr     = cmnHdr;
    hdrPkg.content = reinterpret_cast<char*>(&datHdr);

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
    totalPkgProcessed += 1;
    if (display_print)
    {
        std::cout << "New Package->written package name : " << pkgIdTostring(static_cast<PackageId>(cmnHdr.id)) << std::endl;
    }

    return 0;
}

std::string DatLogger::pkgIdTostring(PackageId id)
{
    switch (id)
    {
        case PackageId::HEADER:
        {
            return "File_Header";
            break;
        }
        case PackageId::TIME_SERIES:
        {
            return "Time";
            break;
        }

        case PackageId::OBJ_ID:
        {
            return "Object_Id";
            break;
        }

        case PackageId::POSITIONS:
        {
            return "Position";
            break;
        }

        case PackageId::SPEED:
        {
            return "Speed";
            break;
        }
        default:
        {
            std::cout << "Unknown package read->package id :" << std::endl;
            return "none";
            break;
        }
    }
}
