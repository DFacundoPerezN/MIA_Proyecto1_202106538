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
        Disk disco;
        scanner scan;
        Mount mount;

    public:
        Reporter();
        void do_report(vector<string> context/*, Mount m*/);
        void report_mbr(string path, string id);
};


#endif