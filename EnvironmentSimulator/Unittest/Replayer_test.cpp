#include <gtest/gtest.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <filesystem>

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
    // 1 hdr, 6 time, 6 obj id, 6 pos, 6 speed  = 25 pkg  received
    // 1 hdr, 6 time, 6 obj id, 2 pos, 5 speed  = 21 pkg  written

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

    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos));
    ASSERT_EQ(logger->totalPkgProcessed, 20);
    ASSERT_EQ(logger->totalPkgSkipped, 5);

    delete logger;
}

TEST(RecordOperationsWithOneObject, TestRecordInitWithOneObject)
{
    std::string             fileName = "sim.dat";
    std::unique_ptr<scenarioengine::Replay> replay = std::make_unique<scenarioengine::Replay>(fileName);


    ASSERT_EQ(replay->pkgs_.size(), 21); // extra one is end of scenario pkg
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
    // 1 hdr, 6 time, 12 obj id, 12 pos, 12 speed  = 43 pkg  received
    // 1 hdr, 6 time, 12 obj id, 4 pos, 10 speed  = 33 pkg  written

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

    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos));
    ASSERT_EQ(logger->totalPkgProcessed, 33);
    ASSERT_EQ(logger->totalPkgSkipped, 10);

    delete logger;
}

TEST(TestRecordWithTwoObject, TestRecordWithTwoObject)
{
    scenarioengine::Replay* replay = new scenarioengine::Replay;

    std::string fileName = "sim.dat";
    replay->RecordPkgs(fileName);
    ASSERT_EQ(replay->pkgs_.size(), 34); // extra one is end of scenario pkg

    replay->InitiateStates(replay->GetTimeFromCnt(1));
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

    delete replay;
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
    int no_of_obj_deleted = 1;

    // calc
    //  3obj- one obj deleted so 1 obj id  + pos + speed pkg less
    //  1 hdr, 6 time, 18 obj id, 18 pos, 18 speed, 1 dele pkg  = 62 pkg  received
    //  1 hdr, 6 time, 18 obj id, 7 pos, 14 speed  = 45 pkg  written (while add obj . all pkg to be added)

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
              total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos));
    ASSERT_EQ(logger->totalPkgProcessed, 46);
    ASSERT_EQ(logger->totalPkgSkipped, 13);

    delete logger;
}

TEST(TestRecordWithThreeObject, TestRecordWithThereObject)
{
    scenarioengine::Replay* replay = new scenarioengine::Replay;

    std::string fileName = "sim.dat";
    replay->RecordPkgs(fileName);
    ASSERT_EQ(replay->pkgs_.size(), 48);

    replay->InitiateStates(replay->GetTimeFromCnt(1));
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
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 2); // obj deleted
    ASSERT_EQ(replay->scenarioState.obj_states[0].id, 0);
    ASSERT_EQ(replay->scenarioState.obj_states[1].id, 1);
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

    delete replay;

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
    // 1 hdr, 1 time, 1 obj id, 6 pos, 6 speed  = 15 pkg  received
    // 1 hdr, 1 time, 1 obj id, 1 pos, 1 speed  = 5 pkg  written

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

    ASSERT_EQ(logger->totalPkgReceived, 1 + valid_time_frame + (valid_time_frame * no_of_obj) + (no_of_obj * total_time * pkg_nos));
    ASSERT_EQ(logger->totalPkgProcessed, 5);
    ASSERT_EQ(logger->totalPkgSkipped, 10);

    delete logger;
}

TEST(RecordOperationsTime, TestRecordOperationsTime)
{
    std::string             fileName = "sim.dat";
    scenarioengine::Replay* replay   = new scenarioengine::Replay;

    replay->RecordPkgs(fileName);
    ASSERT_EQ(replay->pkgs_.size(), 7); // extra two is time and end of scenario pkg

    replay->InitiateStates(replay->GetTimeFromCnt(1));
    ASSERT_EQ(replay->scenarioState.sim_time, replay->GetTimeFromCnt(1));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(replay->GetTimeFromCnt(1), 0.033000000000000002);

    replay->MoveToTime(replay->GetTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[0].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, replay->GetTimeFromCnt(1));
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);

    delete replay;
}
#if 0
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
    ASSERT_EQ(replay->pkgs_.size(), 3764);

    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    replay->MoveToTime(replay->GetTimeFromCnt(15), false);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[6].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[11].time_, replay->GetTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, replay->GetTimeFromCnt(15));
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[7].time_, replay->GetTimeFromCnt(15));

    replay->MoveToTime(replay->GetTimeFromCnt(30), false);
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

    std::filesystem::path cwd = std::filesystem::current_path();
    std::cout << cwd << std::endl;

    scenarioengine::Replay* replay = new scenarioengine::Replay("new_sim.dat");
    ASSERT_EQ(replay->pkgs_.size(), 3974);

    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    replay->MoveToTime(18, false);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, 17.150000255554914); // pos
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, 17.200000256299973); // speed
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[3].time_, 0);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[4].time_, 0);

    replay->MoveToTime(19.5, false);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, 17.150000255554914); // pos
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, 17.200000256299973); // speed
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[3].time_, 0);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[4].time_, 0);

    replay->MoveToTime(21, false);
    ASSERT_EQ(replay->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(replay->scenarioState.obj_states[0].pkgs.size(), 17);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[1].time_, 21.0); // pos
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[2].time_, 21.0); // speed
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[3].time_, 0);
    ASSERT_DOUBLE_EQ(replay->scenarioState.obj_states[0].pkgs[4].time_, 0);

}
#endif
int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}