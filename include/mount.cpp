#include "../libraries/mount.h"

#include <iostream>
#include <stdlib.h>
#include "string"
#include <locale>

using namespace std;

Mount::Mount(){}


Structs::Partition Mount::getmount(string id, string *p) {

    string past = id;
    char letter = id[id.length() - 1];
    id.erase(0, 2);
    id.pop_back();
    int i = stoi(id) - 1;
    if (i < 0) {
        throw runtime_error("identificador de disco inválido");
    }

    for (int j = 0; j < 26; j++) {
        if (mounted[i].mpartitions[j].status == '1') {
            if (mounted[i].mpartitions[j].letter == letter) {

                FILE *validate = fopen(mounted[i].path, "r");
                if (validate == NULL) {
                    throw runtime_error("disco no existente");
                }

                Structs::MBR disk;
                rewind(validate);
                fread(&disk, sizeof(Structs::MBR), 1, validate);
                fclose(validate);
                *p = mounted[i].path;
                return dsk.findby(disk, mounted[i].mpartitions[j].name, mounted[i].path);
            }
        }
    }
    throw runtime_error("partición no existente");
}
