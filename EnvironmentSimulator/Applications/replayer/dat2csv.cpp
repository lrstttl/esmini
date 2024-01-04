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

    while (player->GetTime() + player->deltaTime_ <= player->GetStopTime())
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
        player->MoveToTime(player->GetTime() + 0.01); // already initiated.
    }
#if (0)
    // Then output all entries with comma separated values
    for (size_t i = 0; i < player->data_.size(); i++)
    {
        ObjectStateStructDat* state = &player->data_[i].state;

        snprintf(line,
                 MAX_LINE_LEN,
                 "%.3f, %d, %s, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f\n",
                 static_cast<double>(state->info.timeStamp),
                 state->info.id,
                 state->info.name,
                 static_cast<double>(state->pos.x),
                 static_cast<double>(state->pos.y),
                 static_cast<double>(state->pos.z),
                 static_cast<double>(state->pos.h),
                 static_cast<double>(state->pos.p),
                 static_cast<double>(state->pos.r),
                 static_cast<double>(state->info.speed),
                 static_cast<double>(state->info.wheel_angle),
                 static_cast<double>(state->info.wheel_rot));

        file << line;
    }
#endif
    file.close();

}