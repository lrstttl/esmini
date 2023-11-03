#include <fstream>
#include <vector>

enum class PackageId {
    VERSION = 11,
    POSITIONS   = 12,
    TIME_SERIES = 13,
    ODR_FILENAME = 14,
    OSC_BOUNDING_BOX = 15,
};
#pragma pack(push, 4)
typedef struct
{
    int id;
    int size;
} CommonPackageHeader;

typedef struct
{
    int end_of_package;
} CommonPackageEnd;

typedef struct
{
    CommonPackageHeader hdr;
    unsigned int version;
    CommonPackageEnd package_end;
} PackageStructVersion;

typedef struct
{
    double x;
    double y;
    double z;
    double h;
    double r;
    double p;
} PackageStructPosition;


typedef struct
{
    CommonPackageHeader hdr;
    PackageStructPosition pos;
    CommonPackageEnd package_end;
} PackageStructPos;


typedef struct
{
    CommonPackageHeader hdr;
    double time;
    CommonPackageEnd package_end;
} PackageStructTime;

#pragma pack(pop)

class DatLogger {
private:
    std::fstream data_file_;

public:
    DatLogger() = default;
    ~DatLogger() {
        data_file_.flush();
        data_file_.close();
    }
    bool isFirstEntry = true;
    bool notFirstEnd = false;

    int init(std::string fileName);

    // Log a common package
    void logPackage(PackageStructVersion package);
    void logPackage(PackageStructPos package );
    void logPackage(PackageStructTime package );
    template <typename T>
    T getLatestPackage(const int id);

};

//main.cpp back up

#include <cstring>
#include <iostream>

#include "DatLogger.hpp"

int main(int, char**){
    int version = 2;
    std::string odr_fileName = "someScenario.xosc";
    double time_  = 12.20;
    std::string fileName = "sim.dat";

    double x = 1.0;
    double y = 2.0;
    double z = 3.0;
    double h = 4.0;
    double r = 5.0;
    double p = 6.0;

    float bb = 1.0;
    float bb1 = 2.0;


    DatLogger logger;
    logger.init(fileName);
    for (int i = 0; i < 8 ; i++)
    {
        if (i > 0)
        {
            logger.isFirstEntry = false;
        }
        if (i == 3)
        {
            version = 3;
        }
        if (i == 4)
        {
            h = 6.0;
        }
        PackageStructVersion packageVersion;
        packageVersion.hdr.id = static_cast<int>(PackageId::VERSION); // version id
        packageVersion.hdr.size = sizeof(version);
        packageVersion.version = version;
        packageVersion.package_end.end_of_package = sizeof(packageVersion);
        std::cout << "------Version Package->Send----- " << std::endl;
        // std::cout << "Complete Package Size: " << sizeof(packageVersion) << std::endl;
        // std::cout << "Package id Size: " << sizeof(packageVersion.hdr.id) << std::endl;
        // std::cout << "Package size Size: " << sizeof(packageVersion.hdr.size) << std::endl;
        // std::cout << "Package content Size: " << sizeof(packageVersion.version) << std::endl;
        // std::cout << "Package size end: " << sizeof(packageVersion.package_end) << std::endl;
        // std::cout << "------------------------- " << std::endl;
        logger.logPackage(packageVersion);
        std::cout << "------------------------- " << std::endl;

        PackageStructPos packagePos;
        packagePos.hdr.id = static_cast<int>(PackageId::POSITIONS); // version id
        packagePos.pos.x = x;
        packagePos.pos.y = y;
        packagePos.pos.z = z;
        packagePos.pos.h = h;
        packagePos.pos.r = r;
        packagePos.pos.p = p;
        packagePos.hdr.size = sizeof(packagePos.pos);
        packagePos.package_end.end_of_package = sizeof(packagePos);
        std::cout << "------Pos package->Send----- " << std::endl;
        // std::cout << "Complete Package Size: " << sizeof(packagePos) << std::endl;
        // std::cout << "Package id Size: " << sizeof(packagePos.hdr.id) << std::endl;
        // std::cout << "Package size Size: " << sizeof(packagePos.hdr.size) << std::endl;
        // std::cout << "Package content Size: " << sizeof(packagePos.pos) << std::endl;
        // std::cout << "------------------------- " << std::endl;
        logger.logPackage(packagePos);
        std::cout << "------------------------- " << std::endl;

        PackageStructTime packageTime;
        packageTime.hdr.id = static_cast<int>(PackageId::TIME_SERIES); // version id
        packageTime.hdr.size = sizeof(time_);
        packageTime.time = 12.20 + i;
        packageTime.package_end.end_of_package = sizeof(packageTime);
        std::cout << "------Time Package->Send----- " << std::endl;
        // // // std::cout << "Complete Package Size: " << sizeof(packageVersion) << std::endl;
        // // // std::cout << "Package id Size: " << sizeof(packageVersion.hdr.id) << std::endl;
        // // // std::cout << "Package size Size: " << sizeof(packageVersion.hdr.size) << std::endl;
        // // // std::cout << "Package content Size: " << sizeof(packageVersion.version) << std::endl;
        // // // std::cout << "------------------------- " << std::endl;
        logger.logPackage(packageTime);
        std::cout << "------------------------- " << std::endl;

    }

}


