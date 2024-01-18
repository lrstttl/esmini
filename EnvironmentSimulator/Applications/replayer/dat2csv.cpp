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

/*
 * This application uses the Replay class to read and binary recordings and print content in ascii format to stdout
 */

#include <clocale>

#include "Replay.hpp"
#include "CommonMini.hpp"
#include "DatLogger.hpp"

using namespace scenarioengine;

#define MAX_LINE_LEN 2048

int main(int argc, char** argv)
{

    SE_Options opt;
    opt.AddOption("file", "Simulation recording data file (.dat)", "filename");
    opt.AddOption("time_mode", "control timestamps in the csv (original, min_step, mixed)", "mode", "original");
    opt.AddOption("time_step", "use fixed time step (ms) - overrides time_mode", "time_step", "0.05");

    enum class mode
    {
        ORIGINAL = 0, // default
        MIN_STEP = 1,
        MIXED = 2,
        TIME_STEP = 3 // 0.05 default step time

    };


    std::unique_ptr<Replay> player;
    static char line[MAX_LINE_LEN];

    std::setlocale(LC_ALL, "C.UTF-8");

    mode log_mode;
    log_mode = mode::ORIGINAL;

    if (opt.ParseArgs(argc, argv) != 0 || argc < 1)
    {
        opt.PrintUsage();
        return -1;
    }

    if (opt.GetOptionArg("file").empty())
    {
        printf("Missing file argument\n");
        opt.PrintUsage();
        return -1;
    }

    std::string   filename = FileNameWithoutExtOf(opt.GetOptionArg("file")) + ".csv";
    std::ofstream file;
    file.open(filename);
    if (!file.is_open())
    {
        printf("Failed to create file %s\n", filename.c_str());
        return -1;
    }

    // Create replayer object for parsing the binary data file
    try
    {
        player = std::make_unique<Replay>(opt.GetOptionArg("file"));
    }
    catch (const std::exception& e)
    {
        printf("%s", e.what());
        return -1;
    }

    double delta_time_step = player->deltaTime_;

    if (opt.GetOptionSet("time_mode"))
    {
        std::string time_mode_str = opt.GetOptionArg("time_mode");
        if (!time_mode_str.empty())
        {
            if (time_mode_str == "original" )
            {
                log_mode = mode::ORIGINAL;
            }
            else if (time_mode_str == "min_step" )
            {
                log_mode = mode::MIN_STEP;
            }
            else if (time_mode_str == "mixed" )
            {
                log_mode = mode::MIXED;
            }
            else
            {
                LOG("Unsupported time mode: %s - using default (Original)", time_mode_str.c_str());
            }
        }
    }

    if (opt.GetOptionSet("time_step"))
    {
        std::string timeStep_str = opt.GetOptionArg("time_step");
        if (!timeStep_str.empty())
        {
            log_mode = mode::TIME_STEP;
            delta_time_step = strtod(timeStep_str);
        }
        else
        {
            printf("Failed to provide fixed time step, Logging with default mode\n");
        }
    }

    datLogger::DatHdr        headerNew_;
    headerNew_ = *reinterpret_cast<datLogger::DatHdr*>(player->header_.content);
    // First output header and CSV labels
    snprintf(line,
             MAX_LINE_LEN,
             "Version: %d, OpenDRIVE: %s, 3DModel: %s\n",
             headerNew_.version,
             headerNew_.odrFilename.string,
             headerNew_.modelFilename.string);
    file << line;
    snprintf(line, MAX_LINE_LEN, "time, id, name, x, y, z, h, p, r, speed, wheel_angle, wheel_rot\n");
    file << line;
    player->SetShowRestart(true); // include restart details always in csv files

    if (log_mode == mode::MIN_STEP || log_mode == mode::TIME_STEP)
    { // delta time setting, write for each delta time+sim time. Will skips original time frame
        while (true)
        {
            for (size_t i = 0; i < player->scenarioState.obj_states.size(); i++)
            {
                int obj_id = player->scenarioState.obj_states[i].id;
                std::string name;
                player->GetName(player->scenarioState.obj_states[i].id, name);

                snprintf(line,
                        MAX_LINE_LEN,
                        "%.10f, %d, %s, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
                        player->scenarioState.sim_time,
                        obj_id,
                        name.c_str(),
                        player->GetX(player->scenarioState.obj_states[i].id),
                        player->GetY(player->scenarioState.obj_states[i].id),
                        player->GetZ(player->scenarioState.obj_states[i].id),
                        player->GetH(player->scenarioState.obj_states[i].id),
                        player->GetP(player->scenarioState.obj_states[i].id),
                        player->GetR(player->scenarioState.obj_states[i].id),
                        player->GetSpeed(player->scenarioState.obj_states[i].id),
                        player->GetWheelAngle(player->scenarioState.obj_states[i].id),
                        player->GetWheelRot(player->scenarioState.obj_states[i].id));
                file << line;
            }

            if (player->GetTime() > player->GetStopTime() - SMALL_NUMBER)
            {
                break;  // reached end of file
            }
            else if (delta_time_step < SMALL_NUMBER)
            {
                LOG("Warning: Unexpected delta time zero found! Can't process remaining part of the file");
                break;
            }
            else
            {
                player->MoveToTime(player->GetTime() + delta_time_step); // continue
            }
        }
    }
    else if (log_mode == mode::MIXED)
    { // write for each delta time+sim time and also original time frame if available in between. Dont skip original time frame.
        double requestedTime = SMALL_NUMBER;
        while (true)
        {
            for (size_t i = 0; i < player->scenarioState.obj_states.size(); i++)
            {
                int obj_id = player->scenarioState.obj_states[i].id;
                std::string name;
                player->GetName(player->scenarioState.obj_states[i].id, name);

                snprintf(line,
                        MAX_LINE_LEN,
                        "%.3f, %d, %s, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
                        player->scenarioState.sim_time,
                        obj_id,
                        name.c_str(),
                        player->GetX(player->scenarioState.obj_states[i].id),
                        player->GetY(player->scenarioState.obj_states[i].id),
                        player->GetZ(player->scenarioState.obj_states[i].id),
                        player->GetH(player->scenarioState.obj_states[i].id),
                        player->GetP(player->scenarioState.obj_states[i].id),
                        player->GetR(player->scenarioState.obj_states[i].id),
                        player->GetSpeed(player->scenarioState.obj_states[i].id),
                        player->GetWheelAngle(player->scenarioState.obj_states[i].id),
                        player->GetWheelRot(player->scenarioState.obj_states[i].id));
                file << line;
            }

            if (player->GetTime() > player->GetStopTime() - SMALL_NUMBER)
            {
                break;  // reached end of file
            }
            else if (player->deltaTime_ < SMALL_NUMBER)
            {
                LOG("Warning: Unexpected delta time zero found! Can't process remaining part of the file");
                break;
            }
            else
            {
                if ( isEqualDouble(player->GetTime(), requestedTime) || isEqualDouble(player->GetTime(), player->GetStartTime()))
                { // first time frame or until reach requested time frame reached, dont move to next time frame
                    requestedTime = player->GetTime() + player->deltaTime_;
                    player->MoveToTime(player->GetTime() + player->deltaTime_, false, true); // continue
                }
                else
                {
                    player->MoveToTime(requestedTime, false, true); // continue
                }
            }
        }
    }
    else if ( log_mode == mode::ORIGINAL)
    { // default setting, write time stamps available only in dat file
        for (size_t j = 0; j < player->pkgs_.size(); j++)
        {
            if (player->pkgs_[j].hdr.id == static_cast<int>(datLogger::PackageId::TIME_SERIES))
            {
                double timeTemp = *reinterpret_cast<double*>(player->pkgs_[j].content);

                // next time
                player->SetTime(timeTemp);
                player->SetIndex(static_cast<int>(j));

                player->CheckObjAvailabilityForward();
                player->UpdateCache();
                player->scenarioState.sim_time = timeTemp;
                for (size_t i = 0; i < player->scenarioState.obj_states.size(); i++)
                {
                    int obj_id = player->scenarioState.obj_states[i].id;
                    std::string name;
                    player->GetName(player->scenarioState.obj_states[i].id, name);
                    std::cout << "x-->" << obj_id << "-->"<< player->GetX(obj_id) << std::endl;

                    snprintf(line,
                            MAX_LINE_LEN,
                            "%.3f, %d, %s, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
                            player->scenarioState.sim_time,
                            obj_id,
                            name.c_str(),
                            player->GetX(obj_id),
                            player->GetY(obj_id),
                            player->GetZ(obj_id),
                            player->GetH(obj_id),
                            player->GetP(obj_id),
                            player->GetR(obj_id),
                            player->GetSpeed(obj_id),
                            player->GetWheelAngle(obj_id),
                            player->GetWheelRot(obj_id));
                    file << line;
                }
            }
        }
    }
    file.close();

}