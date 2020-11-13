#include <iostream>
#include <sstream>
#include <chrono>

#ifndef console_header
#define console_header
typedef std::chrono::high_resolution_clock Clock;
using namespace std;

class TimeTracker{
    private:
        string levels[7] = {"ns.", "mc.", "ms.", "secs.", "mins.", "h.", "days"};
        int modifier[6] = {1000, 1000, 1000, 60, 60, 24};
        long long unsigned int tracked_ETA, tracked_elapsed;
        std::chrono::time_point<std::chrono::_V2::system_clock, std::chrono::duration<long int, std::ratio<1, 1000000000> > > tracked_time;
        bool begun_tracking = false;
        int tracked_level=0;
        
        double getTimeDiff(){
            auto now = Clock::now();
            long long unsigned int diff = std::chrono::duration_cast<std::chrono::nanoseconds>(now-tracked_time).count();
            tracked_time = now;
            return diff;
        }

    public:
        int updateTimeInNanoSeconds = 200000000;
    
        TimeTracker(){};

        string  get_tracked_time(int jobsRemaining){
            long long unsigned int time_taken = getTimeDiff();
            tracked_elapsed += time_taken;
            if(!begun_tracking) {
                begun_tracking = true;
                return "";
            }
            if(tracked_elapsed > 200000000){
                tracked_elapsed = 0;
                tracked_ETA = (time_taken) * jobsRemaining;
                tracked_level = 0;

                while(tracked_ETA / modifier[tracked_level] > 1){
                    tracked_ETA /= modifier[tracked_level];
                    tracked_level++;
                }
            }

            stringstream ret;
            ret.precision(2);
            ret << "ETA: " << fixed << tracked_ETA << " " << levels[tracked_level] << flush;
            return ret.str();
        }
};

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

    TimeTracker progressbarTracker;
    int progressbar_length = 0, textSize = 0;
    int print_progressbar(int progress, float percentage, int job, int jobs_size){
        if(!SHOW_PROGRESS) return 0;
        
        stringstream pb;
        pb.precision(2); 

        //print progressbar
        pb << "=> [";
        for(int i=0; i<progress; i++)
            pb << "#";
        for(int i=progress; i<progressbar_length; i++)
            pb << " ";
        pb << "] " << flush;
        
        //print label
        pb << job << "/" << jobs_size << " (" << fixed << (percentage * 100) << "%) " << flush;

        
        pb << progressbarTracker.get_tracked_time(jobs_size-job) << flush;
        
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
