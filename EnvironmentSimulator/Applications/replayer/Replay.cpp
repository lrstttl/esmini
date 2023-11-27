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

#include "Replay.hpp"
#include "ScenarioGateway.hpp"
#include "CommonMini.hpp"
#include "dirent.h"

using namespace scenarioengine;

Replay::Replay(std::string filename, bool clean) : time_(0.0), index_(0), repeat_(false), clean_(clean)
{
    file_.open(filename, std::ofstream::binary);
    if (file_.fail())
    {
        LOG("Cannot open file: %s", filename.c_str());
        throw std::invalid_argument(std::string("Cannot open file: ") + filename);
    }

    file_.read(reinterpret_cast<char*>(&header_), sizeof(header_));
    LOG("Recording %s opened. dat version: %d odr: %s model: %s",
        FileNameOf(filename).c_str(),
        header_.version,
        FileNameOf(header_.odr_filename).c_str(),
        FileNameOf(header_.model_filename).c_str());

    if (header_.version != DAT_FILE_FORMAT_VERSION)
    {
        LOG_AND_QUIT("Version mismatch. %s is version %d while supported version is %d. Please re-create dat file.",
                     filename.c_str(),
                     header_.version,
                     DAT_FILE_FORMAT_VERSION);
    }

    if (header_.version != DAT_FILE_FORMAT_VERSION)
    {
        LOG_AND_QUIT("Version mismatch. %s is version %d while supported version is %d. Please re-create dat file.",
                     filename.c_str(),
                     header_.version,
                     DAT_FILE_FORMAT_VERSION);
    }

    if (header_.version != 2)
    {
        LOG_AND_QUIT("Version mismatch. %s is version %d while supported version is %d. Please re-create dat file.",
                     filename.c_str(),
                     header_.version,
                     DAT_FILE_FORMAT_VERSION);
    }

    while (!file_.eof())
    {
        ReplayEntry data;

        file_.read(reinterpret_cast<char*>(&data.state), sizeof(data.state));

        if (!file_.eof())
        {
            data_.push_back(data);
        }
    }

    if (clean_)
    {
        CleanEntries(data_);
    }

    if (data_.size() > 0)
    {
        // Register first entry timestamp as starting time
        time_       = data_[0].state.info.timeStamp;
        startTime_  = time_;
        startIndex_ = 0;

        // Register last entry timestamp as stop time
        stopTime_  = data_.back().state.info.timeStamp;
        stopIndex_ = static_cast<unsigned int>(FindIndexAtTimestamp(stopTime_));
    }
}

