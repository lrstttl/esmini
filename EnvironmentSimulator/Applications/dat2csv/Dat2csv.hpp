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

#include <clocale>

#include <string>
#include <fstream>

#include "Replay.hpp"

enum class log_mode
{
    ORIGINAL = 0, // default
    MIN_STEP = 1,
    MIN_STEP_MIXED = 2,
    TIME_STEP = 3, // 0.05 default step time.
    TIME_STEP_MIXED = 4 // 0.05 default step time


};

class Dat2csv
{
public:
    Dat2csv(std::string filename);
    ~Dat2csv();

    void CreateCSV();

    void SetLogMode( log_mode mode_)
    {
        log_mode_ = mode_;
    }

    void SetStepTime( double t)
    {
        step_time_ = t;
    }

private:
    log_mode log_mode_;
    double step_time_;
    std::ofstream file;

    std::unique_ptr<scenarioengine::Replay> player;

};