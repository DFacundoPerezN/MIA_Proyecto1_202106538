#include "../libraries/mount.h"

#include <iostream>
#include <stdlib.h>
#include "string"
#include <locale>

using namespace std;

Mount::Mount(){}

void Mount::do_mount(vector<string> parameters) {
    if (parameters.empty()) {
        //listmount();
        return;
    }
    vector<string> required = {"name", "path"};
    string path;
    string name;

    for (auto current : parameters) {
        string parameter = shared.lowerString(current.substr(0, current.find('=')));
        current.erase(0, parameter.length() + 1);
        if (current.substr(0, 1) == "\"") {
            current = current.substr(1, current.length() - 2);
        }

        if (shared.compare(parameter, "name")) {
            if (count(required.begin(), required.end(), parameter)) {
                auto itr = find(required.begin(), required.end(), parameter);
                required.erase(itr);
                name = current;
            }
        } else if (shared.compare(parameter, "path")) {
            if (count(required.begin(), required.end(), parameter)) {
                auto itr = find(required.begin(), required.end(), parameter);
                required.erase(itr);
                path = current;
            }
        }
        //cout << "Parametro: " << current << endl;
    }
    if (required.size() != 0) {
        shared.handler("MOUNT", "requiere ciertos parámetros obligatorios");
        return;
    }
    mountPartition(path, name);
}

void Mount::mountPartition(string path, string name) {
    try {
        FILE *validate = fopen(path.c_str(), "r");
        if (validate == NULL) {
            throw runtime_error("disco no existente");
        }

        Structs::MBR disk;
        rewind(validate);
        fread(&disk, sizeof(Structs::MBR), 1, validate);
        fclose(validate);
        //listMounts();

        cout << "Buscando montaje mediante el path "  << endl;
        Structs::Partition partition = fdisk.findPartitionby(disk, name, path);

        if (partition.part_type == 'E') {
            vector<Structs::EBR> ebrs = fdisk.getlogics(partition, path);
            if (!ebrs.empty()) {
                Structs::EBR ebr = ebrs.at(0);
                name = ebr.part_name;
                shared.handler("", "se montará una partición lógica");
            } else {
                throw runtime_error("no se puede montar una extendida");
            }
        }
        cout << "Buscando montaje con el mismo path "  << endl;
        for (int i = 0; i < 99; i++) {
            if (mountedDiscs[i].path == path) {
                for (int j = 0; j < 26; j++) {
                    if (Mount::mountedDiscs[i].mpartitions[j].status == '0') {
                        mountedDiscs[i].mpartitions[j].status = '1';
                        mountedDiscs[i].mpartitions[j].letter = alfabeto.at(j);
                        strcpy(mountedDiscs[i].mpartitions[j].name, name.c_str());
                        string re = to_string(i + 1) + alfabeto.at(j);
                        shared.handler("MOUNT", "se ha realizado correctamente el mount -id=38" + re);
                        return;
                    }
                }
            }
        }
        cout << "Buscando espacio libre para montar particion"  << endl;
        for (int i = 0; i < 99; i++) {
            if (mountedDiscs[i].status == '0') {
                mountedDiscs[i].status = '1';
                strcpy(mountedDiscs[i].path, path.c_str());
                for (int j = 0; j < 26; j++) {
                    if (Mount::mountedDiscs[i].mpartitions[j].status == '0') {
                        mountedDiscs[i].mpartitions[j].status = '1';
                        mountedDiscs[i].mpartitions[j].letter = alfabeto.at(j);
                        strcpy(mountedDiscs[i].mpartitions[j].name, name.c_str());
                        string re = to_string(i + 1) + alfabeto.at(j);
                        shared.handler("MOUNT", "se ha realizado correctamente el mount -id=38" + re);
                        return;
                    }
                }
            }
        }
    }
    catch (exception &e) {
        cout << "Ha ocurrido un error"  << endl;
        shared.handler("MOUNT", e.what());
        return;
    }
}

Structs::Partition Mount::getmount(string id, string *path) {

    string past = id;
    char letter = id[id.length() - 1];
    id.erase(0, 2);
    id.pop_back();
    int i = stoi(id) - 1;
    if (i < 0) {
        throw runtime_error("identificador de disco inválido");
    }

    for (int j = 0; j < 26; j++) {
        if (mountedDiscs[i].mpartitions[j].status == '1') {
            if (mountedDiscs[i].mpartitions[j].letter == letter) {

                FILE *validate = fopen(mountedDiscs[i].path, "r");
                if (validate == NULL) {
                    throw runtime_error("disco no existente");
                }

                Structs::MBR disk;
                rewind(validate);
                fread(&disk, sizeof(Structs::MBR), 1, validate);
                fclose(validate);
                *path = mountedDiscs[i].path;
                return fdisk.findPartitionby(disk, mountedDiscs[i].mpartitions[j].name, mountedDiscs[i].path);
            }
        }
    }
    throw runtime_error("partición no existente");
}

void Mount:: do_unmount(vector<string> context) {
    vector<string> required = {"id"};
    string id_;

    for (int i = 0; i < context.size(); i++) {
        string current = context.at(i);
        string id = current.substr(0, current.find("="));
        current.erase(0, id.length() + 1);

        if (shared.compare(id, "id")) {
            auto itr = find(required.begin(), required.end(), id);
            required.erase(itr);
            id_ = current;
        }
    }
    if (required.size() != 0) {
        shared.handler("UNMOUNT", "requiere ciertos parámetros obligatorios");
        return;
    }
    unmountPartition(id_);
}

void Mount::unmountPartition(string id) {
    try {
        if (!(id[0] == '6' && id[1] == '5')) {
            throw runtime_error("el primer identificador no es válido");
        }
        string past = id;
        char letter = id[id.length() - 1];
        id.erase(0, 2);
        id.pop_back();
        int i = stoi(id) - 1;
        if (i < 0) {
            throw runtime_error("identificador de disco inválido");
        }

        for (int j = 0; j < 26; j++) {
            if (mountedDiscs[i].mpartitions[j].status == '1') {
                if (mountedDiscs[i].mpartitions[j].letter == letter) {

                    MountedPartition mp = MountedPartition();
                    mountedDiscs[i].mpartitions[j] = mp;
                    shared.handler("UNMOUNT", "se ha realizado correctamente el unmount -id=" + past);
                    return;
                }
            }
        }
        throw runtime_error("id no existente, no se desmontó nada");
    }
    catch (invalid_argument &e) {
        shared.handler("UNMOUNT", "identificador de disco incorrecto, debe ser entero");
        return;
    }
    catch (exception &e) {
        shared.handler("UNMOUNT", e.what());
        return;
    }
}

void Mount::listMounts() {
    cout << "\n Listados de Mounts:"
         << endl;
    for (int i = 0; i < 99; i++) {
        for (int j = 0; j < 26; j++) {
            if (mountedDiscs[i].mpartitions[j].status == '1') {
                cout << "> 87" << i + 1 << alfabeto.at(j) << ", " << mountedDiscs[i].mpartitions[j].name << endl;
            }
        }
    }
}