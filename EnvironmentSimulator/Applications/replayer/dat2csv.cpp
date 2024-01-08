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
    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }
    std::unique_ptr<Replay> player;
    static char line[MAX_LINE_LEN];

    std::setlocale(LC_ALL, "C.UTF-8");

    if (argc < 2)
    {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

    std::string   filename = FileNameWithoutExtOf(argv[1]) + ".csv";
    std::ofstream file;
    file.open(filename);
    if (!file.is_open())
    {
        printf("Failed to create file %s\n", filename.c_str());
        return -1;
    }

    bool use_default_setting = true;
    if (argc > 2)
    {
        std::string   option = argv[2];
        if (std::strcmp(option.c_str(), "use_delta_time") == 0)
        {
            use_default_setting = false;
        }
    }
    // Create replayer object for parsing the binary data file
    try
    {
        player = std::make_unique<Replay>(argv[1]);
    }
    catch (const std::exception& e)
    {
        printf("%s", e.what());
        return -1;
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
    if (!use_default_setting)
    { // delta time setting
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
                player->MoveToTime(player->GetTime() + player->deltaTime_); // continue
            }
        }
    }
    else
    { // default setting
        for (size_t j = 0; j < player->pkgs_.size(); j++)
        {
            if (player->pkgs_[j].hdr.id == static_cast<int>(datLogger::PackageId::TIME_SERIES))
            {
                double timeTemp = *reinterpret_cast<double*>(player->pkgs_[j].content);
                player->MoveToTime(timeTemp);
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
            }
        }
    }
    file.close();

}