#include <gtest/gtest.h>
#include <cstring>
#include <iostream>
#include <chrono>

#include "DatLogger.hpp"

using namespace datLogger;

#if (0)
TEST(LogOperationsWithOneObject, TestLogInitAndStepWithOneObject)
{
    std::string fileName = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name = "e6mini.osgb";
    int version_ = 2;

    DatLogger* logger = new DatLogger;

    int no_of_obj = 1;
    int pkg_nos = 2; // speed and pos pkg
    int total_time = 6;
    // calc
    // 1obj
    // 1 hdr, 6 time, 6 obj id, 6 pos, 6 speed  = 25 pkg  received
    // 1 hdr, 6 time, 6 obj id, 2 pos, 5 speed  = 20 pkg  written

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);
    logger->step(no_of_obj);
    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos));
    ASSERT_EQ(logger->totalPkgProcessed, 20);
    ASSERT_EQ(logger->totalPkgSkipped, 5);

    delete logger;

}

TEST(RecordOperationsWithOneObject, TestRecordInitWithOneObject)
{

    DatLogger* logger = new DatLogger;

    std::string fileName = "sim.dat";
    logger->recordPackage(fileName);
    ASSERT_EQ(logger->pkgs_.size(), 20);

    logger->initiateStates(logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.sim_time, logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(logger->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(logger->getTimeFromCnt(2), 1.122);

    logger->MoveToTime(logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(2));

    logger->MoveToTime(logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));

    logger->MoveToTime(logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 1);


    delete logger;
}

TEST(LogOperationsWithTwoObject, TestLogInitAndStepWithTwoObject)
{
    std::string fileName = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name = "e6mini.osgb";
    int version_ = 2;

    DatLogger* logger = new DatLogger;

    int no_of_obj = 2;
    int pkg_nos = 2; // speed and pos pkg
    int total_time = 6;
    // calc
    // 2obj
    // 1 hdr, 6 time, 12 obj id, 12 pos, 12 speed  = 43 pkg  received
    // 1 hdr, 6 time, 12 obj id, 4 pos, 10 speed  = 33 pkg  written

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);
    logger->step(no_of_obj);
    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos));
    ASSERT_EQ(logger->totalPkgProcessed, 33);
    ASSERT_EQ(logger->totalPkgSkipped, 10);

    delete logger;

}

TEST(TestRecordWithTwoObject, TestRecordWithTwoObject)
{
    DatLogger* logger = new DatLogger;

    std::string fileName = "sim.dat";
    logger->recordPackage(fileName);
    ASSERT_EQ(logger->pkgs_.size(), 33);

    logger->initiateStates(logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.sim_time, logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 2);
    ASSERT_EQ(logger->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(logger->getTimeFromCnt(2), 1.122);

    logger->MoveToTime(logger->getTimeFromCnt(2));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(2));

    logger->MoveToTime(logger->getTimeFromCnt(4));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));

    logger->MoveToTime(logger->getTimeFromCnt(5));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));

    delete logger;

}

TEST(LogOperationsWithThreeObject, TestLogInitAndStepWithThreeObject)
{
    std::string fileName = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name = "e6mini.osgb";
    int version_ = 2;

    DatLogger* logger = new DatLogger;

    int no_of_obj = 3;
    int pkg_nos = 2; // speed and pos pkg
    int total_time = 6;
    int no_of_obj_deleted = 1;

    //calc
    // 3obj- one obj deleted so 1 obj id  + pos + speed pkg less
    // 1 hdr, 6 time, 18 obj id, 18 pos, 18 speed  =58 pkg  received
    // 1 hdr, 6 time, 17 obj id, 6 pos, 14 speed  = 45 pkg  written (while add obj . all pkg to be added)

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);
    logger->step(no_of_obj);
    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos) - (pkg_nos + no_of_obj_deleted));
    ASSERT_EQ(logger->totalPkgProcessed, 45);
    ASSERT_EQ(logger->totalPkgSkipped, 13);

    delete logger;

}

