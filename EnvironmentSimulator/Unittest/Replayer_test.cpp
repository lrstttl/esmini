#include <gtest/gtest.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <filesystem>
#include <dirent.h>

#include "ScenarioEngine.hpp"
#include "esminiLib.hpp"
#include "CommonMini.hpp"
#include "DatLogger.hpp"
#include "Replay.hpp"

using namespace datLogger;

TEST(LogOperationsWithOneObject, TestLogOperationsWithOneObject)
{
    std::string fileName    = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name  = "e6mini.osgb";
    int         version_    = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);

    double x     = 1.0;
    double y     = 2.0;
    double z     = 3.0;
    double h     = 4.0;
    double r     = 5.0;
    double p     = 6.0;
    double speed = 1.0;

    double current_time = 0.033;
    int    no_of_obj    = 1;

    int pkg_nos    = 2;  // speed and pos pkg
    int total_time = 6;
    // calc
    // 1obj
    // 1 hdr, 6 time, 6 obj id, 6 pos, 6 speed, 1 obj added  = 26 pkg  received
    // 1 hdr, 6 time, 6 obj id, 2 pos, 5 speed, 1 obj added  = 21 pkg  written

    for (int i = 0; i < total_time; i++)
    {
        if (i == 4 || i == 5)
        {
            h = 6.0;
        }
        logger->simTimeTemp = current_time;
        for (int j = 0; j < no_of_obj; j++)
        {
            if (i == 2 && j == 2)
            {
                break;  // delete one object.
            }
            int object_id = j;
            logger->AddObject(object_id);
            logger->WriteObjPos(object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
            logger->ObjIdPkgAdded = false;
        }
        if (i != 3)
        {
            speed += 1.0;
        }
        current_time += 1.089;
        logger->deleteObject();
        logger->TimePkgAdded = false;
    }

    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos) + 1);
    ASSERT_EQ(logger->totalPkgProcessed, 21);
    ASSERT_EQ(logger->totalPkgSkipped, 5);

    delete logger;
}


TEST(RecordOperationsWithOneObject, TestRecordInitWithOneObject)
{
    std::string             fileName = "sim.dat";
    std::unique_ptr<scenarioengine::Replay> replay = std::make_unique<scenarioengine::Replay>(fileName);


    ASSERT_EQ(replay->pkgs_.size(), 21); // header not stored, 1 scenario end pkg added
    ASSERT_EQ(replay->scenarioState.sim_time, replay->GetTimeFromCnt(1));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(replay->GetTimeFromCnt(2), 1.1220000000000001);

    replay->MoveToTime(replay->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(2));

    replay->MoveToTime(replay->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(4));

    replay->MoveToTime(replay->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(4));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);

}

TEST(LogOperationsWithTwoObject, TestLogOperationsWithTwoObject)
{
    std::string fileName    = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name  = "e6mini.osgb";
    int         version_    = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);

    double x     = 1.0;
    double y     = 2.0;
    double z     = 3.0;
    double h     = 4.0;
    double r     = 5.0;
    double p     = 6.0;
    double speed = 1.0;

    double current_time = 0.033;

    int no_of_obj  = 2;
    int pkg_nos    = 2;  // speed and pos pkg
    int total_time = 6;
    // calc
    // 2obj
    // 1 hdr, 6 time, 12 obj id, 12 pos, 12 speed, 2 obj added = 43 pkg  received
    // 1 hdr, 6 time, 12 obj id, 4 pos, 10 speed, 2 obj added = 33 pkg  written

    for (int i = 0; i < total_time; i++)
    {
        if (i == 4 || i == 5)
        {
            h = 6.0;
        }
        logger->simTimeTemp = current_time;
        for (int j = 0; j < no_of_obj; j++)
        {
            if (i == 2 && j == 2)
            {
                break;  // delete one object.
            }
            int object_id = j;
            logger->AddObject(object_id);
            logger->WriteObjPos(object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
            logger->ObjIdPkgAdded = false;
        }
        if (i != 3)
        {
            speed += 1.0;
        }
        current_time += 1.089;
        logger->deleteObject();
        logger->TimePkgAdded = false;
    }

    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos) + 2);
    ASSERT_EQ(logger->totalPkgProcessed, 35);
    ASSERT_EQ(logger->totalPkgSkipped, 10);

    delete logger;
}