Replay::Replay(const std::string directory, const std::string scenario, std::string create_datfile)
    : time_(0.0),
      index_(0),
      repeat_(false),
      create_datfile_(create_datfile)
{
    GetReplaysFromDirectory(directory, scenario);
    std::vector<std::pair<std::string, std::vector<ReplayEntry>>> scenarioData;

    for (size_t i = 0; i < scenarios_.size(); i++)
    {
        file_.open(scenarios_[i], std::ofstream::binary);
        if (file_.fail())
        {
            LOG("Cannot open file: %s", scenarios_[i].c_str());
            throw std::invalid_argument(std::string("Cannot open file: ") + scenarios_[i]);
        }
        file_.read(reinterpret_cast<char*>(&header_), sizeof(header_));
        LOG("Recording %s opened. dat version: %d odr: %s model: %s",
            FileNameOf(scenarios_[i]).c_str(),
            header_.version,
            FileNameOf(header_.odr_filename).c_str(),
            FileNameOf(header_.model_filename).c_str());

        if (header_.version != DAT_FILE_FORMAT_VERSION)
        {
            LOG_AND_QUIT("Version mismatch. %s is version %d while supported version is %d. Please re-create dat file.",
                         scenarios_[i].c_str(),
                         header_.version,
                         DAT_FILE_FORMAT_VERSION);
        }
        while (!file_.eof())
        {
            ReplayEntry entry;

            file_.read(reinterpret_cast<char*>(&entry.state), sizeof(entry.state));

            if (!file_.eof())
            {
                data_.push_back(entry);
            }
        }
        // pair <scenario name, scenario data>
        scenarioData.push_back(std::make_pair(scenarios_[i], data_));
        data_ = {};
        file_.close();
    }

    if (scenarioData.size() < 2)
    {
        LOG_AND_QUIT("Too few scenarios loaded, use single replay feature instead\n");
    }

    // Scenario with smallest start time first
    std::sort(scenarioData.begin(),
              scenarioData.end(),
              [](const auto& sce1, const auto& sce2) { return sce1.second[0].state.info.timeStamp < sce2.second[0].state.info.timeStamp; });

    // Log which scenario belongs to what ID-group (0, 100, 200 etc.)
    for (size_t i = 0; i < scenarioData.size(); i++)
    {
        std::string scenario_tmp = scenarioData[i].first;
        LOG("Scenarios corresponding to IDs (%d:%d): %s", i * 100, (i + 1) * 100 - 1, FileNameOf(scenario_tmp.c_str()).c_str());
    }

    // Ensure increasing timestamps. Remove any other entries.
    for (auto& sce : scenarioData)
    {
        CleanEntries(sce.second);
    }

    // Build remaining data in order.
    BuildData(scenarioData);

    if (data_.size() > 0)
    {
        // Register first entry timestamp as starting time
        time_       = data_[0].state.info.timeStamp;
        startTime_  = time_;
        startIndex_ = 0;

        // Register last entry timestamp as stop time
        stopTime_  = data_.back().state.info.timeStamp;
        stopIndex_ = static_cast<unsigned int>(FindIndexAtTimestamp(stopTime_));
    }

    if (!create_datfile_.empty())
    {
        CreateMergedDatfile(create_datfile_);
    }
}

// Browse through replay-folder and appends strings of absolute path to matching scenario
void Replay::GetReplaysFromDirectory(const std::string dir, const std::string sce)
{
    DIR* directory = opendir(dir.c_str());

    // If no directory found, write error
    if (directory == nullptr)
    {
        LOG_AND_QUIT("No valid directory given, couldn't open %s", dir.c_str());
    }

    // While directory is open, check the filename
    struct dirent* file;
    while ((file = readdir(directory)) != nullptr)
    {
        std::string filename = file->d_name;
        if (file->d_type == DT_DIR && filename.find(sce) != std::string::npos)
        {
            DIR* nested_dir = opendir((dir + filename).c_str());
            if (nested_dir == nullptr)
            {
                LOG("Couldn't open nested directory %s", (dir + filename).c_str());
            }

            struct dirent* nested_file;
            while ((nested_file = readdir(nested_dir)) != nullptr)
            {
                std::string nested_filename = nested_file->d_name;

                if (nested_filename != "." && nested_filename != ".." && nested_filename.find(sce) != std::string::npos &&
                    nested_filename.find(".dat") != std::string::npos)
                {
                    scenarios_.emplace_back(CombineDirectoryPathAndFilepath(dir + filename, nested_filename));
                }
            }
            closedir(nested_dir);
        }

        if (filename != "." && filename != ".." && filename.find(sce) != std::string::npos && filename.find(".dat") != std::string::npos)
        {
            scenarios_.emplace_back(CombineDirectoryPathAndFilepath(dir, filename));
        }
    }
    closedir(directory);

    // Sort list of filenames
    std::sort(scenarios_.begin(), scenarios_.end(), [](std::string const& a, std::string const& b) { return a < b; });

    if (scenarios_.empty())
    {
        LOG_AND_QUIT("Couldn't read any scenarios named %s in path %s", sce.c_str(), dir.c_str());
    }
}

size_t Replay::GetNumberOfScenarios()
{
    return scenarios_.size();
}

