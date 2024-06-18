#include "../libraries/fdisk.h"
#include "../libraries/structs.h"
#include "../libraries/scanner.h"

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

using namespace std;

//scanner scan;

FDisk::FDisk(){
}

void FDisk::fdisk_create(vector<string> context){
    vector<string> required = {"s", "path", "name"};
    string size;
    string unit = "k";
    string path;
    string type = "P";
    string fit = "WF";
    string name;
    string add;

    for(auto current: context){
        string id = current.substr(0, current.find("="));
        current.erase(0, id.length() + 1);
        if(current.substr(0, 1) == "\""){
            current = current.substr(1, current.length() - 2);
        }

        if (scan.compare(id, "s"))
        {
            if(count(required.begin(), required.end(), id)){
                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                size = current;
            }
        }else if (scan.compare(id, "u")){
            unit = current;
        }else if(scan.compare(id, "path")){
            if(count(required.begin(), required.end(), id)){
                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                path = current;
            }
        }else if (scan.compare(id, "t")){
            type = current;
        }else if (scan.compare(id, "f")){
            fit = current;
        }else if (scan.compare(id, "name")){
            if(count(required.begin(), required.end(), id)){
                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                name = current;
            }
        }else if (scan.compare(id, "add")){
            add = current;
            if (count(required.begin(), required.end(), "s")) {
                auto itr = find(required.begin(), required.end(), "s");
                required.erase(itr);
                size = current;
            }
        }else{
            scan.errores("FDISK","No se reconoce el parametro "+id);
        }
    }

    if(!required.empty()){
        scan.errores("FDISK","Faltan parametros obligatorios para completar la acción");
        return;
    }else{
        
        cout << "Generar particion" << endl;
        cout << "Size: " << size << endl;
        cout << "U: " << unit << endl;
        cout << "Path: " << path << endl;
        cout << "Type: " << type << endl;
        cout << "F: " << fit << endl;
        cout << "Name: " << name << endl;

        if(add.empty()){
            generatePartition(size, unit, path, type, fit, name, add);
        }else{            
            cout << "Add: " << add << endl;
            //addPartition(path, name, add);
        }
        
    }
}


void FDisk::fdisk_delete(vector<string> context){
    vector<string> required = {"path", "name", "delete"};
    string _delete;
    string path;
    string name;

    for (auto current : context) {
        string id = scan.lowerString(current.substr(0, current.find('=')));
        current.erase(0, id.length() + 1);
        if (current.substr(0, 1) == "\"") {
            current = current.substr(1, current.length() - 2);
        }

        if (scan.compare(id, "path")) {
            if (count(required.begin(), required.end(), id)) {
                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                path = current;
            }
        } else if (scan.compare(id, "name")) {
            if (count(required.begin(), required.end(), id)) {

                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                name = current;
            }
        } else if (scan.compare(id, "delete")) {
            if (count(required.begin(), required.end(), id)) {

                auto itr = find(required.begin(), required.end(), id);
                required.erase(itr);
                _delete = current;
            }
        }
    }
    if(!required.empty()){
        scan.errores("FDISK","Faltan parametros obligatorios para completar la acción");
        return;
    }else{
        //deletePartition(path, name, _delete);
    }

}