TEST(TestRecordWithTwoObject, TestRecordWithTwoObject)
{
    std::string             fileName = "sim.dat";
    std::unique_ptr<scenarioengine::Replay> replay = std::make_unique<scenarioengine::Replay>(fileName);

     ASSERT_EQ(replay->pkgs_.size(), 35); // header not stored, 1 scenario end pkg added
    ASSERT_EQ(replay->scenarioState.sim_time, replay->GetTimeFromCnt(1));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 2);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(replay->GetTimeFromCnt(2), 1.1220000000000001);

    replay->MoveToTime(replay->GetTimeFromCnt(2));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[1].time_, replay->GetTimeFromCnt(2));

    replay->MoveToTime(replay->GetTimeFromCnt(4));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[1].time_, replay->GetTimeFromCnt(4));

    replay->MoveToTime(replay->GetTimeFromCnt(5));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[0].time_, replay->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[1].time_, replay->GetTimeFromCnt(4));

}

TEST(LogOperationsAddAndDelete, TestLogOperationsAddAndDelete)
{
    std::string fileName    = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name  = "e6mini.osgb";
    int         version_    = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);

    double x     = 1.0;
    double y     = 2.0;
    double z     = 3.0;
    double h     = 4.0;
    double r     = 5.0;
    double p     = 6.0;
    double speed = 1.0;

    double current_time = 0.033;

    int no_of_obj         = 3;
    int pkg_nos           = 2;  // speed and pos pkg
    int total_time        = 6;

    // calc
    //  3obj- one obj deleted so 1 pos + speed pkg less
    //  1 hdr, 6 time, 18 obj id, 17 pos, 17 speed, 1 dele pkg, 3 obj added  = 63 pkg  received
    //  1 hdr, 6 time, 18 obj id, 7 pos, 14 speed, 1 dele pkg, 4 obj added  = 50 pkg

    for (int i = 0; i < total_time; i++)
    {
        if (i == 4 || i == 5)
        {
            h = 6.0;
        }
        logger->simTimeTemp = current_time;
        for (int j = 0; j < no_of_obj; j++)
        {
            if (i == 2 && j == 2)
            {
                break;  // delete one object.
            }
            int object_id = j;
            logger->AddObject(object_id);
            logger->WriteObjPos(object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
            logger->ObjIdPkgAdded = false;
        }
        if (i != 3)
        {
            speed += 1.0;
        }
        current_time += 1.089;
        logger->deleteObject();
        logger->TimePkgAdded = false;
    }

    ASSERT_EQ(logger->totalPkgReceived,
              1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos) -1 -1 + 1 + 3);
    ASSERT_EQ(logger->totalPkgProcessed, 51);
    ASSERT_EQ(logger->totalPkgSkipped, 13);

    delete logger;
}

TEST(TestRecordWithThreeObject, TestRecordWithThereObject)
{
    std::string             fileName = "sim.dat";
    std::unique_ptr<scenarioengine::Replay> replay = std::make_unique<scenarioengine::Replay>(fileName);
    ASSERT_EQ(replay->pkgs_.size(), 51); // header not stored.

    ASSERT_EQ(replay->scenarioState.sim_time, replay->GetTimeFromCnt(1));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(replay->GetTimeFromCnt(2), 1.122);

    replay->MoveToTime(replay->GetTimeFromCnt(2));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 3);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[1].time_, replay->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[2].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[2].pkgs[1].time_, replay->GetTimeFromCnt(2));

    replay->MoveToTime(replay->GetTimeFromCnt(3));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replay->scenarioState.obj_states[0].id, 0);
    ASSERT_EQ(replay->scenarioState.obj_states[1].id, 1);
    ASSERT_EQ(replay->scenarioState.obj_states[2].id, 2);
    ASSERT_EQ(replay->scenarioState.obj_states[2].active, false); // obj deleted
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(3));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[1].time_, replay->GetTimeFromCnt(3));

    replay->MoveToTime(replay->GetTimeFromCnt(4));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 3); // obj added
    ASSERT_EQ(replay->scenarioState.obj_states[0].id, 0);
    ASSERT_EQ(replay->scenarioState.obj_states[1].id, 1);
    ASSERT_EQ(replay->scenarioState.obj_states[2].id, 2);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[1].time_, replay->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[2].pkgs[0].time_, replay->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[2].pkgs[1].time_, replay->GetTimeFromCnt(4));

    replay->MoveToTime(replay->GetTimeFromCnt(5));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 3);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[0].time_, replay->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[1].time_, replay->GetTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[2].pkgs[0].time_, replay->GetTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[2].pkgs[1].time_, replay->GetTimeFromCnt(4));


}

