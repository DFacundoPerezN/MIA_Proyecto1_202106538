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
        report_disk(path, id);
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
    cout << "Particion de tipo: " << partition.part_type << endl;

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
// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
//                                  REPORTE DE DISCO
// IIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII

void Reporter::report_disk(string pathReport, string id) {
    try {
        string pathDisk = getPath(id);
        if (pathDisk.empty()) {
            throw runtime_error("no se ha encontrado el disco");
        }
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
        string content = dotDisk(pathDisk); //contenido del archivo dot

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

string Reporter::dotDisk(string pathMBR){
    FILE *file = fopen(pathMBR.c_str(), "rb+");
    if (file == NULL) {
        throw runtime_error("disco no existente");
    }

    Structs::MBR disco;
    rewind(file);
    fread(&disco, sizeof(Structs::MBR), 1, file);
    fclose(file);

    Structs::Partition partitions[4];
    partitions[0] = disco.mbr_Partition_1;
    partitions[1] = disco.mbr_Partition_2;
    partitions[2] = disco.mbr_Partition_3;
    partitions[3] = disco.mbr_Partition_4;
    Structs::Partition extended;
    bool ext = false;
    for (int i = 0; i < 4; ++i) {
        if (partitions[i].part_status == '1') {
            if (partitions[i].part_type == 'E') {
                ext = true;
                extended = partitions[i];
            }
        }
    }
    
    string content="digraph G{\n"
                  "node [shape = plaintext];\n";
        content += "nodo1 [nodo1 [fontcolor=darkslateblue color=aquamarine3 fontname=Arial\n"
            "label = <<table>\n";
        content += "<tr>\n";

        int positions[5] = {0, 0, 0, 0, 0};
        int positionsii[5] = {0, 0, 0, 0, 0};
        positions[0] = disco.mbr_Partition_1.part_start - (1 + sizeof(Structs::MBR));
        positions[1] = disco.mbr_Partition_2.part_start - (disco.mbr_Partition_1.part_start + disco.mbr_Partition_1.part_size);
        positions[2] = disco.mbr_Partition_3.part_start - (disco.mbr_Partition_2.part_start + disco.mbr_Partition_2.part_size);
        positions[3] = disco.mbr_Partition_4.part_start - (disco.mbr_Partition_3.part_start + disco.mbr_Partition_3.part_size);
        positions[4] = disco.mbr_size + 1 - (disco.mbr_Partition_4.part_start + disco.mbr_Partition_4.part_size);

        copy(positions, positionsii, positionsii);
        for (size_t j = 0; j < 5; j++) {
            bool negative = false;
            for (size_t i = 0; i < 4; i++) {
                if (positions[i] < 0) {
                    negative = true;
                }
                if (positions[i] <= 0 && positionsii[i] <= 0 && negative && positions[i + 1] > 0) {
                    positions[i] = positions[i] + positions[i + 1];
                    positions[i + 1] = 0;
                }
            }
            negative = false;
        };

        int logic = 0;
        string tmplogic;
        if (ext) {
            tmplogic = "<tr>\n";
            Structs::EBR aux;
            FILE *ext = fopen(pathMBR.c_str(), "r+b");
            fseek(ext, extended.part_start, SEEK_SET);
            fread(&aux, sizeof(Structs::EBR), 1, ext);
            fclose(ext);
            while (aux.part_next != -1) {
                float res = (float) aux.part_size / (float) disco.mbr_size;
                res = round(res * 10000.00F) / 100.00F;
                tmplogic += "<td>EBR</td>";
                tmplogic += "<td>Logica\n" + to_string(res) + "% del disco</td>\n";
                float resta = (float) aux.part_next - ((float) aux.part_start + (float) aux.part_size);
                resta = resta / disco.mbr_size;
                resta = resta * 10000.00F;
                resta = round(resta) / 100.00F;
                if (resta != 0) {
                    tmplogic += "<td>Logica\n" + to_string(resta) + "% libre del disco</td>\n";
                    logic++;
                }
                logic += 2;
                FILE *ext2 = fopen(pathMBR.c_str(), "r+b");
                fseek(ext2, aux.part_next, SEEK_SET);
                fread(&aux, sizeof(Structs::EBR), 1, ext2);
                fclose(ext2);
            }
            float res = (float) aux.part_size / (float) disco.mbr_size;
            res = round(res * 10000.00F) / 100.00F;
            tmplogic += "<td>EBR</td>";
            tmplogic += "<td>Logica\n" + to_string(res) + "% del disco</td>\n";
            float resta = (float) extended.part_size -
                          ((float) aux.part_start + (float) aux.part_size - (float) extended.part_start);
            resta = resta / disco.mbr_size;
            resta = resta * 10000.00F;
            resta = round(resta) / 100.00F;
            if (resta != 0) {
                tmplogic += "<td>Libre\n" + to_string(resta) + "% del disco</td>\n";
                logic++;
            }
            tmplogic += "</tr>\n\n";
            logic += 2;
        }

        for (int i = 0; i < 4; ++i) {
            if (partitions[i].part_type == 'E') {
                content += "<td COLSPAN='" + to_string(logic) + "'> Extendida </td>\n";
            } else {
                if (positions[i] != 0) {
                    float res = (float) positions[i] / (float) disco.mbr_size;
                    res = round(res * 100.0F * 100.0F) / 100.0F;
                    content += "<td ROWSPAN='2'> Libre \n" + to_string(res) + "% del disco</td>";
                } else {
                    float res = ((float) partitions[i].part_size) / (float) disco.mbr_size;
                    res = round(res * 10000.00F) / 100.00F;
                    content += "<td ROWSPAN='2'> Primaria \n" + to_string(res) + "% del disco</td>";
                }
            }

        }
        if (positions[4] != 0) {
            float res = (float) positions[4] / (float) disco.mbr_size;
            res = round(res * 100.0F * 100.0F) / 100.0F;
            content += "<td ROWSPAN='2'> Libre \n" + to_string(res) + "% del disco</td>";
        }

        content += "</tr>\n\n";
        content += tmplogic;

        content += "</table>>];\n}\n";

    return content;
}   