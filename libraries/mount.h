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
        char letter;
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

    void do_mount(vector<string> context);

    void do_unmount(vector<string> context);

    void mountPartition(string p, string n);

    void unmountPartition(string id);

    void listMounts();

    Structs::Partition getmount(string id, string *p);

    private:
    Disk dsk;
    FDisk fdisk;
    scanner shared;
    vector<char> alfabeto = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                             's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
};
#endif