TEST(LogOperationsTime, TestLogOperationsTime)
{
    std::string fileName    = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name  = "e6mini.osgb";
    int         version_    = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);

    double x     = 1.0;
    double y     = 2.0;
    double z     = 3.0;
    double h     = 4.0;
    double r     = 5.0;
    double p     = 6.0;
    double speed = 1.0;

    double current_time = 0.033;
    int    no_of_obj    = 1;
    int valid_time_frame = 1;

    int pkg_nos    = 2;  // speed and pos pkg
    int total_time = 6;
    // calc  , No pkg change except first time frame
    // 1obj
    // 1 hdr, 1 time, 1 obj id, 6 pos, 6 speed, 1 obj added  = 16 pkg  received
    // 1 hdr, 1 time, 1 obj id, 1 pos, 1 speed, 1 abj added  = 6 pkg  written

    for (int i = 0; i < total_time; i++)
    {
        logger->simTimeTemp = current_time;
        for (int j = 0; j < no_of_obj; j++)
        {
            int object_id = j;
            logger->AddObject(object_id);
            logger->WriteObjPos(object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
            logger->ObjIdPkgAdded = false;
        }
        current_time += 1.089;
        logger->deleteObject();
        logger->TimePkgAdded = false;
    }

    ASSERT_EQ(logger->totalPkgReceived, 1 + valid_time_frame + (valid_time_frame * no_of_obj) + (no_of_obj * total_time * pkg_nos) + 1);
    ASSERT_EQ(logger->totalPkgProcessed, 6);
    ASSERT_EQ(logger->totalPkgSkipped, 10);

    delete logger;
}

TEST(RecordOperationsTime, TestRecordOperationsTime)
{
    std::string             fileName = "sim.dat";
    std::unique_ptr<scenarioengine::Replay> replay = std::make_unique<scenarioengine::Replay>(fileName);
    ASSERT_EQ(replay->pkgs_.size(), 7); // header not stored, last time added

    ASSERT_EQ(replay->scenarioState.sim_time, replay->GetTimeFromCnt(1));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(replay->GetTimeFromCnt(1), 0.033000000000000002);

    replay->MoveToTime(replay->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(1));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);

}


TEST(TestDatSimpleScenario, TestLogAndRecordSimpleScenario)
{

    const char* args[] =
        {"--osc", "../../../EnvironmentSimulator/Unittest/xosc/simple_scenario.xosc", "--record", "new_sim.dat", "--fixed_timestep", "0.5"};

    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args) / sizeof(char*), args), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.05f);
    }

    SE_Close();

    std::filesystem::path cwd = std::filesystem::current_path();
    std::cout << cwd << std::endl;

    scenarioengine::Replay* replay = new scenarioengine::Replay("new_sim.dat");
    ASSERT_EQ(replay->pkgs_.size(), 3165);

    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    replay->MoveToTime(replay->GetTimeFromCnt(15));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[6].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[11].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, replay->GetTimeFromCnt(15));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[7].time_, replay->GetTimeFromCnt(15));

    replay->MoveToTime(replay->GetTimeFromCnt(30));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[5].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[9].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, replay->GetTimeFromCnt(30));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[7].time_, replay->GetTimeFromCnt(30));
}

TEST(TestDatSpeedChange, TestLogAndRecordSpeedChange)
{

    const char* args[] =
        {"--osc", "../../../EnvironmentSimulator/Unittest/xosc/speed_change.xosc", "--record", "new_sim.dat", "--fixed_timestep", "0.5"};

    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args) / sizeof(char*), args), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.05f);
    }

    SE_Close();

    scenarioengine::Replay* replay = new scenarioengine::Replay("new_sim.dat");
    ASSERT_EQ(replay->pkgs_.size(), 3206);

    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    replay->MoveToTime(18);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, 17.150000255554914); // pos
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, 17.200000256299973); // speed
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[3].time_, 0);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[4].time_, 0);

    replay->MoveToTime(19.5);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, 17.150000255554914); // pos
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, 17.200000256299973); // speed
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[3].time_, 0);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[4].time_, 0);

    replay->MoveToTime(21);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, 20.950000312179327); // pos
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, 20.950000312179327); // speed
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[3].time_, 0);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[4].time_, 0);

}

