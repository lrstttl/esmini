#include <fstream>
#include <vector>
#include <map>

namespace DatLogger
{
    enum class PackageId {
        HEADER      = 11,
        TIME_SERIES = 12,
        OBJ_ID      = 13,
        POSITIONS   = 14,
        SPEED       = 15,
    };

    // mandatory packages
    typedef struct
    {
        int id;
        int content_size;
    } CommonPkgHdr;

    typedef struct
    {
        unsigned long long pkg_size;  // avoid padding for 64 bit alignment
    } CommonPkgEnd;

    // common package types
    typedef struct
    {
        int size;  // size of the string
        char *string;
    } CommonString;

    // specific packages
    typedef struct
    {
        int version;
        CommonString odrFilename;
        CommonString modelFilename;
    } DatHdr;

    typedef struct
    {
        double time;
    } Time;

    typedef struct
    {
        double speed_;
    } Speed;

    typedef struct
    {
        int obj_id;
    } ObjId;

    typedef struct
    {
        double x;
        double y;
        double z;
        double h;
        double r;
        double p;
    } Pos;

    typedef struct
    {
        CommonPkgHdr  hdr;
        char*         content;   // pointer to allocated content
        CommonPkgEnd  end;
    } CommonPkg;

    typedef struct
    {
        double       time_;
        char*         pkg;
    } ObjectStateWithTime;

    typedef struct
    {
        int       id;
        std::vector<ObjectStateWithTime> states;
    } ObjectState;

    enum class Mode
    {
        WRITE = 0, // simple write
        READ = 1, // simple read
        LOG = 2,  // log packages
        RECORD = 3, // record all packages in vector
        REPLAY = 4, // method to fetch package
        BOTH_SIMPLE = 5,
        BOTH = 6,
        LOG_PKG = 7,
        SIMPLE = 8 // isolated problem checking
    } ;

    class DatLogger {
    private:
        std::fstream data_file_;
    public:
        DatLogger() = default;
        ~DatLogger() {
            data_file_.flush();
            data_file_.close();
            if (system_mode == Mode::LOG)
            {
                std::cout << "LOG Summary: "<<std::endl;
                std::cout << "Total Package Received: " << totalPkgReceived <<std::endl;
                std::cout << "Total Package logged: " << totalPkgProcessed <<std::endl;
                std::cout << "Total Package Skipped: " << totalPkgSkipped <<std::endl;
            }
            if (system_mode == Mode::RECORD)
            {
                std::cout << "Record Summary: "<<std::endl;
                std::cout << "Total Package Received: " << totalPkgReceived <<std::endl;
                std::cout << "Total Package Recorded: " << totalPkgProcessed <<std::endl;
                std::cout << "Total Package Skipped: " << totalPkgSkipped <<std::endl;
            }

            std::cout << "File closed successfully in destructure" << std::endl;
        }
        bool isFirstEntry = true;
        bool notFirstEnd = false;
        bool display_print = false;
        int totalPkgProcessed = 0;
        int totalPkgReceived = 0;
        int totalPkgSkipped = 0;
        std::vector<CommonPkg> pkgs_;
        std::vector<ObjectState> objectStates_;

        Mode system_mode;
        void initiateStates(double time_frame);
        void updateStates(double time_frame);
        PackageId readPkgHdr(char* package );

        int init(std::string fileName, int ver, std::string odrName, std::string modelName);

        void logPackage(CommonPkg package, const int object_id); // check package can be logged or not
        void writePackage(CommonPkg package ); // will just write package
        int recordPackage(const std::string& fileName); // check package can be recorded or not
        std::vector<int> getObjIdPkgIndexBtwTime( double t); // till next time forward
        int getPkgCntBtwObj( int idx, int id); // till next time forward
        std::string pkgIdTostring(PackageId id);

        template <typename T>
        T getLatestPackage(const int id, const unsigned long long pkgSize, const int contentSize, const int obj_id);

    };

}