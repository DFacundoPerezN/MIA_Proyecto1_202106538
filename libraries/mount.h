#ifndef MOUNT_H
#define MOUNT_H

#include <string>
#include <bits/stdc++.h>
#include "../libraries/scanner.h"
#include "../libraries/structs.h"
#include "../libraries/disk.h"

using namespace std;

class Mount {
    public:
        Mount();

        typedef struct _MP
        {
            string diskName;
            char status = '0';
            char name[20];
        }MountedPartition;

        typedef struct _MD
        {
            char path[150];
            char status = '0';
            MountedPartition mpartitions[26];
        }MountedDisc;

        MountedDisc mountedDiscs[99];

        string DiskNameFromPath(string path);

        void do_mount(vector<string> context);

        void do_unmount(vector<string> context);

        void mountPartition(string p, string n);

        void unmountPartition(string id);

        void listMounts();
        string getDiskName(string path);
        Structs::Partition getmount(string id, string *p);

    private:
        Disk dsk;
        FDisk fdisk;
        scanner shared;
    
};
#endif