TEST(DatMergeScenarioTest, TestTwoSimpleScenarioMerge)
{

    const char* args[] =
        {"--osc", "../../../EnvironmentSimulator/Unittest/xosc/simple_scenario.xosc", "--record", "new_file0.dat", "--fixed_timestep", "0.5"};

    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args) / sizeof(char*), args), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.05f);
    }

    SE_Close();

    const char* args1[] =
        {"--osc", "../../../EnvironmentSimulator/Unittest/xosc/simple_scenario_reversed.xosc", "--record", "new_file1.dat", "--fixed_timestep", "0.5"};

    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args1) / sizeof(char*), args1), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.05f);
    }

    std::string currentPath = std::filesystem::current_path();
    std::unique_ptr<scenarioengine::Replay> replay = std::make_unique<scenarioengine::Replay>(currentPath, "new_file", "");
    ASSERT_EQ(replay->pkgs_.size(), 5504);

    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 2);
    replay->MoveToTime(replay->GetTimeFromCnt(15));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 2);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[6].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[11].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, replay->GetTimeFromCnt(15));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[7].time_, replay->GetTimeFromCnt(15));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[6].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[11].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[2].time_, replay->GetTimeFromCnt(15));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[7].time_, replay->GetTimeFromCnt(15));

    replay->MoveToTime(replay->GetTimeFromCnt(30));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 2);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[5].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[9].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, replay->GetTimeFromCnt(30));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[7].time_, replay->GetTimeFromCnt(30));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[5].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[9].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[2].time_, replay->GetTimeFromCnt(30));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[1].pkgs[7].time_, replay->GetTimeFromCnt(30));
}

