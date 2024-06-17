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

extern Mount mounter;
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
    else if(scan.compare(name, "DISK")){
        //report_disk(path, id);
    }
    else{
        scan.handler("REPORT", "El reporte solicitado no existe");
    }
}

void Reporter::report_mbr(string pathReport, string id) {
    try {
        string pathMBR;

        Structs::Partition partition = mounter.getmount(id, &pathMBR);
        std::cout << "Buscando archivo en" << pathMBR << std::endl;

        FILE *file = fopen(pathMBR.c_str(), "rb+");
        if (file == NULL) {
            throw runtime_error("disco no existente");
        }

        Structs::MBR disco;
        rewind(file);
        fread(&disco, sizeof(Structs::MBR), 1, file);
        fclose(file);

        string dotPath = pathReport.substr(0, pathReport.find('.'));
        dotPath += ".dot";
        FILE *doc = fopen(dotPath.c_str(), "r");
        if (doc == NULL) {
            string cmm = "mkdir -p \"" + dotPath + "\"";
            string cmm2 = "rmdir \"" + dotPath + "\"";
            system(cmm.c_str());
            system(cmm2.c_str());
        } else {
            fclose(doc);
        }
        string content = dotMBR(disco, pathMBR); //contenido del archivo dot

        ofstream reportfile(dotPath);      //creación del archivo dot
        reportfile << content.c_str() << endl; //escritura del contenido
        reportfile.close();        //cierre del archivo

        std::cout << "intentando generar reporte en: "<< dotPath << endl;
        string function = "dot -Tjpg " + dotPath + " -o " + pathReport;         //comando para generar la imagen
        system(function.c_str());    //ejecución del comando
    }
    catch (exception &e) {
        scan.handler("REPORT", e.what());
    }


}
string Reporter::dotMBR(Structs::MBR disco, string path){

    struct tm *tm;
    tm = localtime(&disco.mbr_date_creacion);
    char mostrar_fecha [20];
    strftime(mostrar_fecha, 20, "%Y/%m/%d %H:%M:%S", tm);

    //datos del disco
    string dot = "digraph G {\n\n" 
                    "\tlabel = <\n" 
                    "\t\t<table>\n"
                    "\t\t\t<tr><td colspan='2' bgcolor='darkviolet' ><font color='gold'>REPORTE DE MBR</font></td></tr>\n"
                    "\t\t\t<tr><td bgcolor='azure'>mbr_size </td> "
                    "<td bgcolor='azure'> " + to_string(disco.mbr_size) + "</td></tr>\n"
                    "\t\t\t"
                    "<tr><td bgcolor='plum1'>mbr_date_creation</td>"
                    "<td bgcolor='plum1'>" + string(mostrar_fecha) + "</td></tr>\n"
                    "\t\t\t"
                    "<tr><td bgcolor='azure'>mbr_disk_signature</td>"
                    "<td bgcolor='azure'>" + to_string(disco.mbr_disk_signature) + "</td></tr>\n\n";

    //datos de las particiones
    dot += dotPartition(disco.mbr_Partition_1, path);
    dot += dotPartition(disco.mbr_Partition_2, path);
    dot += dotPartition(disco.mbr_Partition_3, path);
    dot += dotPartition(disco.mbr_Partition_4, path);/**/

    //finalización
    dot+="\t\t </table>\n\t >\n\n}";
    return dot;
}

string Reporter::dotPartition(Structs::Partition partition, string path){
    string dot = " ";
    string status = to_string(partition.part_status);

    if(partition.part_status == '1'){
        dot +=  "\t\t\t<tr><td colspan='2' bgcolor='darkviolet'>" "<font color='gold'>Particion</font></td></tr>\n"
                "\t\t\t<tr><td bgcolor='azure'>part_status</td>"
                "<td bgcolor='azure'> " + status + "</td></tr>\n"
                "\t\t\t"
                "<tr><td bgcolor='plum1'>part_type</td>"
                "<td bgcolor='plum1'>" + partition.part_type + "</td></tr>\n"
                "\t\t\t"
                "<tr><td bgcolor='azure'>part_fit</td>"
                "<td bgcolor='azure'>" + partition.part_fit + "</td></tr>\n"
                "\t\t\t"
                "<tr><td bgcolor='plum1'>part_start</td>"
                "<td bgcolor='plum1'>" + to_string(partition.part_start) + "</td></tr>\n"
                "\t\t\t"
                "<tr><td bgcolor='azure'>part_size</td>"
                "<td bgcolor='azure'>" + to_string(partition.part_size) + "</td></tr>\n"
                "\t\t\t"
                "<tr><td bgcolor='plum1'>part_name</td>"
                "<td bgcolor='plum1'>" + partition.part_name + "</td></tr>\n\n";


        if(partition.part_type == 'E'){
            cout << "Se encontró una particion extendida" << endl;
            vector<Structs::EBR> ebrs = fdisco.getlogics(partition, path);
            for(Structs::EBR extended : ebrs){
                dot += dotLogicPartition(extended);
            }
        }
    }
    //cout << dot << endl;
    return dot;
}

string Reporter:: dotLogicPartition(Structs::EBR extended){
    string status = to_string(extended.part_status);

    string dot = "\t\t\t"
                "<tr><td colspan='2' bgcolor='violetred1'><font color='gold'>Particion Logica</font></td></tr>"
                "\t\t\t"
                "<tr><td bgcolor='azure'>part_status</td>"
                "<td bgcolor='azure'>" + status+
                 "</td></tr>\n" 
                "\t\t\t"
                " <tr><td bgcolor='pink'>part_next</td>"
                "<td bgcolor='pink'>" + to_string(extended.part_next);
    dot += "</td></tr>\n"
            "\t\t\t"
            "<tr><td bgcolor='azure'>part_fit</td>"
            "<td bgcolor='azure'>" + extended.part_fit;
    dot += "</td></tr>\n"
            "\t\t\t"
            "<tr><td bgcolor='pink'>part_start</td>"
            "<td bgcolor='pink'>" + to_string(extended.part_start);
    dot += "</td></tr>\n"
            "\t\t\t"
            "<tr><td bgcolor='azure'>part_size</td>"
            "<td bgcolor='azure'>" + to_string(extended.part_size);
    dot += "</td></tr>\n"
            "\t\t\t"
            "<tr><td bgcolor='pink'>part_name</td>"
            "<td bgcolor='pink'>";
    dot += extended.part_name;

    return dot;
}

string Reporter::getPath(string id) {
    string path;
    for (auto &mountedDisc : mounter.mountedDiscs) {
        for (auto &mountedPartition : mountedDisc.mpartitions) {
            if (mountedPartition.status == '1' && mountedPartition.diskName == mounter.getDiskName(id)) {
                path = mountedDisc.path;
                return path;
            }
        }
    }
    return path;
}