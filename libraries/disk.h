#ifndef DISK_H
#define DISK_H
#include<vector>

#include <string>
#include <algorithm>
#include "../libraries/disk.h"
#include "../libraries/fdisk.h"
#include "../libraries/scanner.h"
#include "../libraries/structs.h"

// #include "../lib/shared.h"

using namespace std;

class Disk{
    public:
        Disk();
        

        void mkdisk(vector<string> tokens);
        void makeDisk(string s, string f, string u, string p);
        void rmdisk(vector<string> context);
        void fdisk(vector<string> context);
        //void fdisk_create(vector<string> context);
        //void fdisk_delete(vector<string> context);
        //void generatepartition(string s, string u, string p, string t, string f, string n, string a);
        //void deletepartition(string d, string p, string n);
        //void addpartition(string add, string u, string n, string p);
            
        vector<Structs::Partition> getPartitions(Structs::MBR disk);

        //Structs::MBR adjust(Structs::MBR mbr, Structs::Partition p, vector<Transition> t, vector<Structs::Partition> ps, int u);

        //Structs::Partition findby(Structs::MBR mbr, string name, string path);

        //void logic(Structs::Partition partition, Structs::Partition ep, string p);

        //vector<Structs::EBR> getlogics(Structs::Partition partition, string p);
        
    // private:
    //     Shared shared;   
    
};




#endif