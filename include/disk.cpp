#include "../libraries/disk.h"
#include "../libraries/structs.h"
#include "../libraries/scanner.h"

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>

using namespace std;

scanner scan;
int startValue;

Disk::Disk(){
}

void Disk::mkdisk(vector<string> tokens){
    string size = "";
    string unit = "";
    string path = "";
    string fit = "";
    bool error = false;
    for(string token:tokens){
        string tk = token.substr(0, token.find("=")); // -f=b
        token.erase(0,tk.length()+1); // b
        if(scan.compare(tk, "f")){
            if(fit.empty()){
                fit = token; // f = b
            }else{
                scan.errores("MKDISK", "El parametro F ya fue ingresado en el comando"+tk);
            }
        }else if(scan.compare(tk, "s")){
            if (size.empty())
            {
                size = token;
            }else{
                scan.errores("MKDISK","parametro SIZE repetido en el comando"+tk);
            }
        }else if (scan.compare(tk, "u"))
        {
            if (unit.empty())
            {
                unit = token;
            }else{
                scan.errores("MKDISK","parametro U repetido en el comando"+tk);
            }
        }else if (scan.compare(tk, "path"))
        {
            if (path.empty())
            {
                if(token.substr(0,1) == "\""){
                    token = token.substr(1,token.length()-2);
                }
                path = token;
            }else{
                scan.errores("MKDISK","parametro PATH repetido en el comando"+tk);
            }    
        }else{
            scan.errores("MKDISK","no se esperaba el parametro "+tk);
            error = true;
            break;
        }
    }
    if (error){
        return;
    }

    if (fit.empty())
    {
        fit = "BF";
    }
    if (unit.empty())
    {
        unit = "M";
    }

    if (path.empty() && size.empty())
    {
        scan.errores("MKDISK", "se requiere parametro Path y Size para este comando");
    }else if(path.empty()){
        scan.errores("MKDISK","se requiere parametro Path para este comando");
    }else if (size.empty())
    {
        scan.errores("MKDISK","se requiere parametro Size para este comando");
    }else if (!scan.compare(fit,"BF") && !scan.compare(fit,"FF") && !scan.compare(fit,"WF"))
    {
        scan.errores("MKDISK","valores de parametro F no esperados");
    }else if (!scan.compare(unit,"k") && !scan.compare(unit,"m"))
    {
        scan.errores("MKDISK","valores de parametro U no esperados");
    }else{
        makeDisk(size,fit,unit,path);
    }  
}

