#include "../libraries/scanner.h"
#include "../libraries/disk.h"
#include "../libraries/rep.h"
#include "../libraries/mount.h"


#include <iostream>
#include <locale>
#include <stdlib.h>
#include <cstring>
#include <locale>
#include <vector>
#include <cstdlib>

//scanner scan;
using namespace std;

Reporter::Reporter(){}

void Reporter::do_report(vector<string> context){

    //mount = m;
    vector<string> required = {"id","path","name"};
    string name;
    string path;
    string id;
    for(string current:context){
        string id_ = scan.lowerString(current.substr(0,current.find('=')));
        current.erase(0, id_.length()+1);
        if(current.substr(0,1) =="\"")
        {
            current = current.substr(1,current.length()-2);
        }
        if(scan.compare(id_,"name")){
            if(count(required.begin(), required.end(), id_)){
                auto itr = find(required.begin(), required.end(), id_);
                required.erase(itr);
                name = current;
            }
        }else if(scan.compare(id_,"id")){
            if(count(required.begin(), required.end(), id_)){
                auto itr = find(required.begin(), required.end(), id_);
                required.erase(itr);
                id = current;
            }
        }else if(scan.compare(id_,"path")){
            if(count(required.begin(), required.end(), id_)){
                auto itr = find(required.begin(), required.end(), id_);
                required.erase(itr);
                path = current;
            }
        }
    }
    if(required.size()!=0){
        scan.handler("REPORT", " faltan parametros para realizar el reporte");
        return;
    }
    if (scan.compare(name, "MBR")) {
        report_mbr(path, id);
    }
    else{
        scan.handler("REPORT", "El reporte solicitado no existe");
    }
}

void Reporter::report_mbr(string pathREPORT, string id) {
    try {
        string pathMBR;
        Structs::Partition partition = mount.getmount(id, &pathMBR);

        FILE *file = fopen(pathMBR.c_str(), "rb+");
        if (file == NULL) {
            throw runtime_error("disco no existente");
        }

        Structs::MBR disco;
        rewind(file);
        fread(&disco, sizeof(Structs::MBR), 1, file);
        fclose(file);

        string pd = pathREPORT.substr(0, pathREPORT.find('.'));
        pd += ".dot";
        FILE *doc = fopen(pd.c_str(), "r");
        if (doc == NULL) {
            string cmm = "mkdir -p \"" + pd + "\"";
            string cmm2 = "rmdir \"" + pd + "\"";
            system(cmm.c_str());
            system(cmm2.c_str());
        } else {
            fclose(doc);
        }
        } catch (exception &e) {
        scan.handler("REPORT", e.what());
    }
}