void FDisk::generatePartition(string size, string unit, string path, string type, string fit, string n, string a){
    try{
        startValue = 0;
        int i = stoi(size);
        if(i <= 0){
            scan.handler("FDISK", "Size debe ser mayor a 0");
            return;
        }
        if(scan.compare(unit, "b") || scan.compare(unit, "k") || scan.compare(unit, "m")){
            if(!scan.compare(unit, "b")){
                i *= (scan.compare(unit, "k") ? 1024 : 1024 * 1024);
            }
        }else{
            scan.handler("FDISK", "U debe ser b, k o m");
            return;
        }
        if(path.substr(0,1) == "\""){
            path = path.substr(1,path.length()-2);
        }
        if(!(scan.compare(type, "p") || scan.compare(type, "e") || scan.compare(type, "l"))){
            scan.handler("FDISK", "El tipo debe ser p, e o l");
            return;
        }
        if(!(scan.compare(fit, "bf") || scan.compare(fit, "ff") || scan.compare(fit, "wf"))){
            scan.handler("FDISK", "El fit debe ser bf, ff o wf");
            return;
        }
        Structs::MBR disco;
        FILE *file = fopen(path.c_str(), "rb+");
        if(file == NULL){
            scan.handler("FDISK", "El disco no existe");
            return;
        }else{
            rewind(file);
            fread(&disco, sizeof(Structs::MBR), 1, file);
        }
        fclose(file);

        vector<Structs::Partition> particiones = getPartitions(disco); // vector de particiones, a un inicio vacio
        vector<Transition> between;

        int used = 0;
        int ext = 0;
        int c = 1;
        int base = sizeof(disco);
        Structs::Partition extended;
        for(Structs::Partition p : particiones){
            if(p.part_status == '1'){
                Transition trn;
                trn.partition = c;
                trn.start = p.part_start;
                trn.end = p.part_start + p.part_size;

                trn.before = trn.start - base;
                base = trn.end;

                if(used != 0){
                    between.at(used-1).after = trn.start - between.at(used-1).end; 
                }
                between.push_back(trn);
                used++;

                if(p.part_type == 'e' || p.part_type == 'E'){
                    ext++;
                    extended = p;
                }
            }
            if(used == 4 && (scan.compare(type, "p"))){
                scan.handler("FDISK", "No se pueden crear mas particiones primarias");
                return;
            }else if(ext==1 && (scan.compare(type, "e"))){
                scan.handler("FDISK", "No se pueden crear mas particiones extendidas");
                return;
            }
            c++;
        }
        if(ext == 0 && scan.compare(type, "l")){
            scan.handler("FDISK", "No se puede crear una particion logica sin una extendida");
            return;
        }
        if(used != 0){
            between.at(between.size()-1).after = disco.mbr_size - between.at(between.size()-1).end;
        }

        try{
            findPartitionby(disco, n,path);
            scan.handler("FDISK", "Ya existe una particion con ese nombre");
            return;
        }catch(exception &e){}

        // Aqui se crea la particion que el usuario este mandando
        Structs::Partition newPartition;
        newPartition.part_status = '1';
        newPartition.part_size = i; 
        newPartition.part_type = toupper(type[0]); // P, E, L
        newPartition.part_fit = toupper(fit[0]); // B, F, W
        strcpy(newPartition.part_name, n.c_str()); // Nombre de la particion

        if(scan.compare(type, "l")){
            // Aqui se crea la particion logica
        }

        disco = adjust(disco, newPartition, between, particiones, used);

        FILE *bfile = fopen(path.c_str(), "rb+");
        if(bfile != NULL){
            fseek(bfile, 0, SEEK_SET);
            fwrite(&disco, sizeof(Structs::MBR), 1, bfile);
            if(scan.compare(type,"e")){
                Structs::EBR ebr;
                ebr.part_start = startValue;
                fseek(bfile, startValue, SEEK_SET);
                fwrite(&ebr, sizeof(Structs::EBR), 1, bfile);
            }
            fclose(bfile);
            scan.respuesta("FDISK", "Particion creada correctamente");
        }

    }catch (invalid_argument &e) {
        scan.handler("FDISK", "-s debe ser un entero");
        return;
    }
    catch (exception &e) {
        scan.handler("FDISK", e.what());
        return;
    }
}

