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

#include <string>
#include <fstream>
#include "CommonMini.hpp"
#include "ScenarioGateway.hpp"
#include "DatLogger.hpp"

namespace scenarioengine
{
    typedef struct
    {
        ObjectStateStructDat state;
        double               odometer;
    } ReplayEntry;

    // new replayer
    // cache for reading states
    typedef struct
    {
        double time_;
        char*  pkg;
    } ObjectStateWithPkg;

    typedef struct
    {
        int                             id;
        std::vector<ObjectStateWithPkg> pkgs;
    } ObjectStateWithObjId;

    typedef struct
    {
        double                            sim_time;
        std::vector<ObjectStateWithObjId> obj_states;
    } ScenarioState;

    class Replay
    {
    public:
        DatHeader                header_;
        std::vector<ReplayEntry> data_;

        Replay() = default;
        Replay(std::string filename, bool clean);
        // Replay(const std::string directory, const std::string scenario, bool clean);
        Replay(const std::string directory, const std::string scenario, std::string create_datfile);
        ~Replay();

        /**
                Go to specific time
                @param time timestamp (0 = beginning, -1 end)
                @param stop_at_next_frame If true move max to next/previous time frame
        */
        void                  GoToTime(double time, bool stop_at_next_frame = false);
        void                  GoToDeltaTime(double dt, bool stop_at_next_frame = false);
        void                  GetReplaysFromDirectory(const std::string dir, const std::string sce);
        size_t                GetNumberOfScenarios();
        void                  GoToStart();
        void                  GoToEnd();
        int                   GoToNextFrame();
        void                  GoToPreviousFrame();
        unsigned int          FindNextTimestamp(bool wrap = false);
        unsigned int          FindPreviousTimestamp(bool wrap = false);
        ReplayEntry*          GetEntry(int id);
        ObjectStateStructDat* GetState(int id);
        void                  SetStartTime(double time);
        void                  SetStopTime(double time);
        double                GetStartTime()
        {
            return startTime_;
        }
        double GetStopTime()
        {
            return stopTime_;
        }
        double GetTime()
        {
            return time_;
        }
        int GetIndex()
        {
            return static_cast<int>(index_);
        }
        void SetRepeat(bool repeat)
        {
            repeat_ = repeat;
        }
        void CleanEntries(std::vector<ReplayEntry>& entries);
        void BuildData(std::vector<std::pair<std::string, std::vector<ReplayEntry>>>& scenarios);
        void CreateMergedDatfile(const std::string filename);

        // new replayer

        std::vector<datLogger::CommonPkg> pkgs_;
        ScenarioState                     scenarioState;

        void                 initiateStates(double time_frame);
        datLogger::PackageId readPkgHdr(char* package);
        int                  recordPackage(const std::string& fileName);  // check package can be recorded or not
        std::vector<int>     GetNumberOfObjectsAtTime(double t);          // till next time forward
        int                  getPkgCntBtwObj(size_t idx);                 // till next time forward
        double               getTimeFromPkgIdx(size_t idx);

        double getTimeFromCnt(int count);            // give time for the time
        void   addObjState(size_t objId, double t);  // add the object state for given object id from the current object state
        int    searchAndReplacePkg(int idx1, int idx2, int idx3, double time);

        bool isObjAvailableInCache(int Idx);                     // check in current state
        bool isObjAvailable(int idx, std::vector<int> Indices);  // check in the object in the given new time
        void MoveToTime(double time_frame);
        int  GetObjCompleteState(double time, int obj_id, ScenarioState& state);

    private:
        std::ifstream            file_;
        std::vector<std::string> scenarios_;
        double                   time_;
        double                   startTime_;
        double                   stopTime_;
        unsigned int             startIndex_;
        unsigned int             stopIndex_;
        unsigned int             index_;
        bool                     repeat_;
        bool                     clean_;
        std::string              create_datfile_;

        int FindIndexAtTimestamp(double timestamp, int startSearchIndex = 0);
    };

}  // namespace scenarioengine