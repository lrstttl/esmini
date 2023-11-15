#include <fstream>
#include <vector>

namespace DatLogger
{
    enum class PackageId {
        HEADER      = 11,
        TIME_SERIES = 12,
        OBJ_ID      = 13,
        POSITIONS   = 14,
        SPEED       = 15,
    };

    // #pragma pack(push, 4)

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

#if 0
    typedef struct
    {
        union
        {
            DatHdr hdr;
            Time   time;
            ObjId  id;
            Pos    pos;
        } content;
    } CommonContent;

    typedef struct
    {
        CommonPkgHdr  hdr;
        CommonContent content;
        CommonPkgEnd  end;
    } CommonPkg;
#else
    typedef struct
    {
        CommonPkgHdr  hdr;
        char*         content;   // pointer to allocated content
        CommonPkgEnd  end;
    } CommonPkg;
#endif
    // #pragma pack(pop)


    class DatLogger {
    private:
        std::fstream data_file_;
    public:
        DatLogger() = default;
        ~DatLogger() {
            data_file_.flush();
            data_file_.close();
            std::cout << "File closed successfully in destructure" << std::endl;
        }
        bool isFirstEntry = true;
        bool notFirstEnd = false;
        bool display_print = false;
        std::vector<CommonPkg> pkgs_;

        int init(std::string fileName, int ver, std::string odrName, std::string modelName);

        void logPackage(CommonPkg package, const int object_id); // check package can be logged or not
        void writePackage(CommonPkg package ); // will just write package
        int recordPackage(const std::string& fileName); // check package can be recorded or not


        template <typename T>
        T getLatestPackage(const int id, const unsigned long long pkgSize, const int contentSize, const int obj_id);

    };

}