TEST(TestRecordWithThreeObject, TestRecordWithThereObject)
{
    DatLogger* logger = new DatLogger;

    std::string fileName = "sim.dat";
    logger->recordPackage(fileName);
    ASSERT_EQ(logger->pkgs_.size(), 45);

    logger->initiateStates(logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.sim_time, logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(logger->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(logger->getTimeFromCnt(2), 1.122);

    logger->MoveToTime(logger->getTimeFromCnt(2));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 3);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[1].time_, logger->getTimeFromCnt(2));

    logger->MoveToTime(logger->getTimeFromCnt(3));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 2); // obj deleted
    ASSERT_EQ(logger->scenarioState.obj_states[0].id, 0);
    ASSERT_EQ(logger->scenarioState.obj_states[1].id, 1);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(3));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(3));

    logger->MoveToTime(logger->getTimeFromCnt(4));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 3); // obj added
    ASSERT_EQ(logger->scenarioState.obj_states[0].id, 0);
    ASSERT_EQ(logger->scenarioState.obj_states[1].id, 1);
    ASSERT_EQ(logger->scenarioState.obj_states[2].id, 2);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[0].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[1].time_, logger->getTimeFromCnt(4));

    logger->MoveToTime(logger->getTimeFromCnt(5));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 3);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[1].time_, logger->getTimeFromCnt(4));

    delete logger;

}
# endif


TEST(LogOperationsWithOneObject, TestLogInitAndStepWithOneObject)
{
    std::string fileName = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name = "e6mini.osgb";
    int version_ = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);

    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    double h = 4.0;
    double r = 5.0;
    double p = 6.0;
    double speed = 1.0;

    double current_time = 0.033;
    int no_of_obj = 1;


    int pkg_nos = 2; // speed and pos pkg
    int total_time = 6;
    // calc
    // 1obj
    // 1 hdr, 6 time, 6 obj id, 6 pos, 6 speed  = 25 pkg  received
    // 1 hdr, 6 time, 6 obj id, 2 pos, 5 speed  = 20 pkg  written

    for (int i = 0; i < total_time ; i++)
    {
        if (i == 4 || i == 5)
        {
            h = 6.0;
        }
        logger->WriteTime(current_time);
        for ( int j = 0; j < no_of_obj; j++)
        {
            if ( i == 2 && j == 2)
            {
                break; // delete one object.
            }
            int object_id = j;
            logger->WriteObjId(object_id);
            logger->WriteObjPos( object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
        }
        if (i != 3 )
        {
            speed += 1.0;
        }
        current_time += 1.089;
    }

    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos));
    ASSERT_EQ(logger->totalPkgProcessed, 20);
    ASSERT_EQ(logger->totalPkgSkipped, 5);

    delete logger;

}

TEST(RecordOperationsWithOneObject, TestRecordInitWithOneObject)
{

    DatLogger* logger = new DatLogger;

    std::string fileName = "sim.dat";
    logger->recordPackage(fileName);
    ASSERT_EQ(logger->pkgs_.size(), 20);

    logger->initiateStates(logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.sim_time, logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 1);
    ASSERT_EQ(logger->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(logger->getTimeFromCnt(2), 1.122);

    logger->MoveToTime(logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(2));

    logger->MoveToTime(logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));

    logger->MoveToTime(logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 1);


    delete logger;
}

TEST(LogOperationsWithTwoObject, TestLogInitAndStepWithTwoObject)
{
    std::string fileName = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name = "e6mini.osgb";
    int version_ = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);

    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    double h = 4.0;
    double r = 5.0;
    double p = 6.0;
    double speed = 1.0;

    double current_time = 0.033;

    int no_of_obj = 2;
    int pkg_nos = 2; // speed and pos pkg
    int total_time = 6;
    // calc
    // 2obj
    // 1 hdr, 6 time, 12 obj id, 12 pos, 12 speed  = 43 pkg  received
    // 1 hdr, 6 time, 12 obj id, 4 pos, 10 speed  = 33 pkg  written

    for (int i = 0; i < total_time ; i++)
    {
        if (i == 4 || i == 5)
        {
            h = 6.0;
        }
        logger->WriteTime(current_time);
        for ( int j = 0; j < no_of_obj; j++)
        {
            if ( i == 2 && j == 2)
            {
                break; // delete one object.
            }
            int object_id = j;
            logger->WriteObjId(object_id);
            logger->WriteObjPos( object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
        }
        if (i != 3 )
        {
            speed += 1.0;
        }
        current_time += 1.089;
    }

    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos));
    ASSERT_EQ(logger->totalPkgProcessed, 33);
    ASSERT_EQ(logger->totalPkgSkipped, 10);

    delete logger;

}

