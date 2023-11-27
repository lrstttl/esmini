// logDat

#include <iostream>
#include <iostream>
#include <fstream>
#include <cstring>
#include <chrono>
#include <cmath>

#include "DatLogger.hpp"

using namespace datLogger;

#define SMALL_NUMBER 1e-6


/*#if (0)
void DatLogger::step(int obj_no)
{
    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    double h = 4.0;
    double r = 5.0;
    double p = 6.0;
    double speed = 1.0;
    double current_time = 0.033;

    for (int i = 0; i < 6 ; i++)
    {
        if (i > 0)
        {
            isFirstEntry = false;
        }

        if (i == 4 || i == 5)
        {
            h = 6.0;
        }


        // time package
        CommonPkg timePkg;

        timePkg.hdr.id = static_cast<int>(PackageId::TIME_SERIES);
        timePkg.hdr.content_size = sizeof(current_time);
        timePkg.content = reinterpret_cast<char*>(&current_time);
        timePkg.end.pkg_size = sizeof(CommonPkgHdr) + timePkg.hdr.content_size + sizeof(CommonPkgEnd);
        logPackage(timePkg, 0);

        for ( size_t j = 0; j < obj_no; j++)
        {
            if ( i == 2 && j == 2)
            {
                break; // delete one object.
            }
            int object_id = j;
            // Obj id package
            CommonPkg pkgObjId;
            pkgObjId.hdr.id = static_cast<int>(PackageId::OBJ_ID);
            pkgObjId.hdr.content_size = sizeof(object_id);
            pkgObjId.content = reinterpret_cast<char*>(&object_id);
            pkgObjId.end.pkg_size = sizeof(CommonPkgHdr) + pkgObjId.hdr.content_size + sizeof(CommonPkgEnd);
            logPackage(pkgObjId, object_id);

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
            logPackage(packagePos, object_id);

            // Speed package
            CommonPkg pkgSpeed;
            pkgSpeed.hdr.id = static_cast<int>(PackageId::SPEED);
            pkgSpeed.hdr.content_size = sizeof(speed);
            pkgSpeed.content = reinterpret_cast<char*>(&speed);
            pkgSpeed.end.pkg_size = sizeof(CommonPkgHdr) + pkgSpeed.hdr.content_size + sizeof(CommonPkgEnd);
            logPackage(pkgSpeed, object_id);

        }
        if (i != 3 )
        {
            speed += 1.0;
        }
        current_time += 1.089;
    }

}
#endif*/

