#include <gtest/gtest.h>
#include <cstring>
#include <iostream>
#include <chrono>

#include "DatLogger.hpp"

using namespace datLogger;


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
    ASSERT_EQ(logger->objectStates_.sim_time, logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->objectStates_.obj_states.size(), 1);
    ASSERT_EQ(logger->objectStates_.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(logger->getTimeFromCnt(2), 1.122);

    logger->updateStates(logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(2));

    logger->updateStates(logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));

    logger->updateStates(logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_EQ(logger->objectStates_.obj_states.size(), 1);


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
    ASSERT_EQ(logger->objectStates_.sim_time, logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->objectStates_.obj_states.size(), 2);
    ASSERT_EQ(logger->objectStates_.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(logger->getTimeFromCnt(2), 1.122);

    logger->updateStates(logger->getTimeFromCnt(2));
    ASSERT_EQ(logger->objectStates_.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(2));

    logger->updateStates(logger->getTimeFromCnt(4));
    ASSERT_EQ(logger->objectStates_.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));

    logger->updateStates(logger->getTimeFromCnt(5));
    ASSERT_EQ(logger->objectStates_.obj_states.size(), 2);
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(5));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));

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

    logger->init(fileName, version_, odrFileName, model_Name);
    ASSERT_EQ(logger->totalPkgReceived, 1);
    ASSERT_EQ(logger->totalPkgSkipped, 0);
    logger->step(no_of_obj);
    ASSERT_EQ(logger->totalPkgReceived, 1 + total_time + (total_time * no_of_obj) + (no_of_obj * total_time * pkg_nos) - (pkg_nos + no_of_obj_deleted));
    ASSERT_EQ(logger->totalPkgProcessed, 44);
    ASSERT_EQ(logger->totalPkgSkipped, 14);

    delete logger;

}

TEST(TestRecordWithThreeObject, TestRecordWithThereObject)
{
    DatLogger* logger = new DatLogger;

    std::string fileName = "sim.dat";
    logger->recordPackage(fileName);
    ASSERT_EQ(logger->pkgs_.size(), 44);

    logger->initiateStates(logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->objectStates_.sim_time, logger->getTimeFromCnt(1));
    ASSERT_EQ(logger->objectStates_.obj_states.size(), 3);
    ASSERT_EQ(logger->objectStates_.obj_states[0].pkgs.size(), 2);
    ASSERT_DOUBLE_EQ(logger->getTimeFromCnt(2), 1.122);

    logger->updateStates(logger->getTimeFromCnt(2));
    ASSERT_EQ(logger->objectStates_.obj_states.size(), 3);
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(2));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[2].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[2].pkgs[1].time_, logger->getTimeFromCnt(2));

    logger->updateStates(logger->getTimeFromCnt(3));
    ASSERT_EQ(logger->objectStates_.obj_states.size(), 2); // obj deleted
    ASSERT_EQ(logger->objectStates_.obj_states[0].id, 0);
    ASSERT_EQ(logger->objectStates_.obj_states[1].id, 1);
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(3));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(3));

    logger->updateStates(logger->getTimeFromCnt(4));
    ASSERT_EQ(logger->objectStates_.obj_states.size(), 3); // obj added
    ASSERT_EQ(logger->objectStates_.obj_states[0].id, 0);
    ASSERT_EQ(logger->objectStates_.obj_states[1].id, 1);
    ASSERT_EQ(logger->objectStates_.obj_states[2].id, 2);
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[0].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[0].time_, logger->getTimeFromCnt(1));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[1].pkgs[1].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[2].pkgs[0].time_, logger->getTimeFromCnt(4));
    ASSERT_DOUBLE_EQ(logger->objectStates_.obj_states[2].pkgs[1].time_, logger->getTimeFromCnt(4));

    delete logger;

}


int main(int argc, char **argv){

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}