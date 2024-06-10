#include "../libraries/scanner.h"

#include <iostream>
#include <stdlib.h>
#include <locale>
#include <cstring>
#include <locale>
#include <fstream>
#include <vector>
#include <cstdlib>

using namespace std;

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
        cout << "--------------------Porfis ingresa un comando; o exit para salir--------------------------\n" << endl;
        //cout << "--------------------------------exit para salir-------------------------------\n" << endl;
        cout << ">>";
        string texto;
        getline(cin, texto);
        texto = upperString(texto);
        if (texto == "EXIT"){
            break;
        }
        /*string tk = token(texto); // mkdisk
        texto.erase(0,tk.length()+1);
        vector<string> tks = split_tokens(texto); //[-size=10, -u=m, -path=/home/hola.dk]
        functions(tk, tks);*/
        //cout << "\nPresione Enter para continuar...." << endl;
        //getline(cin,texto);
        ClearScreen();  
    }
    
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