Replay::~Replay()
{
    data_.clear();
}

void Replay::GoToStart()
{
    index_ = startIndex_;
    time_  = startTime_;
}

void Replay::GoToEnd()
{
    if (repeat_)
    {
        index_ = startIndex_;
        time_  = startTime_;
    }
    else
    {
        index_ = stopIndex_;
        time_  = stopTime_;
    }
}

void Replay::GoToTime(double time, bool stop_at_next_frame)
{
    if (!stop_at_next_frame)
    {
        if (time > stopTime_)
        {
            GoToEnd();
        }
        else if (time < GetStartTime())
        {
            GoToStart();
        }
        else
        {
            index_ = static_cast<unsigned int>(FindIndexAtTimestamp(time, static_cast<int>(index_)));
            time_  = time;
        }
    }
    else
    {
        size_t next_index = index_;

        if (time > time_)
        {
            next_index = FindNextTimestamp();
            if (next_index > index_ && time > static_cast<double>(data_[next_index].state.info.timeStamp) &&
                static_cast<double>(data_[next_index].state.info.timeStamp) <= GetStopTime())
            {
                index_ = static_cast<unsigned int>(next_index);
                time_  = data_[index_].state.info.timeStamp;
            }
            else
            {
                if (time > GetStopTime())
                {
                    GoToEnd();
                }
                else
                {
                    time_ = time;
                }
            }
        }
        else if (time < time_)
        {
            next_index = FindPreviousTimestamp();
            if (next_index < index_ && time < static_cast<double>(data_[next_index].state.info.timeStamp))
            {
                index_ = static_cast<unsigned int>(next_index);
                time_  = data_[index_].state.info.timeStamp;
            }
            else
            {
                if (time < GetStartTime())
                {
                    GoToStart();
                }
                else
                {
                    time_ = time;
                }
            }
        }
    }
}

void Replay::GoToDeltaTime(double dt, bool stop_at_next_frame)
{
    GoToTime(time_ + dt, stop_at_next_frame);
}

int Replay::GoToNextFrame()
{
    float ctime = data_[index_].state.info.timeStamp;
    for (size_t i = index_ + 1; i < data_.size(); i++)
    {
        if (data_[i].state.info.timeStamp > ctime)
        {
            GoToTime(data_[i].state.info.timeStamp);
            return static_cast<int>(i);
        }
    }
    return -1;
}

void Replay::GoToPreviousFrame()
{
    if (index_ > 0)
    {
        GoToTime(data_[index_ - 1].state.info.timeStamp);
    }
}

int Replay::FindIndexAtTimestamp(double timestamp, int startSearchIndex)
{
    int i = 0;

    if (timestamp > stopTime_)
    {
        GoToEnd();
        return static_cast<int>(index_);
    }
    else if (timestamp < GetStartTime())
    {
        return static_cast<int>(index_);
    }

    if (timestamp < time_)
    {
        // start search from beginning
        startSearchIndex = 0;
    }

    for (i = startSearchIndex; i < static_cast<int>(data_.size()); i++)
    {
        if (static_cast<double>(data_[static_cast<unsigned int>(i)].state.info.timeStamp) >= timestamp)
        {
            break;
        }
    }

    return MIN(i, static_cast<int>(data_.size()) - 1);
}

unsigned int Replay::FindNextTimestamp(bool wrap)
{
    unsigned int index = index_ + 1;
    for (; index < data_.size(); index++)
    {
        if (data_[index].state.info.timeStamp > data_[index_].state.info.timeStamp)
        {
            break;
        }
    }

    if (index >= data_.size())
    {
        if (wrap)
        {
            return 0;
        }
        else
        {
            return index_;  // stay on current index
        }
    }

    return index;
}

