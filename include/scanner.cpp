#include "../libraries/scanner.h"
#include "../libraries/disk.h"

#include <iostream>
#include <stdlib.h>
#include <locale>
#include <cstring>
#include <locale>
#include <fstream>
#include <vector>
#include <cstdlib>

using namespace std;

Disk disco;
scanner::scanner()
{
}

void ClearScreen() {
    cout << string( 4, '\n' );
}

void scanner::start()
{
    system("clear");
    
    while (true)
    {
        cout << "-----------------Porfis ingresa un comando; o exit para salir---------------------\n" << endl;
        //cout << "--------------------------------exit para salir-------------------------------\n" << endl;
        cout << ">>";
        string texto;
        getline(cin, texto);
        //texto = upperString(texto);
        if (upperString(texto) == "EXIT"){
            break;
        }
        string tk = token(texto); // mkdisk
        texto.erase(0,tk.length()+1);
        vector<string> tks = split_tokens(texto); //[-s=10, -u=m, -path=/home/hola.dk]
        cout << "Comand: " << tk << endl;
        cout << "Parameters: ";
        for(string &a: tks){
            cout << a << " ";
        }
        functions(tk, tks);
        //cout << "\nPresione Enter para continuar...." << endl;
        //getline(cin,texto);
        ClearScreen();  
    }
    
}


string scanner::token(string texto){
    string tk="";
    for(char &caracter: texto){
        if(caracter == ' '){
            return tk;
        }
        tk+=caracter;
    }
    return tk;
}

vector<string> scanner::split(string texto, string separador){
    vector<string> tokens;
    string tk="";
    for(char &caracter: texto){
        if(caracter == separador[0]){
            tokens.push_back(tk);
            tk="";
        }else{
            tk+=caracter;
        }
    }
    tokens.push_back(tk);
    return tokens;
}

vector<string> scanner::split_tokens(string text){
    vector<string> tokens;
    if (text.empty())
    {
        return tokens;
    }
    text.push_back(' ');
    string token = "";
    int estado = 0;
    for(char &c: text){
        if (estado ==0 && c=='-')
        {
            estado = 1;

        }else if(estado==0 && c=='#'){
            continue;
        }else if(estado!=0){
            if (estado == 1)
            {
                if(c=='='){
                    estado = 2;
                }else if(c == ' '){
                    continue;
                }
            }else if(estado == 2){
                if (c=='\"')
                {
                    estado = 3;
                }else{
                    estado = 4;
                }
                
            }else if(estado == 3){
                if (c=='\"')
                {
                    estado = 4;
                }
            }else if (estado==4 && c=='\"')
            {
                tokens.clear();
                continue;
            }else if (estado ==4 && c==' ')
            {
                estado = 0;
                tokens.push_back(token);
                token = "";
                continue;
            }
            token+=c;
        }
    }
    return tokens;
}


string scanner::upperString(string a){
    string up="";
    for(char &a: a){
        up+=toupper(a);
    }
    return up;  
}

bool scanner::compare(string a, string b){
    return upperString(a) == upperString(b);
}

void scanner::errores(string operacion, string mensaje){    
    cout << "\033[1;41m Error\033"<< "\033[0;31m(" + operacion + ")~~> \033[0m"<< mensaje << endl;
}

void scanner::respuesta(string operacion, string mensaje){    
    cout << "\033[0;42m(" + operacion + ")~~> \033[0m"<< mensaje << endl;
}


void scanner::functions(string comand, vector<string> parameters){
    if (compare(comand, "MKDISK")){     //mkdisk -s=5 -u=M -path/home/hola.dsk
        cout << " \n Creando disco..." << endl;
        disco.mkdisk(parameters);

    }else if(compare(comand, "RMDISK")){
        cout << "Eliminando disco..." << endl;
    }else if(compare(comand, "FDISK")){
        cout << "Creando particion..." << endl;
    }else if(compare(comand, "MOUNT")){
        cout << "Montando particion..." << endl;
    }else if(compare(comand, "UNMOUNT")){
        cout << "Desmontando particion..." << endl;
    }else if(compare(comand, "REP")){
        cout << "Generando reporte..." << endl;
    }else if(compare(comand, "EXEC")){
        cout << "Ejecutando script..." << endl;
    }else if(compare(comand, "PAUSE")){
        cout << "Pausando programa..." << endl;
    }else if(compare(comand, "MKFS")){
        cout << "Formateando particion..." << endl;
    }else if(compare(comand, "LOGIN")){
        cout << "Iniciando sesion..." << endl;
    }else if(compare(comand, "LOGOUT")){
        cout << "Cerrando sesion..." << endl;
    }else if(compare(comand, "MKGRP")){
        cout << "Creando grupo..." << endl;
    }else if(compare(comand, "RMGRP")){
        cout << "Eliminando grupo..." << endl;
    }else if(compare(comand, "MKUSR")){
        cout << "Creando usuario..." << endl;
    }else if(compare(comand, "RMUSR")){
        cout << "Eliminando usuario..." << endl;
    }else if(compare(comand, "CHMOD")){
        cout << "Cambiando permisos..." << endl;
    }else if(compare(comand, "MKFILE")){
        cout << "Creando archivo..." << endl;
    }else if(compare(comand, "CAT")){
        cout << "Mostrando contenido de archivo..." << endl;
    }else if(compare(comand, "REM")){
        cout << "Eliminando archivo..." << endl;
    }else if(compare(comand, "EDIT")){
        cout << "Editando archivo..." << endl;
    }else if(compare(comand, "REN")){
        cout << "Renombre de archivo..." << endl;
    }else if(compare(comand, "MKDIR")){
        cout << "Creando directorio..." << endl;
    }
    else{
        cout << "Comando no reconocido..." << endl;
    }
}