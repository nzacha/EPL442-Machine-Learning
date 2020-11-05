#ifndef main_func
#define main_func
#define runner_cpp
#endif

#include <iostream> 
#include <thread>

#include "bpRoutine.cpp"

using namespace std; 
void silenceOutputStream(){
		cout.setstate(ios_base::failbit);
	}

void activateOutputStream(){
	cout.clear();
}

void createStandardBPRoutine(int index, unordered_map<string, string> params, bool createDatasets){
	params["errors_out"] = params["dir_out"] + "/" + params["errors_out"] + "_" + to_string(index) +".txt";
	params["accuracy_out"] = params["dir_out"] + "/" + params["accuracy_out"] + "_" + to_string(index) +".txt";
	BackPropagationRoutine* routine = new BackPropagationRoutine(params);
    if(createDatasets){
        vector<string> labels;
        for(char c='A'; c<='Z'; c++)
            labels.push_back(string(1,c)); 
        routine->readUniformDataSet(labels);
    }
    routine->readDataSetsFromFiles();
    routine->run_routine();
    routine->writeResults();
	cerr << "Thread " << index << " finished" << endl;
}

void printHelp(){
	cout << "runner.cpp" << endl;
	cout << "\t-q , -quiet : to quiet outputstream" << endl;
	cout << "\t-t [number of threads] : to enable threaded execution with specified no. of threads" << endl;
	cout << "\t-c : to create input and output dataset from single dataset" << endl << "\tthis option exits the program when done.";
}

#ifdef runner_cpp
int main(int argc, char** argv){ 
	int numThreads = 1;
	bool createDatasets = false;

	for(int i=0; i<argc; i++){
		string arg = argv[i];
		if(arg == "-q" || arg == "-quiet"){
			silenceOutputStream();
		}else if(arg == "-t" ){
			arg = argv[++i];
			numThreads = stoi(arg);
			cout << "Number of threads set to " <<  numThreads << endl;
		}else if(arg == "-c"){
			createDatasets = true;
			cout << "Input and Output sets will be created from dataset file " << endl;
		}else if(arg == "-h" || arg == "-help"){
			printHelp();
			return 0;
		}else if(arg == "-progress"){
			Console::SHOW_PROGRESS = true;
			cout << "Progress bar will be displayed" << endl;
		}
	}
	
	cout << "runner.cpp" << endl;
	
	if(createDatasets){
		BackPropagationRoutine* routine = new BackPropagationRoutine("parameters.txt", ' ');
		vector<string> labels;
		for(char c='A'; c<='Z'; c++)
			labels.push_back(string(1,c)); 
		routine->readUniformDataSet(labels);
		cout << "Input and Output sets created " << endl;
		cout << "Program exits..." << endl;
		return 0;
	}

	unordered_map<string, string> params = readParameters("parameters.txt", ' ');
	if(numThreads > 1){
		silenceOutputStream();
		Console::SHOW_PROGRESS = false;
		thread threads[numThreads];
		BackPropagationRoutine* bproutines[numThreads];
		
		for(int i=0; i<numThreads; i++){
			cerr << "Thread " << i << " starts" << endl;
			threads[i] = thread(createStandardBPRoutine, i, params, true);
		}
		for(int i=0; i<numThreads; i++){
			threads[i].join();
		}
	
		activateOutputStream();
		cout << "All routines DONE" << endl;
	} else {
		BackPropagationRoutine* routine = new BackPropagationRoutine("parameters.txt", ' ');
		routine->readDataSetsFromFiles();
		routine->run_routine();
		routine->writeResults();
    return 0;	

		activateOutputStream();
		cout << "Routine DONE" << endl;
	}
	return 0; 
}
#endif