Structs::MBR FDisk::adjust(Structs::MBR mbr, Structs::Partition path, vector<Transition> type, vector<Structs::Partition> ps, int u){
    if (u == 0) {
        path.part_start = sizeof(mbr);
        startValue = path.part_start;
        mbr.mbr_Partition_1 = path;
        return mbr;
    } else {
        Transition toUse;
        int c = 0;
        for (Transition tr : type) {
            if (c == 0) {
                toUse = tr;
                c++;
                continue;
            }

            if (toupper(mbr.disk_fit) == 'F') {
                if (toUse.before >= path.part_size || toUse.after >= path.part_size) {
                    break;
                }
                toUse = tr;
            } else if (toupper(mbr.disk_fit) == 'B') {
                if (toUse.before < path.part_size || toUse.after < path.part_size) {
                    toUse = tr;
                } else {
                    if (tr.before >= path.part_size || tr.after >= path.part_size) {
                        int b1 = toUse.before - path.part_size;
                        int a1 = toUse.after - path.part_size;
                        int b2 = tr.before - path.part_size;
                        int a2 = tr.after - path.part_size;

                        if ((b1 < b2 && b1 < a2) || (a1 < b2 && a1 < a2)) {
                            c++;
                            continue;
                        }
                        toUse = tr;
                    }
                }
            } else if (toupper(mbr.disk_fit) == 'W') {
                if (!(toUse.before >= path.part_size) || !(toUse.after >= path.part_size)) {
                    toUse = tr;
                } else {
                    if (tr.before >= path.part_size || tr.after >= path.part_size) {
                        int b1 = toUse.before - path.part_size;
                        int a1 = toUse.after - path.part_size;
                        int b2 = tr.before - path.part_size;
                        int a2 = tr.after - path.part_size;

                        if ((b1 > b2 && b1 > a2) || (a1 > b2 && a1 > a2)) {
                            c++;
                            continue;
                        }
                        toUse = tr;
                    }
                }
            }
            c++;
        }
        if (toUse.before >= path.part_size || toUse.after >= path.part_size) {
            if (toupper(mbr.disk_fit) == 'F') {
                if (toUse.before >= path.part_size) {
                    path.part_start = (toUse.start - toUse.before);
                    startValue = path.part_start;
                } else {
                    path.part_start = toUse.end;
                    startValue = path.part_start;
                }
            } else if (toupper(mbr.disk_fit) == 'B') {
                int b1 = toUse.before - path.part_size;
                int a1 = toUse.after - path.part_size;

                if ((toUse.before >= path.part_size && b1 < a1) || !(toUse.after >= path.part_start)) {
                    path.part_start = (toUse.start - toUse.before);
                    startValue = path.part_start;
                } else {
                    path.part_start = toUse.end;
                    startValue = path.part_start;
                }
            } else if (toupper(mbr.disk_fit) == 'W') {
                int b1 = toUse.before - path.part_size;
                int a1 = toUse.after - path.part_size;

                if ((toUse.before >= path.part_size && b1 > a1) || !(toUse.after >= path.part_start)) {
                    path.part_start = (toUse.start - toUse.before);
                    startValue = path.part_start;
                } else {
                    path.part_start = toUse.end;
                    startValue = path.part_start;
                }
            }
            Structs::Partition partitions[4];
            for (int i = 0; i < ps.size(); i++) {
                partitions[i] = ps.at(i);
            }
            for (auto &partition : partitions) {
                if (partition.part_status == '0') {
                    partition = path;
                    break;
                }
            }

            Structs::Partition aux;
            for (int i = 3; i >= 0; i--) {
                for (int j = 0; j < i; j++) {
                    if ((partitions[j].part_start > partitions[j + 1].part_start)) {
                        aux = partitions[j + 1];
                        partitions[j + 1] = partitions[j];
                        partitions[j] = aux;
                    }
                }
            }

            for (int i = 3; i >= 0; i--) {
                for (int j = 0; j < i; j++) {
                    if (partitions[j].part_status == '0') {
                        aux = partitions[j];
                        partitions[j] = partitions[j + 1];
                        partitions[j + 1] = aux;
                    }
                }
            }
            mbr.mbr_Partition_1 = partitions[0];
            mbr.mbr_Partition_2 = partitions[1];
            mbr.mbr_Partition_3 = partitions[2];
            mbr.mbr_Partition_4 = partitions[3];
            return mbr;
        } else {
            throw runtime_error("No hay espacio suficiente para esta particion");
        }
    }
}

