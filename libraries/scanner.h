#ifndef SCANNER_H
#define SCANNER_H
#include <vector>
#include <string>
// #include <stdlib.h>
// #include <locale>
// #include <fstream>
//#include <cstlib>

using namespace std;

class scanner{
    public:
        scanner();
        bool compare(string texta, string textb);
        string upperString(string texto);
        string lowerString(string texto);
        void handler(string title, string mensaje);
        void start();
        string token(string texto);
        void functions(string comando, vector<string> instruciones); //Recibe el comando y lista de instrucciones
        vector<string> split(string texto, string separador);
        vector<string> split_tokens(string texto);
        void errores(string operacion, string mensaje);
        void respuesta(string operacion, string mensaje);
        void funcion_exec(vector<string> tokens);
        void exec(string path);
        bool confirm(string mensaje);
};


#endif