// Crear funcion makeDisk
void Disk::makeDisk(string stringSize, string fit, string unit, string path){\
    Structs::MBR disco; 
    try{
        int size = stoi(stringSize); // stoi = string to int
        if (size <=0){
            scan.errores("MKDISK","Size debe ser mayor a 0");
        }
        if(scan.compare(unit,"M")){
            size = size * 1024 * 1024;
        }
        if(scan.compare(unit,"K")){
            size = size * 1024;
        }
        fit = fit.substr(0,1); // BF -> B
        disco.mbr_size = size;
        disco.mbr_date_creacion = time(nullptr);
        disco.disk_fit = toupper(fit[0]);
        disco.mbr_disk_signature = rand() % 9999 + 100;

        FILE *file = fopen(path.c_str(),"r"); // c_str() = convertir string a char
        if(file != NULL){
            scan.errores("MKDISK","El disco ya existe");
            fclose(file);
            return;
        }

        disco.mbr_Partition_1 = Structs::Partition();
        disco.mbr_Partition_2 = Structs::Partition();
        disco.mbr_Partition_3 = Structs::Partition();
        disco.mbr_Partition_4 = Structs::Partition();

        string path2 = path;
        if(path.substr(0,1) == "\""){
            path2 = path.substr(1,path.length()-2);
        };

        if(!scan.compare(path.substr(path.find_last_of(".") + 1), "dsk")){
            scan.errores("MKDISK","El disco debe ser de tipo .dsk");
            return;
        }

        try{
            FILE *file = fopen(path.c_str(), "w+b");
            if(file!=NULL){
                fwrite("\0", 1, 1, file);
                fseek(file, size-1, SEEK_SET);
                fwrite("\0", 1, 1, file);
                rewind(file);
                fwrite(&disco, sizeof(Structs::MBR), 1, file);
                fclose(file);
            }else{
                string comando1 = "mkdir -p \""+ path + "\"";
                string comando2 = "rmdir \""+ path + "\"";
                system(comando1.c_str());
                system(comando2.c_str());
                FILE *file = fopen(path.c_str(), "w+b");
                fwrite("\0",1,1,file);
                fseek(file, size - 1, SEEK_SET);
                fwrite("\0", 1, 1, file);
                rewind(file);
                fwrite(&disco, sizeof(Structs::MBR),1, file);
                fclose(file);
            }

            FILE *imprimir = fopen(path.c_str(), "r");
            if(imprimir!=NULL){
                Structs::MBR discoI;
                fseek(imprimir, 0, SEEK_SET);
                fread(&discoI,sizeof(Structs::MBR), 1,imprimir);
                struct tm *tm;
                tm = localtime(&discoI.mbr_date_creacion);
                char mostrar_fecha [20];
                strftime(mostrar_fecha, 20, "%Y/%m/%d %H:%M:%S", tm);                
                scan.respuesta("MKDISK","   Disco creado exitosamente");
                std::cout << "********Nuevo Disco********" << std::endl;
                std::cout << "Size:  "<< discoI.mbr_size << std::endl;
                std::cout << "Fecha:  "<< mostrar_fecha << std::endl;
                std::cout << "Fit:  "<< discoI.disk_fit << std::endl;
                std::cout << "Disk_Signature:  "<< discoI.mbr_disk_signature << std::endl;
                cout << "Bits del MBR:  " << sizeof(Structs::MBR) << endl;
                std::cout << "Path:  "<< path2 << std::endl;
            }
            fclose(imprimir);

        }catch(const exception& e){
            scan.errores("MKDISK","Error al crear el disco");
        }
    }catch(invalid_argument &e){
        scan.errores("MKDISK","Size debe ser un entero");
    }

}

void Disk::rmdisk(vector<string> context){
    string path = "";
    bool error = false;
    for(string token:context){
        string tk = token.substr(0, token.find("="));
        token.erase(0,tk.length()+1);
        if(scan.compare(tk, "path")){
            if(path.empty()){
                if(token.substr(0,1) == "\""){
                    token = token.substr(1,token.length()-2);
                }
                path = token;
            }else{
                scan.errores("RMDISK","El parametro PATH ya fue ingresado en el comando"+tk);
            }
        }else{
            scan.errores("RMDISK","no se esperaba el parametro "+tk);
            error = true;
            break;
        }
    }
    if (error){
        return;
    }
    if (path.empty()){
        scan.errores("RMDISK","se requiere parametro PATH para este comando");
    }else{
        FILE *file = fopen(path.c_str(), "r");
        if(file == NULL){
            scan.errores("RMDISK","El disco no existe");
            return;
        }
        fclose(file);
        if(scan.confirm("¿Desea eliminar el disco?")){
            string comando = "rm \"" + path + "\"";
            system(comando.c_str());
            scan.respuesta("RMDISK","Disco eliminado exitosamente");
        }else{
            scan.respuesta("RMDISK","Operacion cancelada");
        }
    }
}


void Disk::fdisk(vector<string> context){
    
    bool deletePartition = false;
    bool addPartition = false;
    //fdisk -s=300 -path=/home/Disco1.dsk -name=Particion1
    //fdisk -delete=fast -path=/home/Disco1.dsk -name=Particion1
    // [s=300, path=/home/Disco1.dsk, name=Particion1]
    // [delete=fast, path=/home/Disco1.dsk, name=Particion1]
    for (string current: context){
        string id = current.substr(0, current.find("="));
        current.erase(0, id.length() + 1);
        if(current.substr(0, 1) == "\""){
            current = current.substr(1, current.length() - 2);
        }
        if(scan.compare(id, "delete")){
            deletePartition = true;
        }else if(scan.compare(id, "add")){
            addPartition = true;
        }
    }

    if(deletePartition){        
        fdisk_delete(context);
    }else if(addPartition){
        fdisk_create(context);
    }else{
        fdisk_create(context);
    }
}


void Disk::fdisk_create(vector<string> context){
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
        }else if (scan.compare(id, "type")){
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
        cout << "Add: " << add << endl;
    }
}