TEST(ReplayRestartTest, TestShowAndNotShowRestart)
{
    const char* args[] =
        {"--osc", "../../../EnvironmentSimulator/Unittest/xosc/timing_scenario_with_restarts.xosc", "--record", "new_sim.dat", "--fixed_timestep", "0.1"};

    SE_AddPath("../../../resources/models");
    ASSERT_EQ(SE_InitWithArgs(sizeof(args) / sizeof(char*), args), 0);

    while (SE_GetQuitFlag() == 0)
    {
        SE_StepDT(0.01f);
    }

    SE_Close();

    scenarioengine::Replay* replay = new scenarioengine::Replay("new_sim.dat");
    ASSERT_EQ(replay->pkgs_.size(), 10426);

    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 3);

    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetStartTime()); // pos
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, replay->GetStartTime()); // speed
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[3].time_, replay->GetStartTime());
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[4].time_, replay->GetStartTime());
    std::string name;
    replay->GetName(replay->scenarioState.obj_states[0].id, name);
    EXPECT_EQ(name, "Ego");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[0].id), 10.0, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[0].id), -1.5, 1E-3);

    replay->GetName(replay->scenarioState.obj_states[1].id, name);
    EXPECT_EQ(name, "Target");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[1].id), 10.0, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[1].id), -4.5, 1E-3);

    replay->GetName(replay->scenarioState.obj_states[2].id, name);
    EXPECT_EQ(name, "Ego_ghost");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[2].id), 10.0, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[2].id), -1.5, 1E-3);


    // with show restart
    replay->SetShowRestart(true);
    replay->GetRestartTimes();

    ASSERT_EQ(replay->restartTimes.size(), 2);
    ASSERT_EQ(replay->restartTimes[0].restart_index_, 2158);
    ASSERT_EQ(replay->restartTimes[0].next_index_, 2525);
    ASSERT_DOUBLE_EQ(replay->restartTimes[0].restart_time_, 2.009999955072999);
    ASSERT_DOUBLE_EQ(replay->restartTimes[0].next_time_, 2.0199999548494829);
    ASSERT_EQ(replay->restartTimes[1].restart_index_, 8062);
    ASSERT_EQ(replay->restartTimes[1].next_index_, 8429);
    ASSERT_DOUBLE_EQ(replay->restartTimes[1].restart_time_, 8.00999982096255);
    ASSERT_DOUBLE_EQ(replay->restartTimes[1].next_time_, 8.019999820739022);

    replay->MoveToTime(replay->restartTimes[0].restart_time_); //first restart frame
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.sim_time, replay->restartTimes[0].restart_time_);
    replay->GetName(replay->scenarioState.obj_states[0].id, name);
    EXPECT_EQ(name, "Ego");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[0].id), 10.0, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[0].id), -1.5, 1E-3);
    replay->GetName(replay->scenarioState.obj_states[1].id, name);
    EXPECT_EQ(name, "Target");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[1].id), 50.199, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[1].id), -4.5, 1E-3);
    replay->GetName(replay->scenarioState.obj_states[2].id, name);
    EXPECT_EQ(name, "Ego_ghost");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[2].id), 60.099, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[2].id), -1.5, 1E-3);

    replay->MoveToTime(replay->restartTimes[0].next_time_); // shall go first restart frame
    ASSERT_DOUBLE_EQ(replay->scenarioState.sim_time, -0.93000004515051837);
    replay->GetName(replay->scenarioState.obj_states[0].id, name);
    EXPECT_EQ(name, "Ego");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[0].id), 10.0, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[0].id), -1.5, 1E-3);
    replay->GetName(replay->scenarioState.obj_states[1].id, name);
    EXPECT_EQ(name, "Target");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[1].id), 50.399, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[1].id), -4.5, 1E-3);
    replay->GetName(replay->scenarioState.obj_states[2].id, name);
    EXPECT_EQ(name, "Ego_ghost");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[2].id), 10.000, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[2].id), -1.5, 1E-3);


    replay->MoveToTime(replay->restartTimes[1].restart_time_); //second restart frame
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.sim_time, replay->restartTimes[1].restart_time_);

    replay->MoveToTime(replay->restartTimes[1].next_time_); // shall go second restart frame
    ASSERT_DOUBLE_EQ(replay->scenarioState.sim_time, 5.0699998207390324);


    // with no show restart
    replay->InitiateStates();
    replay->SetShowRestart(false);
    replay->GetRestartTimes();

    replay->MoveToTime(replay->restartTimes[0].restart_time_); //first restart frame
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    replay->GetName(replay->scenarioState.obj_states[0].id, name);
    EXPECT_EQ(name, "Ego");
    ASSERT_DOUBLE_EQ(replay->scenarioState.sim_time, replay->restartTimes[0].restart_time_);
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[0].id), 10.0, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[0].id), -1.5, 1E-3);
    replay->GetName(replay->scenarioState.obj_states[1].id, name);
    EXPECT_EQ(name, "Target");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[1].id), 50.199, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[1].id), -4.5, 1E-3);
    replay->GetName(replay->scenarioState.obj_states[2].id, name);
    EXPECT_EQ(name, "Ego_ghost");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[2].id), 60.099, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[2].id), -1.5, 1E-3);

    replay->MoveToTime(replay->restartTimes[0].next_time_); // shall go next frame
    ASSERT_DOUBLE_EQ(replay->scenarioState.sim_time, replay->restartTimes[0].next_time_);
    replay->GetName(replay->scenarioState.obj_states[0].id, name);
    EXPECT_EQ(name, "Ego");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[0].id), 10.0, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[0].id), -1.5, 1E-3);
    replay->GetName(replay->scenarioState.obj_states[1].id, name);
    EXPECT_EQ(name, "Target");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[1].id), 50.199, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[1].id), -4.5, 1E-3);
    replay->GetName(replay->scenarioState.obj_states[2].id, name);
    EXPECT_EQ(name, "Ego_ghost");
    EXPECT_NEAR(replay->GetX(replay->scenarioState.obj_states[2].id), 23.724, 1E-3);
    EXPECT_NEAR(replay->GetY(replay->scenarioState.obj_states[2].id), -1.5, 1E-3);

    replay->MoveToTime(replay->restartTimes[1].restart_time_); //second restart frame
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.sim_time, replay->restartTimes[1].restart_time_);

    replay->MoveToTime(replay->restartTimes[1].next_time_); // shall go next frame
    ASSERT_DOUBLE_EQ(replay->scenarioState.sim_time, replay->restartTimes[1].next_time_);

}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}