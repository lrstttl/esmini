#include <fstream>
#include <vector>

enum class PackageId {
    VERSION = 11,
    POSITIONS   = 12,
    TIME_SERIES = 13,
    ODR_FILENAME = 14,
    OSC_BOUNDING_BOX = 15,
};

typedef struct
{
    int id;
    int size;
} CommonPackageHeader;

typedef struct
{
    CommonPackageHeader hdr;
    unsigned int version;
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
    } PackageStructPos;

    typedef struct
    {
        CommonPackageHeader hdr;
        double time;
    } PackageStructTime;

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

    int init(std::string fileName);

    // Log a common package
    void logPackage(PackageStructVersion package);
    void logPackage(PackageStructPos package );
    void logPackage(PackageStructTime package );
    template <typename T>
    T getLatestPackage(const int id);

};