int DatLogger::WriteObjSpeed(int obj_id, double speed)
{
    totalPkgReceived += 1;
    for (size_t i = 0; i < completeObjectState.obj_states.size(); i ++)
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
            pkg.hdr.id = static_cast<int>(PackageId::SPEED);
            pkg.hdr.content_size = sizeof(speed);
            pkg.content = reinterpret_cast<char*>(&speed);
            writePackage(pkg);
            completeObjectState.obj_states[i].speed_.speed_= speed;
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
            pkg.hdr.id = static_cast<int>(PackageId::TIME_SERIES);
            pkg.hdr.content_size = sizeof(t);
            pkg.content = reinterpret_cast<char*>(&t);
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
        pkg.hdr.id = static_cast<int>(PackageId::OBJ_ID);
        pkg.hdr.content_size = sizeof(obj_id);
        pkg.content = reinterpret_cast<char*>(&obj_id);
        writePackage(pkg);

        if (completeObjectState.obj_states.size() == 0) // first time
        {
            ObjState objState_;
            objState_.obj_id_.obj_id = obj_id;
            completeObjectState.obj_states.push_back(objState_);
        }
        else
        {
            for (size_t i = 0; i < completeObjectState.obj_states.size(); i ++)
            {
                if (completeObjectState.obj_states[i].obj_id_.obj_id == obj_id)
                {
                    break; // object already available
                }
                if ( i ==  completeObjectState.obj_states.size() - 1) // reached last iteration so add object
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
        for (size_t i = 0; i < completeObjectState.obj_states.size(); i ++)
        {
            if (completeObjectState.obj_states[i].obj_id_.obj_id != obj_id)
            {
                continue;
            }
            if (completeObjectState.obj_states[i].pos_.h != h ||
                completeObjectState.obj_states[i].pos_.p != p ||
                completeObjectState.obj_states[i].pos_.r != r ||
                completeObjectState.obj_states[i].pos_.x != x ||
                completeObjectState.obj_states[i].pos_.y != y ||
                completeObjectState.obj_states[i].pos_.z != z)
            {
                posPkgs += 1;
                // create pkg
                CommonPkg pkg;
                Pos pos_ ;
                pos_.x = x;
                pos_.y = y;
                pos_.z = z;
                pos_.h = h;
                pos_.r = r;
                pos_.p = p;

                pkg.hdr.id = static_cast<int>(PackageId::POSITIONS);
                pkg.hdr.content_size = sizeof(pos_);
                pkg.content = reinterpret_cast<char*>(&pos_);
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

void DatLogger::writePackage(CommonPkg package )
{
    if (data_file_.is_open())
    {
        if (display_print) {
        std::cout << "New Package->written package name : " << pkgIdTostring( static_cast<PackageId>(package.hdr.id)) << std::endl;
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

/*void DatLogger::logPackage(CommonPkg package, const int object_id)
{
    totalPkgReceived += 1;
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
                if (display_print) {
                std::cout << "Same as pervious->Skipped Package name : " << pkgIdTostring( static_cast<PackageId>(package.hdr.id)) << std::endl;
                }
                totalPkgSkipped += 1;
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
                if (display_print) {
                std::cout << "Same as pervious->Skipped Package name : " << pkgIdTostring( static_cast<PackageId>(package.hdr.id)) << std::endl;
                }
                totalPkgSkipped += 1;
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
}*/

int DatLogger::recordPackage(const std::string& fileName)
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

    // Get the file size
    file_Read_.seekg(0, std::ios::end);
    std::streampos file_size = file_Read_.tellg();
    file_Read_.seekg(0, std::ios::beg);
    while (file_Read_.good())
    {
        if (file_Read_.tellg() == file_size)
        {
            break;
        }
        totalPkgReceived += 1;
        // read the header for every loop
        CommonPkgHdr  cmnHdrPkgRead;
        file_Read_.read(reinterpret_cast<char*>(&cmnHdrPkgRead), sizeof(CommonPkgHdr));
        switch (static_cast<PackageId>(cmnHdrPkgRead.id))
        {
            case PackageId::HEADER:
            {
                CommonPkg hdrPkgRead;
                hdrPkgRead.hdr = cmnHdrPkgRead;

                DatHdr* datHdrRead = new DatHdr;
                // Read content -> version
                file_Read_.read(reinterpret_cast<char*>(&datHdrRead->version), sizeof(DatHdr::version));

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

                datHdrRead->odrFilename = odrStrRead;
                datHdrRead->modelFilename = mdlStrRead;
                hdrPkgRead.content = reinterpret_cast<char*>(datHdrRead);

                pkgs_.push_back(hdrPkgRead);

                if (display_print) {
                std::cout << "New Package->Read package name : " << pkgIdTostring( static_cast<PackageId>(cmnHdrPkgRead.id)) << std::endl;
                }

                totalPkgProcessed += 1;
                break;
            }
            case PackageId::TIME_SERIES:
            {
                CommonPkg timePkgRead;
                timePkgRead.hdr = cmnHdrPkgRead;

                Time* t = new Time;
                file_Read_.read(reinterpret_cast<char*>(&t->time), timePkgRead.hdr.content_size);
                timePkgRead.content = reinterpret_cast<char*>(t);
                pkgs_.push_back(timePkgRead);
                if (display_print) {
                std::cout << "New Package->Read package name : " << pkgIdTostring( static_cast<PackageId>(cmnHdrPkgRead.id)) << std::endl;
                }
                totalPkgProcessed += 1;
                break;
            }

            case PackageId::OBJ_ID:
            {
                CommonPkg objIdPkgRead;
                objIdPkgRead.hdr = cmnHdrPkgRead;

                ObjId* objIdRead = new ObjId;
                file_Read_.read(reinterpret_cast<char*>(&objIdRead->obj_id), objIdPkgRead.hdr.content_size);
                objIdPkgRead.content = reinterpret_cast<char*>(objIdRead);
                pkgs_.push_back(objIdPkgRead);
                if (display_print) {
                std::cout << "New Package->Read package name : " << pkgIdTostring( static_cast<PackageId>(cmnHdrPkgRead.id)) << std::endl;
                }
                totalPkgProcessed += 1;
                break;
            }

            case PackageId::POSITIONS:
            {

                CommonPkg posPkgRead;
                posPkgRead.hdr = cmnHdrPkgRead;

                Pos* posRead = new Pos;
                // file_Read_.read(reinterpret_cast<char*>(&posRead), posPkgRead.hdr.content_size);
                file_Read_.read(reinterpret_cast<char*>(&posRead->x), sizeof(Pos::x));
                file_Read_.read(reinterpret_cast<char*>(&posRead->y), sizeof(Pos::y));
                file_Read_.read(reinterpret_cast<char*>(&posRead->z), sizeof(Pos::z));
                file_Read_.read(reinterpret_cast<char*>(&posRead->h), sizeof(Pos::h));
                file_Read_.read(reinterpret_cast<char*>(&posRead->r), sizeof(Pos::r));
                file_Read_.read(reinterpret_cast<char*>(&posRead->p), sizeof(Pos::p));
                posPkgRead.content = reinterpret_cast<char*>(posRead);
                pkgs_.push_back(posPkgRead);
                if (display_print) {
                std::cout << "New Package->Read package name : " << pkgIdTostring( static_cast<PackageId>(cmnHdrPkgRead.id)) << std::endl;
                }
                totalPkgProcessed += 1;
                break;
            }

            case PackageId::SPEED:
            {
                CommonPkg speedPkgRead;
                speedPkgRead.hdr = cmnHdrPkgRead;

                Speed* SpeedRead = new Speed;
                file_Read_.read(reinterpret_cast<char*>(&SpeedRead->speed_), speedPkgRead.hdr.content_size);
                speedPkgRead.content = reinterpret_cast<char*>(SpeedRead);
                pkgs_.push_back(speedPkgRead);
                if (display_print) {
                std::cout << "New Package->Read package name : " << pkgIdTostring( static_cast<PackageId>(cmnHdrPkgRead.id)) << std::endl;
                }
                totalPkgProcessed += 1;
                break;
            }
            default:
            {
                totalPkgSkipped += 1;
                std::cout << "Unknown package read->package id :" << std::endl;
                break;
            }
        }
    }
    return 0;
}

PackageId DatLogger::readPkgHdr(char* package )
{
    CommonPkg pkg;
    pkg = *reinterpret_cast<CommonPkg*>(package);
    // std::cout << "Found package ID from current state: " << pkgIdTostring( static_cast<PackageId>(pkg.hdr.id)) << std::endl;
    return static_cast<PackageId>(pkg.hdr.id);
}

int DatLogger::getPkgCntBtwObj( size_t idx)
{
    int count = 0;
    for (size_t i = idx + 1; i < pkgs_.size(); i++) // start looking from next package
    {
        if (static_cast<PackageId>(pkgs_[i].hdr.id) == PackageId::TIME_SERIES ||
            static_cast<PackageId>(pkgs_[i].hdr.id) == PackageId::OBJ_ID)
        {
            break; // stop looking if time or obj id package found
        }
        count += 1; // count package
    }
    return count;
}

std::vector<int> DatLogger::GetNumberOfObjectsAtTime( double t)
{
    std::vector<int> Indices;
    bool timeFound = false;

    for (size_t i = 0; i < pkgs_.size(); i++)
    {
        if (pkgs_[i].hdr.id == static_cast<int>(PackageId::TIME_SERIES) && !timeFound)
        {
            double timeTemp = *reinterpret_cast<double*>(pkgs_[i].content);
            if (timeTemp == t)
            {
                timeFound = true;
            }
            continue; // continue till time match found. if time matched then
        }
        if (timeFound && static_cast<PackageId>(pkgs_[i].hdr.id) == PackageId::OBJ_ID)
        {
            Indices.push_back(static_cast<int>(i)); // time matches
        }
        if (pkgs_[i].hdr.id == static_cast<int>(PackageId::TIME_SERIES))
        {
            return Indices; // second time instances
        }
    }
    return Indices;
}

bool areApproximatelyEqual(double d1, double d2)
{
    const double TOLERANCE = 0.0001;  // Tolerance value for approximate equality
    double absoluteDifference = fabs(d1 - d2);
    return absoluteDifference <= TOLERANCE;
}


int DatLogger::searchAndReplacePkg(int idx1, int idx2, int idx3, double t)
{
    // lets take idx3 as a first reference ptr
    int ptr = idx3;

    // read the pkg id to find
    PackageId pkgIdToFind = readPkgHdr(scenarioState.obj_states[static_cast<size_t>(idx1)].pkgs[static_cast<size_t>(idx2)].pkg);

    // idx3 is the index where the obj id package found
    int objIdToFind = *reinterpret_cast<int*>(pkgs_[static_cast<size_t>(idx3)].content);

    // last time when this pkg has updated.
    double refTime = scenarioState.obj_states[static_cast<size_t>(idx1)].pkgs[static_cast<size_t>(idx2)].time_;

    if ( t == refTime)
    { // already latest pkg
        return 0;
    }

    // find the index for immediate time frame.(can be next or pervious one)
    // find next or immediate time frame
    int idxDir = 0 ;
    if ( t > refTime) // search forward
    {
        idxDir = -1; // forward
    }
    else if (t < refTime) // search backward
    {
        idxDir = 1; // backward
    }

    // start idx3 as reference to start backward or forward search
    while(true)
    {
        ptr += idxDir;
        if ( pkgs_[static_cast<size_t>(ptr)].hdr.id == static_cast<int>(PackageId::OBJ_ID))
        {
            if ( objIdToFind == *reinterpret_cast<int*>(pkgs_[static_cast<size_t>(ptr)].content)) // objIdToFind found
            {
                // now find the package
                int ptrTemp = 1;
                while(true)
                {
                    if ( static_cast<int>(pkgIdToFind) == pkgs_[static_cast<size_t>(ptr + ptrTemp)].hdr.id)
                    { // found looking package finally. now replace
                        scenarioState.obj_states[static_cast<size_t>(idx1)].pkgs[static_cast<size_t>(idx2)].pkg = reinterpret_cast<char*>(&pkgs_[static_cast<size_t>(ptr + ptrTemp)]);
                        scenarioState.obj_states[static_cast<size_t>(idx1)].pkgs[static_cast<size_t>(idx2)].time_ = getTimeFromPkgIdx(static_cast<size_t>(ptr + ptrTemp));
                        return 1;
                    }
                    if (pkgs_[static_cast<size_t>(ptr + ptrTemp)].hdr.id == static_cast<int>(PackageId::OBJ_ID) ||
                        pkgs_[static_cast<size_t>(ptr + ptrTemp)].hdr.id == static_cast<int>(PackageId::TIME_SERIES)) // already reached next time frame or next obj id
                    {
                        break; // go one step above
                    }

                    ptrTemp += 1;
                }

            }
        }
        else if( pkgs_[static_cast<size_t>(ptr)].hdr.id == static_cast<int>(PackageId::TIME_SERIES) )
        {
            // check last updated time frame reached
            double foundTime =  *reinterpret_cast<double*>(pkgs_[static_cast<size_t>(ptr)].content);
            if (areApproximatelyEqual(foundTime, refTime)) // ref time frame reached
            {
                return 1; // stop searching
            }
        }
    }
}

bool DatLogger::isObjAvailableInCache(int idx) // check in current state
{
    bool status = false;
    for (size_t i = 0; i < scenarioState.obj_states.size(); i++) // loop current state object id to find the object id
    {
        if (scenarioState.obj_states[i].id == *reinterpret_cast<int*>(pkgs_[static_cast<size_t>(idx)].content))
        {
            status = true; // obj id present
            break;
        }
    }
    return status;
}

bool DatLogger::isObjAvailable(int idx,  std::vector<int> Indices) // check in the object in the given new time
{
    bool status = false;
    for (size_t Index = 0; Index < Indices.size(); Index++) // loop found object ids for given new time to find the object id in current object state
    {
        if (scenarioState.obj_states[static_cast<size_t>(idx)].id == *reinterpret_cast<int*>(pkgs_[static_cast<size_t>(Indices[Index])].content))
        {
            status = true; // obj id present
            break;
        }
    }
    return status;
}

void DatLogger::MoveToTime(double t)
{
    if ( scenarioState.sim_time != t) // already current object is in given time
    {
        scenarioState.sim_time = t;

        std::vector<int> objIdIndices = GetNumberOfObjectsAtTime(t);

        for (size_t Index; Index < objIdIndices.size(); Index++)
        {
            if(isObjAvailableInCache(objIdIndices[Index]))
            {
                ObjectStateWithObjId stateObjId;
                stateObjId.id = *reinterpret_cast<int*>(pkgs_[static_cast<size_t>(objIdIndices[Index])].content);
                int pkgCnt = getPkgCntBtwObj(static_cast<size_t>(objIdIndices[Index]));
                for (size_t i = 0; i < scenarioState.obj_states.size(); i++) // loop current state object id to find the object id
                {
                    if (isObjAvailable(static_cast<int>(i), objIdIndices))
                    {
                        if (scenarioState.obj_states[i].id ==   stateObjId.id) // found object id
                        {
                            for (size_t j = 0;j < scenarioState.obj_states[i].pkgs.size(); j++) // loop current state states to find the correct package to replace
                            {
                                for ( size_t k = static_cast<size_t>(objIdIndices[Index] + 1);  k < static_cast<size_t>(objIdIndices[Index] + pkgCnt + 1); k++) // start with Index + 1, Index will have object id package index. looking from next package
                                {
                                    PackageId id_ = readPkgHdr(scenarioState.obj_states[i].pkgs[j].pkg);

                                    if (scenarioState.obj_states[i].pkgs[j].time_ == t ) // time matches so already new package in current state
                                    {
                                        break;
                                    }

                                    if (id_ == static_cast<PackageId>(pkgs_[k].hdr.id)) // package id matches in given time, replace it with new package
                                    {
                                        scenarioState.obj_states[i].pkgs[j].pkg = reinterpret_cast<char*>(&pkgs_[k]);
                                        scenarioState.obj_states[i].pkgs[j].time_ = t;
                                        break;
                                    }
                                    if ( k == static_cast<size_t>(objIdIndices[Index] + pkgCnt)) // reaches last iteration, only look for pervious time, No pkg match found in new time.
                                    {
                                        searchAndReplacePkg(static_cast<int>(i), static_cast<int>(j), objIdIndices[Index], t);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    else
                    { // object deleted
                        scenarioState.obj_states.erase(scenarioState.obj_states.begin() + static_cast<int>(i));
                        std::cout << "Object deleted" << std::endl;
                    }
                }
            }
            else
            {// object added
                addObjState(static_cast<size_t>(objIdIndices[Index]), t);
                std::cout << "Object added" << std::endl;
            }
        }
    }
}

double DatLogger::getTimeFromPkgIdx( size_t idx)
{
    double time_ = -1.0;
    for ( size_t i = idx; i > 0; i--) // return time if idx is already time pkg or find the respective time pkg for given pkg idx
    {
        if (static_cast<PackageId>(pkgs_[i].hdr.id) == PackageId::TIME_SERIES) // find time pkg for given pkg idx
        {
            time_ = *reinterpret_cast<double*>(pkgs_[i].content);
            break;
        }
    }
    return time_;
}

double DatLogger::getTimeFromCnt(int count)
{
    double time_ = -1.0;
    int count_ = 0;
    for ( size_t i = 0; i < pkgs_.size(); i++) // return time if idx is already time pkg or find the respective time pkg for given pkg idx
    {
        if (static_cast<PackageId>(pkgs_[i].hdr.id) == PackageId::TIME_SERIES) // find time pkg for given pkg idx
        {
            count_ += 1;
            if (count == count_)
            {
                time_ = *reinterpret_cast<double*>(pkgs_[i].content);
                break;
            }

        }
    }
    return time_;
}

void DatLogger::deleteObjState(int objId)
{
    for (size_t i = 0; i < completeObjectState.obj_states.size(); i++) // loop current state object id to find the object id
    {
        if (completeObjectState.obj_states[i].obj_id_.obj_id == objId) // found object id
        { // delete now
            completeObjectState.obj_states.erase(completeObjectState.obj_states.begin() + static_cast<int>(i));
        }
    }

}

void DatLogger::addObjState(size_t idx, double t)
{
    ObjectStateWithObjId stateObjId;
    if (static_cast<PackageId>(pkgs_[idx].hdr.id) != PackageId::OBJ_ID)
    {
        std::cout << " Initialization error->Stop replay " << std::endl;
    }
    stateObjId.id = *reinterpret_cast<int*>(pkgs_[idx].content);
    int pkgCount = getPkgCntBtwObj(idx);

    for ( size_t i = idx + 1;  i < static_cast<size_t>(pkgCount) + idx + 1; i++)
    { // getPkgCntBtwObj will return count of package.
        ObjectStateWithPkg statePkg;
        statePkg.pkg = reinterpret_cast<char*>(&pkgs_[i]);
        statePkg.time_ = t;
        stateObjId.pkgs.push_back(statePkg);
    }
    scenarioState.obj_states.push_back(stateObjId);

}

void DatLogger::initiateStates(double t)
{

    std::vector<int> objIdIndices = GetNumberOfObjectsAtTime(t);

    if (objIdIndices.size() == 0)
    {
        // no obj found for given time, may be given time frame pkg not available.
        std::cout << " No obj found for given time, may be given time frame pkg not available. " << std::endl;
    }

    scenarioState.sim_time = t;
    for (size_t Index = 0; Index < objIdIndices.size(); Index++)
    {
        addObjState(static_cast<size_t>(objIdIndices[Index]), t);
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
    if (data_file_.is_open()) {
        std::cout << "File opened successfully." << std::endl;
    }

    CommonPkgHdr cmnHdr;
    cmnHdr.id = static_cast<int>(PackageId::HEADER);

    CommonString odrStr;
    odrStr.size = static_cast<int>(odrName.size() + 1);
    odrStr.string = new char[odrStr.size];
    strncpy(odrStr.string, odrName.c_str(), static_cast<size_t>(odrStr.size));
    CommonString mdlStr;
    mdlStr.size = static_cast<int>(modelName.size() + 1);
    mdlStr.string = new char[mdlStr.size];
    strncpy(mdlStr.string, modelName.c_str(), static_cast<size_t>(mdlStr.size));

    cmnHdr.content_size = static_cast<int>(sizeof(odrStr.size)) + odrStr.size + mdlStr.size + static_cast<int>(sizeof(mdlStr.size));

    DatHdr datHdr;
    datHdr.version = ver;
    datHdr.odrFilename = odrStr;
    datHdr.modelFilename = mdlStr;

    CommonPkg hdrPkg;
    hdrPkg.hdr = cmnHdr;
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
    if (display_print) {
    std::cout << "New Package->written package name : " << pkgIdTostring( static_cast<PackageId>(cmnHdr.id)) << std::endl;
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

/*
template <typename T>
T DatLogger::getLatestPackage(const int id, const unsigned long long pkgSize, const int contentSize, const int objId) {
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
    int lastReadTimePkgPtrPos = 0; // to jump to last read obj id package pointer
    int ptrsToMove = 0; // pointer size to move backward
    int currentObjID; // to jump to last read obj id package pointer
    bool firstTimePkgFound = false;

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
        if(display_print)
        {
        std::cout << "Package ID: " << pkgIdTostring(static_cast<PackageId>(cmnPkgHdr_.id)) << std::endl;
        }

        // continue till time package found
        if (cmnPkgHdr_.id != static_cast<int>(PackageId::TIME_SERIES) && !firstTimePkgFound)
        {
            // current pointer is end of content size field
            ptrsToMove = sizeof(cmnPkgHdr_);
            continue;
        }

        if (!firstTimePkgFound && cmnPkgHdr_.id == static_cast<int>(PackageId::TIME_SERIES))
        { // continue till pervious time package found

            firstTimePkgFound = true;
            // current pointer is end of content size field
            ptrsToMove = sizeof(cmnPkgHdr_);
            continue;
        }

        // continue till time package found, no given obj id found in pervious time so this is new obj
        if (cmnPkgHdr_.id == static_cast<int>(PackageId::TIME_SERIES))
        {
            // set ptr to correct position on return
            data_file_.seekg(currentFilePointerPosition, std::ios::beg);
            return T();
        }

        // look till obj id found
        if (cmnPkgHdr_.id != static_cast<int>(PackageId::OBJ_ID))
        {
            // set ptr to correct position
            ptrsToMove = sizeof(cmnPkgHdr_);
            continue;
        }

        // read content
        int obj_id;
        data_file_.read(reinterpret_cast<char*>(&obj_id), cmnPkgHdr_.content_size);

        // check the obj id match
        if (obj_id != objId)
        {
            // set ptr to correct position
            ptrsToMove = sizeof(cmnPkgHdr_) + cmnPkgHdr_.content_size;
            continue;
        }

        if(display_print)
        {
            std::cout << "Pointer number before stage 2: " << data_file_.tellg() << std::endl;
        }

        lastReadTimePkgPtrPos = data_file_.tellg() - (sizeof(cmnPkgHdr_) + cmnPkgHdr_.content_size);
        ptrsToMove = 0;
        // stage 2-> Find the package
        // object id found now look for package forward.Always obj id package content end will be current pointer position for stage2(seek time pkg content and end size)
        data_file_.seekg((sizeof(CommonPkgEnd)), std::ios::cur);
        if(display_print)
        {
            std::cout << "Pointer number in stage 2: " << data_file_.tellg() << std::endl;
        }

        while (!data_file_.eof() || data_file_.tellg() == std::streampos(lastReadTimePkgPtrPos))
        {
            // read header
            data_file_.read(reinterpret_cast<char*>(&cmnPkgHdr_), sizeof(cmnPkgHdr_));
            // skip package content.
            data_file_.seekg(cmnPkgHdr_.content_size, std::ios::cur);
            // read overall size
            data_file_.read(reinterpret_cast<char*>(&pkgEnd.pkg_size), sizeof(CommonPkgEnd));


            // go back to stage1.
            if (cmnPkgHdr_.id == static_cast<int>(PackageId::TIME_SERIES) ||
                cmnPkgHdr_.id == static_cast<int>(PackageId::OBJ_ID))
            {
                // Jump to last known obj id package.
                data_file_.seekg(lastReadTimePkgPtrPos, std::ios::beg);
                firstTimePkgFound = false;
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
*/