TEST(TestRecordWithTwoObject, TestRecordWithTwoObject)
{
    DatLogger* logger = new DatLogger;

    std::string fileName = "sim.dat";
    logger->recordPackage(fileName);
    ASSERT_EQ(logger->pkgs_.size(), 33);

    logger->initiateStates(logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.sim_time, logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 2);
    ASSERT_EQ(logger->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(logger->getTimeFromCnt(2), 1.122);

    logger->MoveToTime(logger->getTimeFromCnt(2));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(2));

    logger->MoveToTime(logger->getTimeFromCnt(4));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));

    logger->MoveToTime(logger->getTimeFromCnt(5));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));

    delete logger;

}

TEST(LogOperationsWithThreeObject, TestLogInitAndStepWithThreeObject)
{
    std::string fileName = "sim.dat";
    std::string odrFileName = "e6mini.xodr";
    std::string model_Name = "e6mini.osgb";
    int version_ = 2;

    DatLogger* logger = new DatLogger;

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);

    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    double h = 4.0;
    double r = 5.0;
    double p = 6.0;
    double speed = 1.0;

    double current_time = 0.033;

    int no_of_obj = 3;
    int pkg_nos = 2; // speed and pos pkg
    int total_time = 6;
    int no_of_obj_deleted = 1;

    //calc
    // 3obj- one obj deleted so 1 obj id  + pos + speed pkg less
    // 1 hdr, 6 time, 18 obj id, 18 pos, 18 speed  =58 pkg  received
    // 1 hdr, 6 time, 17 obj id, 6 pos, 14 speed  = 45 pkg  written (while add obj . all pkg to be added)

    for (int i = 0; i < total_time ; i++)
    {
        if (i == 4 || i == 5)
        {
            h = 6.0;
        }
        logger->WriteTime(current_time);
        for ( int j = 0; j < no_of_obj; j++)
        {
            if ( i == 2 && j == 2)
            {
                logger->deleteObjState(j);
                break; // delete one object.
            }
            int object_id = j;
            logger->WriteObjId(object_id);
            logger->WriteObjPos( object_id, x, y, z, h, p, r);
            logger->WriteObjSpeed(object_id, speed);
        }
        if (i != 3 )
        {
            speed += 1.0;
        }
        current_time += 1.089;
    }

    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos) - (pkg_nos + no_of_obj_deleted));
    ASSERT_EQ(logger->totalPkgProcessed, 45);
    ASSERT_EQ(logger->totalPkgSkipped, 13);

    delete logger;

}

TEST(TestRecordWithThreeObject, TestRecordWithThereObject)
{
    DatLogger* logger = new DatLogger;

    std::string fileName = "sim.dat";
    logger->recordPackage(fileName);
    ASSERT_EQ(logger->pkgs_.size(), 45);

    logger->initiateStates(logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.sim_time, logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 3);
    ASSERT_EQ(logger->scenarioState.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(logger->getTimeFromCnt(2), 1.122);

    logger->MoveToTime(logger->getTimeFromCnt(2));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 3);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[1].time_, logger->getTimeFromCnt(2));

    logger->MoveToTime(logger->getTimeFromCnt(3));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 2); // obj deleted
    ASSERT_EQ(logger->scenarioState.obj_states[0].id, 0);
    ASSERT_EQ(logger->scenarioState.obj_states[1].id, 1);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(3));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(3));

    logger->MoveToTime(logger->getTimeFromCnt(4));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 3); // obj added
    ASSERT_EQ(logger->scenarioState.obj_states[0].id, 0);
    ASSERT_EQ(logger->scenarioState.obj_states[1].id, 1);
    ASSERT_EQ(logger->scenarioState.obj_states[2].id, 2);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[0].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[1].time_, logger->getTimeFromCnt(4));

    logger->MoveToTime(logger->getTimeFromCnt(5));
    ASSERT_EQ(logger->scenarioState.obj_states.size(), 3);
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->scenarioState.obj_states[2].pkgs[1].time_, logger->getTimeFromCnt(4));

    delete logger;

}

int main(int argc, char **argv){

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}