unsigned int Replay::FindPreviousTimestamp(bool wrap)
{
    int index = static_cast<int>(index_) - 1;

    if (index < 0)
    {
        if (wrap)
        {
            index = static_cast<int>(data_.size()) - 1;
        }
        else
        {
            return 0;
        }
    }

    for (int i = index - 1; i >= 0; i--)
    {
        // go backwards until we identify the first entry with same timestamp
        if (data_[static_cast<unsigned int>(i)].state.info.timeStamp < data_[static_cast<unsigned int>(index)].state.info.timeStamp)
        {
            break;
        }
        index = i;
    }

    return static_cast<unsigned int>(index);
}

ReplayEntry* Replay::GetEntry(int id)
{
    // Read all vehicles at current timestamp
    float timestamp = data_[index_].state.info.timeStamp;
    int   i         = 0;
    while (index_ + static_cast<unsigned int>(i) < data_.size() && !(data_[index_ + static_cast<unsigned int>(i)].state.info.timeStamp > timestamp))
    {
        if (data_[index_ + static_cast<unsigned int>(i)].state.info.id == id)
        {
            return &data_[index_ + static_cast<unsigned int>(i)];
        }
        i++;
    }

    return nullptr;
}

ObjectStateStructDat* Replay::GetState(int id)
{
    ReplayEntry* entry = GetEntry(id);
    if (entry != nullptr)
    {
        return &entry->state;
    }
    else
    {
        return nullptr;
    }
}

void Replay::SetStartTime(double time)
{
    startTime_ = time;
    if (time_ < startTime_)
    {
        time_ = startTime_;
    }

    startIndex_ = static_cast<unsigned int>(FindIndexAtTimestamp(startTime_));
}

void Replay::SetStopTime(double time)
{
    stopTime_ = time;
    if (time_ > stopTime_)
    {
        time_ = stopTime_;
    }

    stopIndex_ = static_cast<unsigned int>(FindIndexAtTimestamp(stopTime_));
}

void Replay::CleanEntries(std::vector<ReplayEntry>& entries)
{
    for (unsigned int i = 0; i < entries.size() - 1; i++)
    {
        if (entries[i + 1].state.info.timeStamp < entries[i].state.info.timeStamp)
        {
            entries.erase(entries.begin() + i + 1);
            i--;
        }

        for (unsigned int j = 1; (i + j < entries.size()) && NEAR_NUMBERSF(entries[i + j].state.info.timeStamp, entries[i].state.info.timeStamp); j++)
        {
            // Keep the latest instance of entries with same timestamp
            if (entries[i + j].state.info.id == entries[i].state.info.id)
            {
                entries.erase(entries.begin() + i);
                i--;
                break;
            }
        }
    }
}