void Disk::fdisk_delete(vector<string> context){
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
}

void Disk::generatepartition(string s, string u, string p, string t, string f, string n, string a){
    try{
        startValue = 0;
        int i = stoi(s);
        if(i <= 0){
            scan.handler("FDISK", "Size debe ser mayor a 0");
            return;
        }
        if(scan.compare(u, "b") || scan.compare(u, "k") || scan.compare(u, "m")){
            if(!scan.compare(u, "b")){
                i *= (scan.compare(u, "k") ? 1024 : 1024 * 1024);
            }
        }else{
            scan.handler("FDISK", "U debe ser b, k o m");
            return;
        }
        if(p.substr(0,1) == "\""){
            p = p.substr(1,p.length()-2);
        }
        if(!(scan.compare(t, "p") || scan.compare(t, "e") || scan.compare(t, "l"))){
            scan.handler("FDISK", "El tipo debe ser p, e o l");
            return;
        }
        if(!(scan.compare(f, "bf") || scan.compare(f, "ff") || scan.compare(f, "wf"))){
            scan.handler("FDISK", "El fit debe ser bf, ff o wf");
            return;
        }
        Structs::MBR disco;
        FILE *file = fopen(p.c_str(), "rb+");
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
            if(used == 4 && !(scan.compare(t, "l"))){
                scan.handler("FDISK", "No se pueden crear mas particiones primarias");
                return;
            }else if(ext==1 && !(scan.compare(t, "e"))){
                scan.handler("FDISK", "No se pueden crear mas particiones extendidas");
                return;
            }
            c++;
        }
        if(ext == 0 && scan.compare(t, "l")){
            scan.handler("FDISK", "No se puede crear una particion logica sin una extendida");
            return;
        }
        if(used != 0){
            between.at(between.size()-1).after = disco.mbr_size - between.at(between.size()-1).end;
        }

        try{
            findby(disco, n,p);
            scan.handler("FDISK", "Ya existe una particion con ese nombre");
            return;
        }catch(exception &e){}

        // Aqui se crea la particion que el usuario este mandando
        Structs::Partition newPartition;
        newPartition.part_status = '1';
        newPartition.part_size = i; 
        newPartition.part_type = toupper(t[0]); // P, E, L
        newPartition.part_fit = toupper(f[0]); // B, F, W
        strcpy(newPartition.part_name, n.c_str()); // Nombre de la particion

        if(scan.compare(t, "l")){
            // Aqui se crea la particion logica
        }

        disco = adjust(disco, newPartition, between, particiones, used);

        FILE *bfile = fopen(p.c_str(), "rb+");
        if(bfile != NULL){
            fseek(bfile, 0, SEEK_SET);
            fwrite(&disco, sizeof(Structs::MBR), 1, bfile);
            if(scan.compare(t,"e")){
                Structs::EBR ebr;
                ebr.part_start = startValue;
                fseek(bfile, startValue, SEEK_SET);
                fwrite(&ebr, sizeof(Structs::EBR), 1, bfile);
            }
            fclose(bfile);
            scan.handler("FDISK", "Particion creada correctamente");
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

vector<Structs::Partition> Disk::getPartitions(Structs::MBR mbr){
    vector<Structs::Partition> partitions;
    partitions.push_back(mbr.mbr_Partition_1);
    partitions.push_back(mbr.mbr_Partition_2);
    partitions.push_back(mbr.mbr_Partition_3);
    partitions.push_back(mbr.mbr_Partition_4);
    return partitions;
}

Structs::MBR

Disk::adjust(Structs::MBR mbr, Structs::Partition p, vector<Transition> t, vector<Structs::Partition> ps, int u){
    if (u == 0) {
        p.part_start = sizeof(mbr);
        startValue = p.part_start;
        mbr.mbr_Partition_1 = p;
        return mbr;
    } else {
        Transition toUse;
        int c = 0;
        for (Transition tr : t) {
            if (c == 0) {
                toUse = tr;
                c++;
                continue;
            }

            if (toupper(mbr.disk_fit) == 'F') {
                if (toUse.before >= p.part_size || toUse.after >= p.part_size) {
                    break;
                }
                toUse = tr;
            } else if (toupper(mbr.disk_fit) == 'B') {
                if (toUse.before < p.part_size || toUse.after < p.part_size) {
                    toUse = tr;
                } else {
                    if (tr.before >= p.part_size || tr.after >= p.part_size) {
                        int b1 = toUse.before - p.part_size;
                        int a1 = toUse.after - p.part_size;
                        int b2 = tr.before - p.part_size;
                        int a2 = tr.after - p.part_size;

                        if ((b1 < b2 && b1 < a2) || (a1 < b2 && a1 < a2)) {
                            c++;
                            continue;
                        }
                        toUse = tr;
                    }
                }
            } else if (toupper(mbr.disk_fit) == 'W') {
                if (!(toUse.before >= p.part_size) || !(toUse.after >= p.part_size)) {
                    toUse = tr;
                } else {
                    if (tr.before >= p.part_size || tr.after >= p.part_size) {
                        int b1 = toUse.before - p.part_size;
                        int a1 = toUse.after - p.part_size;
                        int b2 = tr.before - p.part_size;
                        int a2 = tr.after - p.part_size;

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
        if (toUse.before >= p.part_size || toUse.after >= p.part_size) {
            if (toupper(mbr.disk_fit) == 'F') {
                if (toUse.before >= p.part_size) {
                    p.part_start = (toUse.start - toUse.before);
                    startValue = p.part_start;
                } else {
                    p.part_start = toUse.end;
                    startValue = p.part_start;
                }
            } else if (toupper(mbr.disk_fit) == 'B') {
                int b1 = toUse.before - p.part_size;
                int a1 = toUse.after - p.part_size;

                if ((toUse.before >= p.part_size && b1 < a1) || !(toUse.after >= p.part_start)) {
                    p.part_start = (toUse.start - toUse.before);
                    startValue = p.part_start;
                } else {
                    p.part_start = toUse.end;
                    startValue = p.part_start;
                }
            } else if (toupper(mbr.disk_fit) == 'W') {
                int b1 = toUse.before - p.part_size;
                int a1 = toUse.after - p.part_size;

                if ((toUse.before >= p.part_size && b1 > a1) || !(toUse.after >= p.part_start)) {
                    p.part_start = (toUse.start - toUse.before);
                    startValue = p.part_start;
                } else {
                    p.part_start = toUse.end;
                    startValue = p.part_start;
                }
            }
            Structs::Partition partitions[4];
            for (int i = 0; i < ps.size(); i++) {
                partitions[i] = ps.at(i);
            }
            for (auto &partition : partitions) {
                if (partition.part_status == '0') {
                    partition = p;
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

Structs::Partition Disk::findby(Structs::MBR mbr, string name, string path) {
    Structs::Partition partitions[4];
    partitions[0] = mbr.mbr_Partition_1;
    partitions[1] = mbr.mbr_Partition_2;
    partitions[2] = mbr.mbr_Partition_3;
    partitions[3] = mbr.mbr_Partition_4;

    bool ext = false;
    Structs::Partition extended;
    for (auto &partition : partitions) {
        if (partition.part_status == '1') {
            if (scan.compare(partition.part_name, name)) {
                return partition;
            } else if (partition.part_type == 'E') {
                ext = true;
                extended = partition;
            }
        }
    }
    if (ext) {
        vector<Structs::EBR> ebrs = getlogics(extended, path);
        for (Structs::EBR ebr : ebrs) {
            if (ebr.part_status == '1') {
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
    }
    throw runtime_error("la partición no existe");
}


vector<Structs::EBR> Disk::getlogics(Structs::Partition partition, string p) {
    vector<Structs::EBR> ebrs;

    FILE *file = fopen(p.c_str(), "rb+");
    rewind(file);
    Structs::EBR tmp;
    fseek(file, partition.part_start, SEEK_SET);
    fread(&tmp, sizeof(Structs::EBR), 1, file);
    do {
        if (!(tmp.part_status == '0' && tmp.part_next == -1)) {
            if (tmp.part_status != '0') {
                ebrs.push_back(tmp);
            }
            fseek(file, tmp.part_next, SEEK_SET);
            fread(&tmp, sizeof(Structs::EBR), 1, file);
        } else {
            fclose(file);
            break;
        }
    } while (true);
    return ebrs;
}
