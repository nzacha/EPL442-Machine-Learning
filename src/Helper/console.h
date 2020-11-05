#include <iostream>
#include <sstream>

#ifndef console_header
#define console_header
using namespace std;

namespace Console{
    bool SHOW_PROGRESS = false;
    
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

    static void cursor_up(){
        cout << "\033[1A" << flush;
    }

    static void cursor_up(int n){
        string text = "\33["+ to_string(n) +"A";
        cout << text << flush;
    }

    static void cursor_forward(){
        cout << "\33[1C" << flush;
    }

    static void cursor_forward(int n){
        string text = "\33["+ to_string(n) +"C";
        cout << text << flush;
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

    int progressbar_length = 0, textSize = 0;
    static int print_progressbar(int progress, float percentage, int job, int jobs_size){
        if(!SHOW_PROGRESS) return 0;

        stringstream pb; 

        //print progressbar
        pb << "=> [";
        for(int i=0; i<progress; i++)
            pb << "#";
        for(int i=progress; i<progressbar_length; i++)
            pb << " ";
        pb << "] " << flush;
        
        //print label
        pb.precision(2);
        pb << job << "/" << jobs_size << " (" << fixed << (percentage * 100) << "%) " << flush;

        cout << pb.str() << flush;
        return pb.str().length();
    }

    void showProgress(int job, int jobs_size){
        float roundup_offset = progressbar_length * 1.0f / jobs_size / 2;
        float percentage = job * 1.0f / jobs_size;
        int progress = (int) (percentage * progressbar_length + roundup_offset);

        textSize = print_progressbar(progress, percentage, job, jobs_size);
    }

    static void clear_progressbar(){
        backspace(textSize);
        textSize = 0;
    }

    static void update_progressbar(int job, int jobs_size){
        if(job > jobs_size) return;
        
        //clear_progressbar();
        showProgress(job, jobs_size);
    }

    static void create_progressbar(int max){
        progressbar_length = max;
        showProgress(0, 1);
    }
};
#endif