void Replay::BuildData(std::vector<std::pair<std::string, std::vector<ReplayEntry>>>& scenarios)
{
    // Keep track of current index of each scenario
    std::vector<int> cur_idx;
    std::vector<int> next_idx;

    for (size_t j = 0; j < scenarios.size(); j++)
    {
        cur_idx.push_back(0);
        next_idx.push_back(0);
    }

    // Set scenario ID-group (0, 100, 200 etc.)
    for (size_t j = 0; j < scenarios.size(); j++)
    {
        for (size_t k = 0; k < scenarios[j].second.size(); k++)
        {
            // Set scenario ID-group (0, 100, 200 etc.)
            scenarios[j].second[k].state.info.id += static_cast<int>(j) * 100;
        }
    }

    // Populate data_ based on first (with lowest timestamp) scenario
    double cur_timestamp = static_cast<double>(scenarios[0].second[0].state.info.timeStamp);
    while (cur_timestamp < LARGE_NUMBER - SMALL_NUMBER)
    {
        // populate entries if all scenarios at current time step
        double min_time_stamp = LARGE_NUMBER;
        for (size_t j = 0; j < scenarios.size(); j++)
        {
            if (next_idx[j] != -1)
            {
                unsigned int k = static_cast<unsigned int>(cur_idx[j]);
                for (; k < scenarios[j].second.size() && static_cast<double>(scenarios[j].second[k].state.info.timeStamp) < cur_timestamp + 1e-6; k++)
                {
                    // push entry with modified timestamp
                    scenarios[j].second[k].state.info.timeStamp = static_cast<float>(cur_timestamp);
                    data_.push_back(scenarios[j].second[k]);
                }

                if (k < scenarios[j].second.size())
                {
                    next_idx[j] = static_cast<int>(k);
                    if (static_cast<double>(scenarios[j].second[k].state.info.timeStamp) < min_time_stamp)
                    {
                        min_time_stamp = static_cast<double>(scenarios[j].second[k].state.info.timeStamp);
                    }
                }
                else
                {
                    next_idx[j] = -1;
                }
            }
        }

        if (min_time_stamp < LARGE_NUMBER - SMALL_NUMBER)
        {
            for (size_t j = 0; j < scenarios.size(); j++)
            {
                if (next_idx[j] > 0 && static_cast<double>(scenarios[j].second[static_cast<unsigned int>(next_idx[j])].state.info.timeStamp) <
                                           min_time_stamp + SMALL_NUMBER)
                {
                    // time has reached next entry, step this scenario
                    cur_idx[j] = next_idx[j];
                }
            }
        }

        cur_timestamp = min_time_stamp;
    }
}

void Replay::CreateMergedDatfile(const std::string filename)
{
    std::ofstream data_file_;
    data_file_.open(filename, std::ofstream::binary);
    if (data_file_.fail())
    {
        LOG("Cannot open file: %s", filename.c_str());
        exit(-1);
    }

    data_file_.write(reinterpret_cast<char*>(&header_), sizeof(header_));

    if (data_file_.is_open())
    {
        // Write status to file - for later replay
        for (size_t i = 0; i < data_.size(); i++)
        {
            data_file_.write(reinterpret_cast<char*>(&data_[i].state), sizeof(data_[i].state));
        }
    }
}

// new replayer

