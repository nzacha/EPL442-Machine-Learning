#define runner_cpp

#include <iostream> 
#include <thread>

#include "BackPropagation/bpRoutine.cpp"
#include "Kohonen/khRoutine.cpp"

using namespace std; 
void silenceOutputStream(){
		cout.setstate(ios_base::failbit);
	}

void activateOutputStream(){
	cout.clear();
}

enum Program{None=-1, BackPropagation=1, Kohonen=2};

void createRoutine(int program, int index, unordered_map<string, string> params, bool createDatasets){
	Routine* routine;
    params["errors_out"] = params["errors_out"] + "_" + to_string(index);
	params["accuracy_out"] = params["accuracy_out"] + "_" + to_string(index);
	
	if(program = Program::BackPropagation){
		routine = new BackPropagationRoutine(params);
	}else if(program = Program::Kohonen){
		routine = new KohonenRoutine(params);
	}else{
		cout << "Program does not exist..." << endl;
		exit(0);
	}

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
	cout << "\t-q , -quiet : to quiet outputstream" << endl;
	cout << "\t-t [number of threads] : to enable threaded execution with specified number of threads" << endl;
	cout << "\t-progress : to show progress bar" << endl;
	cout << "\t-c : create input and train set from dataset" << endl;
	cout << "\t-sound : play sound when execution ends" << endl;
	cout << "\t-mail : send me a personal mail when execution ends" << endl;
	cout << "\t-p [program number] : to choose a program from list of programs" << endl;
	cout << "\t[program name] : to choose a program from list of programs" << endl;
	cout << "\tPrograms : " << endl;
	cout << "\t\t1. \"backpropagation\"" << endl;
	cout << "\t\t2. \"kohonen\"" << endl;
}

#ifdef runner_cpp
int main(int argc, char** argv){ 
	int numThreads = 1;
	Program program = None;
	bool createDatasets = false, play_sound = false, send_mail = false;

	for(int i=0; i<argc; i++){
		string arg = argv[i];
		if(arg == "-q" || arg == "-quiet"){
			silenceOutputStream();
		}else if(arg == "-t" ){
			arg = argv[++i];
			numThreads = stoi(arg);
			cout << "- Number of threads set to " <<  numThreads << endl;
		}else if(arg == "-c"){
			createDatasets = true;
			cout << "- Input and Output sets will be created from dataset file " << endl;
		}else if(arg == "-h" || arg == "-help"){
			printHelp();
			return 0;
		}else if(arg == "-progress"){
			Console::SHOW_PROGRESS = true;
			cout << "- Progress bar will be displayed" << endl;
		}else if(arg == "-p"){
			switch (stoi(argv[++i])){
				case 1:
					program = Program::BackPropagation;
					break;
				case 2:
					program = Program::Kohonen;
					break;
				default:
					cout << "No such program exists..." << endl;
					exit(0);
			}
		}else if(arg == "backpropagation"){
			program = Program::BackPropagation;
		}else if(arg == "kohonen"){
			program = Program::Kohonen;
		}else if(arg == "-mail"){
			send_mail = true;
			cout << "- Mail will be sent on execution ends" << endl;
		}else if(arg == "-sound"){
			play_sound = true;
			cout << "- Sound will be played when execution ends" << endl;
		}
	}
	
	cout << "runner.cpp" << endl;
	switch(program){
		case -1:
			cout << "No program chosen, exiting..." << endl << endl;
			printHelp();
			return 1;
		case 1:
			cout << "Running Back Propagation Routine" << endl;
			break;
		case 2:
			cout << "Running Kohonen Routine" << endl;
			break;
		default:
			cout << "No such program exists" << endl;
			return 1;
	}
	cout << endl;

	unordered_map<string, string> params = readParameters("parameters.txt", ' ');
	if(numThreads > 1){
		silenceOutputStream();
		Console::SHOW_PROGRESS = false;
		thread threads[numThreads];
		Routine* routines[numThreads];
		
		switch(program){
			case BackPropagation:
				for(int i=0; i<numThreads; i++)
					routines[i] = new BackPropagationRoutine("parameters.txt", ' ');
				break;
			case Kohonen:
				for(int i=0; i<numThreads; i++)
					routines[i] = new KohonenRoutine("parameters.txt", ' ');
				break;
			default:
				cerr << "No such program exists..." << endl;
				return 1;
		}
		for(int i=0; i<numThreads; i++){
			cerr << "Thread " << i << " starts" << endl;
			threads[i] = thread(createRoutine, program, i, params, createDatasets);
		}
		for(int i=0; i<numThreads; i++){
			threads[i].join();
		}
	
		activateOutputStream();
		cout << "All routines DONE" << endl;
	} else {
		Routine* routine;
		switch(program){
			case BackPropagation:
				routine = new BackPropagationRoutine("parameters.txt", ' ');
				break;
			case Kohonen:
				routine = new KohonenRoutine("parameters.txt", ' ');
				break;
			default:
				cout << "No such program exists..." << endl;
				exit(0);
		}
		if(createDatasets){
			vector<string> labels;
			for(char c='A'; c<='Z'; c++)
				labels.push_back(string(1,c)); 
			routine->readUniformDataSet(labels);
		}
		routine->readDataSetsFromFiles();
		routine->run_routine();
		routine->writeResults();
    	
		activateOutputStream();
		cout << "Routine DONE" << endl;
		if(send_mail){
			system("./send_mail.sh");
		}
		if(play_sound){
			system("./play_sound.sh");
		}
	}
	return 0; 
}
#endif