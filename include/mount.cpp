#include "../libraries/mount.h"

#include <iostream>
#include <stdlib.h>
#include "string"
#include <locale>

using namespace std;
int codigoLetter = 97;

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

string Mount::DiskNameFromPath(string path) {
    size_t posInicio = path.find_last_of('/');  // Encontrar la última posición del separador de directorios '/'
    // Si no se encuentra, empezamos desde el principio

    if (posInicio == string::npos) {
        posInicio = 0;
    } else {
        
        posInicio++; // Avanzar una posición para omitir el '/'
    }
    
    size_t posFinal = path.find_last_of('.');   // Encontrar la última posición del separador de extensión '.'
    
    if (posFinal == string::npos) {// Si no se encuentra, la extensión no existe y tomamos el resto de la cadena
        posFinal = path.length();
    }

    // Extraer el nombre del archivo sin la extensión
    string nombreArchivo = path.substr(posInicio, posFinal - posInicio);

    return nombreArchivo;
}

void Mount::mountPartition(string pathMBR, string name) {
    try {
        FILE *validate = fopen(pathMBR.c_str(), "r");
        if (validate == NULL) {
            throw runtime_error("disco no existente");
        }

        Structs::MBR disk;
        rewind(validate);
        fread(&disk, sizeof(Structs::MBR), 1, validate);
        fclose(validate);
        //listMounts();

        cout << "Buscando montaje mediante el path "  << endl;
        Structs::Partition partition = fdisk.findPartitionby(disk, name, pathMBR);

        if (partition.part_type == 'E') {
            vector<Structs::EBR> ebrs = fdisk.getlogics(partition, pathMBR);
            if (!ebrs.empty()) {
                Structs::EBR ebr = ebrs.at(0);
                name = ebr.part_name;
                shared.respuesta("", "se montará una partición lógica");
            } else {
                throw runtime_error("no se puede montar una extendida");
            }
        }

        cout << "Buscando montaje con el mismo path "  << endl;
        for (int i = 0; i < 99; i++) {
            if (mountedDiscs[i].path == pathMBR) {
                for (int j = 0; j < 26; j++) {
                    if (Mount::mountedDiscs[i].mpartitions[j].status == '0') {
                        mountedDiscs[i].mpartitions[j].letter == codigoLetter + j;
                        codigoLetter++;
                        mountedDiscs[i].mpartitions[j].status = '1';
                        mountedDiscs[i].mpartitions[j].diskName = DiskNameFromPath(pathMBR);
                        strcpy(mountedDiscs[i].mpartitions[j].name, name.c_str());
                        string id_MD = "38" + to_string(i + 1) + mountedDiscs[i].mpartitions[j].diskName;
                        shared.respuesta("MOUNT", "se ha realizado correctamente el mount -id=" + id_MD+
                        " -path="+pathMBR+" -name="+name+ " en mountedDiscs["+to_string(i)+"].mpartitions["+to_string(j)+"]");
                        listMounts();
                        return;
                    }
                }
            }
        }
        cout << "Buscando espacio libre para montar particion"  << endl;
        for (int i = 0; i < 99; i++) {
            if (mountedDiscs[i].status == '0') {
                mountedDiscs[i].status = '1';
                strcpy(mountedDiscs[i].path, pathMBR.c_str());
                for (int j = 0; j < 26; j++) {
                    if (Mount::mountedDiscs[i].mpartitions[j].status == '0') {
                        
                        mountedDiscs[i].mpartitions[j].status = '1';
                        mountedDiscs[i].mpartitions[j].diskName = DiskNameFromPath(pathMBR);
                        strcpy(mountedDiscs[i].mpartitions[j].name, name.c_str());
                        string id_MD = "38" + to_string(i + 1) + mountedDiscs[i].mpartitions[j].diskName;
                        shared.respuesta("MOUNT", "se ha realizado correctamente el mount -id=" + id_MD+
                        " -path="+pathMBR+" status="+mountedDiscs[i].mpartitions[j].status+ " en mountedDiscs["+to_string(i)+"].mpartitions["+to_string(j)+"]");
                        listMounts();
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

string getPartitionNumber(string ID){  //381Disco1
    for (int i = 2; i < ID.length(); i++) {
        if (isalpha(ID[i])) {
            return ID.substr(2, i-2);
        }
    }
    string salida = ID[2]+"";
    return salida;
}

string Mount:: getDiskName(string ID){  //381Disco1
    for (int i = 2; i < ID.length(); i++) {
        if (isalpha(ID[i])) {
            return ID.substr(i, ID.length());
        }
    }
    string salida = ID.substr(3, ID.length() - 2);
    return salida;
}

Structs::Partition Mount::getmount(string id, string *path) {
    listMounts();

    string past = id;
    string diskName = getDiskName(id);
    id.erase(0, 2);
    id.pop_back();

    int i = stoi(id) - 1;

    if (i < 0) {
        throw runtime_error("identificador de disco inválido");
    }

    for (int j = 0; j < 26; j++) {
        //cout << "Comparando " << mountedDiscs[i].mpartitions[j].status << " con 1" << endl;
        if (mountedDiscs[i].mpartitions[j].status == '1') {
            //cout << "Comparando " << mountedDiscs[i].mpartitions[j].diskName << " con " << diskName << endl;
            if (mountedDiscs[i].mpartitions[j].diskName == diskName) {

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
        if (!(id[0] == '3' && id[1] == '8')) {
            throw runtime_error("el primer identificador no es válido");
        }
        string past = id;

        string Diskname = getDiskName(id);
        id.erase(0, 2);

        id.pop_back();
        int i = stoi(id) - 1;
        if (i < 0) {
            throw runtime_error("identificador de disco inválido");
        }

        for (int j = 0; j < 26; j++) {
            if (mountedDiscs[i].mpartitions[j].status == '1') {
                if (mountedDiscs[i].mpartitions[j].diskName == Diskname) {

                    MountedPartition mp = MountedPartition();
                    mountedDiscs[i].mpartitions[j] = mp;
                    shared.respuesta("UNMOUNT", "se ha realizado correctamente el unmount -id=" + past);
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
    cout << "\n Listados de Particiones Montadas :"
         << endl;
    for (int i = 0; i < 99; i++) {
        for (int j = 0; j < 26; j++) {
            if (mountedDiscs[i].mpartitions[j].status == '1') {
                cout << "-- 38" << i + 1 << mountedDiscs[i].mpartitions[j].diskName << ", " << mountedDiscs[i].mpartitions[j].name << endl;
            }
        }
    }
    cout << endl;
}