int Replay::recordPackage(const std::string& fileName)
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
        // read the header for every loop
        datLogger::CommonPkgHdr  cmnHdrPkgRead;
        file_Read_.read(reinterpret_cast<char*>(&cmnHdrPkgRead), sizeof(datLogger::CommonPkgHdr));
        switch (static_cast<datLogger::PackageId>(cmnHdrPkgRead.id))
        {
            case datLogger::PackageId::HEADER:
            {
                datLogger::CommonPkg hdrPkgRead;
                hdrPkgRead.hdr = cmnHdrPkgRead;

                datLogger::DatHdr* datHdrRead = new datLogger::DatHdr;
                // Read content -> version
                file_Read_.read(reinterpret_cast<char*>(&datHdrRead->version), sizeof(datLogger::DatHdr::version));

                datLogger::CommonString odrStrRead;
                // Read content -> odr filename size
                file_Read_.read(reinterpret_cast<char*>(&odrStrRead.size), sizeof(odrStrRead.size));

                // Read content -> odr filename string
                odrStrRead.string = new char[odrStrRead.size];
                file_Read_.read(odrStrRead.string, odrStrRead.size);

                datLogger::CommonString mdlStrRead;
                // Read content -> model filename size
                file_Read_.read(reinterpret_cast<char*>(&mdlStrRead.size), sizeof(mdlStrRead.size));

                // Read content -> model filename string
                mdlStrRead.string = new char[mdlStrRead.size];
                file_Read_.read(mdlStrRead.string, mdlStrRead.size);

                datHdrRead->odrFilename = odrStrRead;
                datHdrRead->modelFilename = mdlStrRead;
                hdrPkgRead.content = reinterpret_cast<char*>(datHdrRead);

                pkgs_.push_back(hdrPkgRead);

                break;
            }
            case datLogger::PackageId::TIME_SERIES:
            {
                datLogger::CommonPkg timePkgRead;
                timePkgRead.hdr = cmnHdrPkgRead;

                datLogger::Time* t = new datLogger::Time;
                file_Read_.read(reinterpret_cast<char*>(&t->time), timePkgRead.hdr.content_size);
                timePkgRead.content = reinterpret_cast<char*>(t);
                pkgs_.push_back(timePkgRead);
                break;
            }

            case datLogger::PackageId::OBJ_ID:
            {
                datLogger::CommonPkg objIdPkgRead;
                objIdPkgRead.hdr = cmnHdrPkgRead;

                datLogger::ObjId* objIdRead = new datLogger::ObjId;
                file_Read_.read(reinterpret_cast<char*>(&objIdRead->obj_id), objIdPkgRead.hdr.content_size);
                objIdPkgRead.content = reinterpret_cast<char*>(objIdRead);
                pkgs_.push_back(objIdPkgRead);
                break;
            }

            case datLogger::PackageId::POSITIONS:
            {

                datLogger::CommonPkg posPkgRead;
                posPkgRead.hdr = cmnHdrPkgRead;

                datLogger::Pos* posRead = new datLogger::Pos;
                // file_Read_.read(reinterpret_cast<char*>(&posRead), posPkgRead.hdr.content_size);
                file_Read_.read(reinterpret_cast<char*>(&posRead->x), sizeof(datLogger::Pos::x));
                file_Read_.read(reinterpret_cast<char*>(&posRead->y), sizeof(datLogger::Pos::y));
                file_Read_.read(reinterpret_cast<char*>(&posRead->z), sizeof(datLogger::Pos::z));
                file_Read_.read(reinterpret_cast<char*>(&posRead->h), sizeof(datLogger::Pos::h));
                file_Read_.read(reinterpret_cast<char*>(&posRead->r), sizeof(datLogger::Pos::r));
                file_Read_.read(reinterpret_cast<char*>(&posRead->p), sizeof(datLogger::Pos::p));
                posPkgRead.content = reinterpret_cast<char*>(posRead);
                pkgs_.push_back(posPkgRead);
                break;
            }

            case datLogger::PackageId::SPEED:
            {
                datLogger::CommonPkg speedPkgRead;
                speedPkgRead.hdr = cmnHdrPkgRead;

                datLogger::Speed* SpeedRead = new datLogger::Speed;
                file_Read_.read(reinterpret_cast<char*>(&SpeedRead->speed_), speedPkgRead.hdr.content_size);
                speedPkgRead.content = reinterpret_cast<char*>(SpeedRead);
                pkgs_.push_back(speedPkgRead);
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

datLogger::PackageId Replay::readPkgHdr(char* package )
{
    datLogger::CommonPkg pkg;
    pkg = *reinterpret_cast<datLogger::CommonPkg*>(package);
    // std::cout << "Found package ID from current state: " << pkgIdTostring( static_cast<PackageId>(pkg.hdr.id)) << std::endl;
    return static_cast<datLogger::PackageId>(pkg.hdr.id);
}

int Replay::getPkgCntBtwObj( size_t idx)
{
    int count = 0;
    for (size_t i = idx + 1; i < pkgs_.size(); i++) // start looking from next package
    {
        if (static_cast<datLogger::PackageId>(pkgs_[i].hdr.id) == datLogger::PackageId::TIME_SERIES ||
            static_cast<datLogger::PackageId>(pkgs_[i].hdr.id) == datLogger::PackageId::OBJ_ID)
        {
            break; // stop looking if time or obj id package found
        }
        count += 1; // count package
    }
    return count;
}

std::vector<int> Replay::GetNumberOfObjectsAtTime( double t)
{
    std::vector<int> Indices;
    bool timeFound = false;

    for (size_t i = 0; i < pkgs_.size(); i++)
    {
        if (pkgs_[i].hdr.id == static_cast<int>(datLogger::PackageId::TIME_SERIES) && !timeFound)
        {
            double timeTemp = *reinterpret_cast<double*>(pkgs_[i].content);
            if (timeTemp == t)
            {
                timeFound = true;
            }
            continue; // continue till time match found. if time matched then
        }
        if (timeFound && static_cast<datLogger::PackageId>(pkgs_[i].hdr.id) == datLogger::PackageId::OBJ_ID)
        {
            Indices.push_back(static_cast<int>(i)); // time matches
        }
        if (pkgs_[i].hdr.id == static_cast<int>(datLogger::PackageId::TIME_SERIES))
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


int Replay::searchAndReplacePkg(int idx1, int idx2, int idx3, double t)
{
    // lets take idx3 as a first reference ptr
    int ptr = idx3;

    // read the pkg id to find
    datLogger::PackageId pkgIdToFind = readPkgHdr(scenarioState.obj_states[static_cast<size_t>(idx1)].pkgs[static_cast<size_t>(idx2)].pkg);

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
        if ( pkgs_[static_cast<size_t>(ptr)].hdr.id == static_cast<int>(datLogger::PackageId::OBJ_ID))
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
                    if (pkgs_[static_cast<size_t>(ptr + ptrTemp)].hdr.id == static_cast<int>(datLogger::PackageId::OBJ_ID) ||
                        pkgs_[static_cast<size_t>(ptr + ptrTemp)].hdr.id == static_cast<int>(datLogger::PackageId::TIME_SERIES)) // already reached next time frame or next obj id
                    {
                        break; // go one step above
                    }

                    ptrTemp += 1;
                }

            }
        }
        else if( pkgs_[static_cast<size_t>(ptr)].hdr.id == static_cast<int>(datLogger::PackageId::TIME_SERIES) )
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

bool Replay::isObjAvailableInCache(int idx) // check in current state
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

bool Replay::isObjAvailable(int idx,  std::vector<int> Indices) // check in the object in the given new time
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

void Replay::MoveToTime(double t)
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
                                    datLogger::PackageId id_ = readPkgHdr(scenarioState.obj_states[i].pkgs[j].pkg);

                                    if (scenarioState.obj_states[i].pkgs[j].time_ == t ) // time matches so already new package in current state
                                    {
                                        break;
                                    }

                                    if (id_ == static_cast<datLogger::PackageId>(pkgs_[k].hdr.id)) // package id matches in given time, replace it with new package
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

double Replay::getTimeFromPkgIdx( size_t idx)
{
    double timeTemp = -1.0;
    for ( size_t i = idx; i > 0; i--) // return time if idx is already time pkg or find the respective time pkg for given pkg idx
    {
        if (static_cast<datLogger::PackageId>(pkgs_[i].hdr.id) == datLogger::PackageId::TIME_SERIES) // find time pkg for given pkg idx
        {
            timeTemp = *reinterpret_cast<double*>(pkgs_[i].content);
            break;
        }
    }
    return timeTemp;
}

double Replay::getTimeFromCnt(int count)
{
    double timeTemp = -1.0;
    int count_ = 0;
    for ( size_t i = 0; i < pkgs_.size(); i++) // return time if idx is already time pkg or find the respective time pkg for given pkg idx
    {
        if (static_cast<datLogger::PackageId>(pkgs_[i].hdr.id) == datLogger::PackageId::TIME_SERIES) // find time pkg for given pkg idx
        {
            count_ += 1;
            if (count == count_)
            {
                timeTemp = *reinterpret_cast<double*>(pkgs_[i].content);
                break;
            }

        }
    }
    return timeTemp;
}

void Replay::addObjState(size_t idx, double t)
{
    ObjectStateWithObjId stateObjId;
    if (static_cast<datLogger::PackageId>(pkgs_[idx].hdr.id) != datLogger::PackageId::OBJ_ID)
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

void Replay::initiateStates(double t)
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