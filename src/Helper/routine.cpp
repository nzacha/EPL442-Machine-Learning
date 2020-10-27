#include <unordered_map>

#include "helper.cpp"

using namespace std;

class Routine{
    public:
        unordered_map<string,string> params;

        Routine(string params_in, char delimeter){
            this->params = readParameters("parameters.txt", delimeter);
        }

        virtual void run_routine()=0; 
        virtual void writeResults()=0;
        
        void writeResults(string error_out, string accuracy_out, string error_content, string accuracy_content){
            cout << "Writing error output to: " << error_out << endl;
            writeFile(error_out, error_content);
            cout << "Writing accuracy output to: " << accuracy_out << endl;
            writeFile(accuracy_out, accuracy_content);
        }
};