Structs::Partition FDisk::findPartitionby(Structs::MBR mbr, string name, string path) {
    Structs::Partition partitions[4];
    partitions[0] = mbr.mbr_Partition_1;
    partitions[1] = mbr.mbr_Partition_2;
    partitions[2] = mbr.mbr_Partition_3;
    partitions[3] = mbr.mbr_Partition_4;

    bool ext = false;
    Structs::Partition extended;
    for (auto &partition : partitions) {
        if (partition.part_status == '1') {
            //cout << "Comparando nombre: "<< partition.part_name << " con " << name << endl;
            if (scan.compare(partition.part_name, name)) {
                return partition;
            } else if (partition.part_type == 'E') {
                ext = true;
                extended = partition;
            }
        }
    }
    if (ext) {
        //cout << "Buscando particion logica" << endl;
        vector<Structs::EBR> ebrs = getlogicPartitions(extended, path);
        for (Structs::EBR ebr : ebrs) {
            if (ebr.part_status == '1') {
                cout << "Encontramos una particion logica" << endl;
                if (scan.compare(ebr.part_name, name)) {
                    Structs::Partition tmp;
                    tmp.part_status = '1';
                    tmp.part_type = 'L';
                    tmp.part_fit = ebr.part_fit;
                    tmp.part_start = ebr.part_start;
                    tmp.part_size = ebr.part_size;
                    strcpy(tmp.part_name, ebr.part_name);
                    return tmp;
                }
            }
        }
        //cout << "No se encontro" << endl;
    }
    throw runtime_error("la partición no existe");
}


vector<Structs::EBR> FDisk::getlogicPartitions(Structs::Partition partition, string path) {
    vector<Structs::EBR> ebrs;

    FILE *file = fopen(path.c_str(), "rb+");
    rewind(file);
    Structs::EBR tmp;
    fseek(file, partition.part_start, SEEK_SET);
    fread(&tmp, sizeof(Structs::EBR), 1, file);
    bool recoordio = true;
    do {
        if (!(tmp.part_status == '0' && tmp.part_next == -1)) {
            if (tmp.part_status != '0') {
                ebrs.push_back(tmp);
            }
            fseek(file, tmp.part_next, SEEK_SET);
            fread(&tmp, sizeof(Structs::EBR), 1, file);
        } else {
            fclose(file);
            recoordio = false;
        }
    } while (recoordio);
    return ebrs;
}

vector<Structs::Partition> FDisk::getPartitions(Structs::MBR mbr){
    vector<Structs::Partition> partitions;
    partitions.push_back(mbr.mbr_Partition_1);
    partitions.push_back(mbr.mbr_Partition_2);
    partitions.push_back(mbr.mbr_Partition_3);
    partitions.push_back(mbr.mbr_Partition_4);
    return partitions;
}

