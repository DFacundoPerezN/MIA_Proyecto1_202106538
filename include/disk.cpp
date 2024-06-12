#include "../libraries/disk.h"
#include "../libraries/structs.h"
#include "../libraries/scanner.h"

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <algorithm>

using namespace std;

scanner scan;
int startValue;

Disk::Disk(){
}

void Disk::mkdisk(vector<string> tokens){
    string size = "";
    string u = "";
    string path = "";
    string f = "";
    bool error = false;
    for(string token:tokens){
        string tk = token.substr(0, token.find("=")); // -f=b
        token.erase(0,tk.length()+1); // b
        if(scan.compare(tk, "f")){
            if(f.empty()){
                f = token; // f = b
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
            if (u.empty())
            {
                u = token;
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

    if (f.empty())
    {
        f = "BF";
    }
    if (u.empty())
    {
        u = "M";
    }

    if (path.empty() && size.empty())
    {
        scan.errores("MKDISK", "se requiere parametro Path y Size para este comando");
    }else if(path.empty()){
        scan.errores("MKDISK","se requiere parametro Path para este comando");
    }else if (size.empty())
    {
        scan.errores("MKDISK","se requiere parametro Size para este comando");
    }else if (!scan.compare(f,"BF") && !scan.compare(f,"FF") && !scan.compare(f,"WF"))
    {
        scan.errores("MKDISK","valores de parametro F no esperados");
    }else if (!scan.compare(u,"k") && !scan.compare(u,"m"))
    {
        scan.errores("MKDISK","valores de parametro U no esperados");
    }else{
        makeDisk(size,f,u,path);
    }  
}


// Crear funcion makeDisk
void Disk::makeDisk(string s, string f, string u, string path){\
    Structs::MBR disco; 
    try{
        int size = stoi(s); // stoi = string to int
        if (size <=0){
            scan.errores("MKDISK","Size debe ser mayor a 0");
        }
        if(scan.compare(u,"M")){
            size = size * 1024 * 1024;
        }
        if(scan.compare(u,"K")){
            size = size * 1024;
        }
        f = f.substr(0,1); // BF -> B
        disco.mbr_tamano = size;
        disco.mbr_fecha_creacion = time(nullptr);
        disco.disk_fit = toupper(f[0]);
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
                tm = localtime(&discoI.mbr_fecha_creacion);
                char mostrar_fecha [20];
                strftime(mostrar_fecha, 20, "%Y/%m/%d %H:%M:%S", tm);                
                scan.respuesta("MKDISK","   Disco creado exitosamente");
                std::cout << "********Nuevo Disco********" << std::endl;
                std::cout << "Size:  "<< discoI.mbr_tamano << std::endl;
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
        string comando = "rm \"" + path + "\"";
        system(comando.c_str());
        scan.respuesta("RMDISK","Disco eliminado exitosamente");
    }
}

void Disk::fdisk(vector<string> context){
    bool deletePartition = false;
    bool addPartition = false;
    for (string current:context)
    {
        string id = current.substr(0, current.find("="));
        current.erase(0, id.length()+1);
        if(current.substr(0,1) == "\""){
            current = current.substr(1,current.length()-2);
        }   
        if(scan.compare(id, "delete")){
            deletePartition = true;
    
        }else if(scan.compare(id, "add")){
            addPartition = true;
        }   
    }
}

void Disk::fdisk_create(vector<string> context){
    string size = "";  
    string unit = "k";
    string path;
    string type = "P";
    string fit = "FF";
    string name;
    string add;
}