#ifndef REP_H
#define REP_H
#include <vector>
#include <string>

#include "../libraries/structs.h"
#include "../libraries/mount.h"
#include "../libraries/disk.h"
#include "../libraries/scanner.h"

using namespace std;

class Reporter{
    private:
        FDisk fdisco;
        Disk disco;
        scanner scan;

    public:
        Reporter();
        void do_report(vector<string> context/*, Mount m*/);
        void report_mbr(string path, string id);
        string getPath(string id);
        string dotMBR(Structs::MBR mbr, string path);
        string dotPartition(Structs::Partition partition, string path);
        string dotLogicPartition(Structs::EBR extended);
        //Mount mount;
        void report_disk(string pathReport, string id);
        string dotDisk(string pathReport);
};

#endif