void FDisk:: deletePartition(string d, string path, string name){
    try {

        if (path.substr(0, 1) == "\"") {
            path = path.substr(1, path.length() - 2);
        }

        if (!(scan.compare(d, "fast") || scan.compare(d, "full"))) {
            throw runtime_error("-delete necesita valores específicos");
        }

        FILE *file = fopen(path.c_str(), "rb+");
        if (file == NULL) {
            throw runtime_error("disco no existente");
        }

        Structs::MBR disk;
        rewind(file);
        fread(&disk, sizeof(Structs::MBR), 1, file);

        findPartitionby(disk, name, path);

        Structs::Partition partitions[4];
        partitions[0] = disk.mbr_Partition_1;
        partitions[1] = disk.mbr_Partition_2;
        partitions[2] = disk.mbr_Partition_3;
        partitions[3] = disk.mbr_Partition_4;

        Structs::Partition past;
        bool fll = false;
        for (int i = 0; i < 4; i++) {
            if (partitions[i].part_status == '1') {
                if (partitions[i].part_type == 'P') {
                    if (scan.compare(partitions[i].part_name, name)) {
                        if (scan.compare(d, "fast")) {
                            partitions[i].part_status = '0';
                        } else {
                            past = partitions[i];
                            partitions[i] = Structs::Partition();
                            fll = true;
                        }
                        break;
                    }
                } else {
                    if (scan.compare(partitions[i].part_name, name)) {
                        if (scan.compare(d, "fast")) {
                            partitions[i].part_status = '0';
                        } else {
                            past = partitions[i];
                            partitions[i] = Structs::Partition();
                            fll = true;
                        }
                        break;
                    }
                    vector<Structs::EBR> ebrs = getlogicPartitions(partitions[i], path);

                    int count = 0;
                    for (Structs::EBR ebr : ebrs) {
                        if (scan.compare(ebr.part_name, name)) {
                            ebr.part_status = '0';
                        }
                        fseek(file, ebr.part_start, SEEK_SET);
                        fwrite(&ebr, sizeof(Structs::EBR), 1, file);
                        count++;
                    }
                    scan.respuesta("FDISK", "partición eliminada correctamente -" + d);
                    return;
                }
            }
        }

        Structs::Partition aux;
        for (int i = 3; i >= 0; i--) {
            for (int j = 0; j < i; j++) {
                if (partitions[j].part_status == '0') {
                    aux = partitions[j];
                    partitions[j] = partitions[j + 1];
                    partitions[j + 1] = aux;
                }
            }
        }

        disk.mbr_Partition_1 = partitions[0];
        disk.mbr_Partition_2 = partitions[1];
        disk.mbr_Partition_3 = partitions[2];
        disk.mbr_Partition_4 = partitions[3];

        rewind(file);
        fwrite(&disk, sizeof(Structs::MBR), 1, file);
        if (fll) {
            fseek(file, past.part_start, SEEK_SET);
            int num = static_cast<int>(past.part_size / 2);
            fwrite("\0", sizeof("\0"), num, file);
        }
        scan.respuesta("FDISK", "partición eliminada correctamente -" + d);
        fclose(file);
    }
    catch (exception &e) {
        scan.handler("FDISK", e.what());
        return;
    }
}

void FDisk::addPartition(string add, string u, string n, string p) {
    try {
        int i = stoi(add);

        if (scan.compare(u, "b") || scan.compare(u, "k") || scan.compare(u, "m")) {

            if (!scan.compare(u, "b")) {
                i *= (scan.compare(u, "k")) ? 1024 : 1024 * 1024;
            }
        } else {
            throw runtime_error("-u necesita valores específicos");
        }


        FILE *file = fopen(p.c_str(), "rb+");
        if (file == NULL) {
            throw runtime_error("disco no existente");
        }

        Structs::MBR disk;
        rewind(file);
        fread(&disk, sizeof(Structs::MBR), 1, file);

        findPartitionby(disk, n, p);

        Structs::Partition partitions[4];
        partitions[0] = disk.mbr_Partition_1;
        partitions[1] = disk.mbr_Partition_2;
        partitions[2] = disk.mbr_Partition_3;
        partitions[3] = disk.mbr_Partition_4;


        for (int i = 0; i < 4; i++) {
            if (partitions[i].part_status == '1') {
                if (scan.compare(partitions[i].part_name, n)) {
                    if ((partitions[i].part_size + (i)) > 0) {
                        if (i != 3) {
                            if (partitions[i + 1].part_start != 0) {
                                if (((partitions[i].part_size + (i) +
                                      partitions[i].part_start) <=
                                     partitions[i + 1].part_start)) {
                                    partitions[i].part_size += i;
                                    break;
                                } else {
                                    throw runtime_error("se sobrepasa el límite");
                                }
                            }
                        }
                        if ((partitions[i].part_size + i +
                             partitions[i].part_start) <= disk.mbr_size) {
                            partitions[i].part_size += i;
                            break;
                        } else {
                            throw runtime_error("se sobrepasa el límite");
                        }

                    }
                }
            }
        }

        disk.mbr_Partition_1 = partitions[0];
        disk.mbr_Partition_2 = partitions[1];
        disk.mbr_Partition_3 = partitions[2];
        disk.mbr_Partition_4 = partitions[3];

        rewind(file);
        fwrite(&disk, sizeof(Structs::MBR), 1, file);
        scan.respuesta("FDISK", "la partición se ha aumentado correctamente");
        fclose(file);
    }
    catch (exception &e) {
        scan.handler("FDISK", e.what());
        return;
    }

}