#ifndef FDISK_H
#define FDISK_H
#include<vector>

#include <string>
#include <algorithm>
#include "../libraries/disk.h"
#include "../libraries/fdisk.h"
#include "../libraries/scanner.h"
#include "../libraries/structs.h"

using namespace std;

class FDisk{
    public:
        FDisk();
        typedef struct _Transition{
            int partition;
            int start;
            int end;
            int before;
            int after;
        } Transition;

        void fdisk_create(vector<string> context);
        void fdisk_delete(vector<string> context);
        void generatePartition(string s, string u, string p, string t, string f, string n, string a);
        void deletePartition(string d, string p, string n);
        void addPartition(string add, string u, string n, string p);     

        vector<Structs::Partition> getPartitions(Structs::MBR disk);        
        Structs::MBR adjust(Structs::MBR mbr, Structs::Partition p, vector<Transition> t, vector<Structs::Partition> ps, int u);
        Structs::Partition findPartitionby(Structs::MBR mbr, string name, string path);
        void logic(Structs::Partition partition, Structs::Partition ep, string p);
        vector<Structs::EBR> getlogicPartitions(Structs::Partition partition, string p);
    
    private:
        scanner scan;
        int startValue;
};


#endif