#include <iostream>

#ifndef console_header
#define console_header
using namespace std;

namespace Console{
    static void backspace(){
        cout << "\b" << flush;
    }

    static void backspace(int amount){
        for(int i=0; i<amount; i++){
            backspace();
        }
    }

    static void ring_bell(){
        cout << "\a" << flush;
    }

    static void save_cursor(){
        cout << "\e7" << flush;
    }

    static void load_cursor(){
        cout << "\e8" << flush;
    }

    static void clear_line(){
        cout << "\e[2K" << "\033[1A" << endl;
    }

    static void print(string text){
        cout << text << flush;
    }

    static void println(string text){
        cout << text << endl;
    }

    static void set_blink(){
        cout << "\e[5m" << flush;
    }
};
#endif