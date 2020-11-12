
#include <stdio.h>
#include <unistd.h>

#include "bpNetwork.cpp"
#include "../Helper/routine.cpp"
#include "../Helper/console.h"

class BackPropagationRoutine: public Routine{
    private:
        BackPropagationNetwork* bpNetwork;

        int* hiddenLayerSizes;
        int maxIterations, numInputNeurons, numOutputNeurons, numHiddenLayers;
        float learningRate, momentum;
        
        void init(){
            for(string s: required_params){
                checkParameterExists(s);
            } 

            //Set parameters
            this->maxIterations = stoi(params["maxIterations"]);
            this->numInputNeurons = stoi(params["numInputNeurons"]);
            this->numOutputNeurons = stoi(params["numOutputNeurons"]);
            this->numHiddenLayers = stoi(params["numHiddenLayers"]);
            this->learningRate = stof(params["learningRate"]);
            this->momentum = stof(params["momentum"]);

            //Retrieve layer sizes from params
            hiddenLayerSizes = new int[numHiddenLayers];
            for(int i=0; i<numHiddenLayers; i++){
                string layerName = "layer-"+ to_string(i);
                checkParameterExists(layerName);
                hiddenLayerSizes[i] = stoi(params[layerName]);
                if(hiddenLayerSizes[i]==0){
                    cout << "Layer size cannot be 0." << endl;
                    cout << "Exiting..." << endl;
                    exit(0);
                }
            }

            //Create network
            bpNetwork = new BackPropagationNetwork(numInputNeurons, numOutputNeurons, numHiddenLayers, hiddenLayerSizes); 
            network = bpNetwork;
            if (DEBUG){
                cout << "Printing network..." << endl;

                network->printNeurons();
                cout << endl;

                network->printNetwork();
                cout << endl;
            }
        }
    
    public:
        string required_params[12] = {"maxIterations", "numInputNeurons", "numOutputNeurons", "numHiddenLayers", "learningRate", "momentum", "train_test_ratio", "progressbar_length", "trainFile", "testFile", "errors_out", "accuracy_out"};

        void handleMissingParameter(string parameter){
            cout << "Parameter: \"" << parameter << "\" was not defined, program exiting..." << endl;
            exit(0);
        }

        void checkParameterExists(string parameter){
            if(params.find(parameter) == params.end()) handleMissingParameter(parameter);
        }

        BackPropagationRoutine(unordered_map<string, string> params):Routine(params){
            init();
        }

        BackPropagationRoutine(string params_in, char delimeter):Routine(params_in, delimeter){
            init();
        }

        void readUniformDataSet(vector<string> labels){
            checkParameterExists("datasetFile");
            unordered_map<string, vector<vector<string>>> data = readLabeledVarSizeData(params["datasetFile"], ',');
            vector<pair<string, vector<string>>> trainset, testset;
            
            cout << "-> Creating train set and test set files from uniform, labeled dataset ..." << endl;
            //split data into train and test data for every input type
            for(string s : labels){
                int datasize = data[s].size();
                int trainsize = datasize * stof(params["train_test_ratio"]);
                int testsize = datasize - trainsize;
                if(DEBUG){
                    cout << "label: " << s << endl;
                    cout << "data size is: " << datasize << endl;
                    cout << "train size: " << trainsize << endl;
                    cout << "test size: " << testsize << endl;
                }

                for(int i=datasize-1; i>=trainsize; i--){
                    testset.push_back(make_pair(s, data[s].at(i)));
                    data[s].erase(data[s].begin()+i);
                }
                for(int i=trainsize-1; i>=0; i--){
                    trainset.push_back(make_pair(s, data[s].at(i)));
                    data[s].erase(data[s].begin()+i);
                }
            }

            //fill trainstream
            stringstream trainstream = fillstream(labels, trainset);
            stringstream teststream = fillstream(labels, testset);
            
            writeFile(params["trainFile"], trainstream.str());
            writeFile(params["testFile"], teststream.str());
        }

        stringstream fillstream(vector<string> labels, vector<pair<string, vector<string>>> dataset){
            stringstream stream;
            srand(time(NULL));
            int total_data = dataset.size();
            for(int i=0; i<total_data; i++){
                //get a random entry from all train set to randomize order
                int index = (int)(rand() % dataset.size());
                pair<string, vector<string>> entry = dataset.at(index);
                dataset.erase(dataset.begin()+index);
                for(int j=0; j<numInputNeurons; j++){
                    stream << entry.second.at(j) << " ";
                }
                for(string s: labels){
                    if(s == entry.first){
                        stream << 1 << " ";
                        continue;
                    }
                    stream << 0 << " ";
                }
                stream << endl;
            }
            return stream;
        }

        //Store info to dump into file later
        pair<double, double>* successes;
        pair<double, double>* errors;
        void run_routine(){
            successes = new pair<double, double>[maxIterations];
            errors = new pair<double, double>[maxIterations];
            pair<double, double> temp; 

            cout << "-> Training network for " << maxIterations << " cycles ..." << endl;
            cout << "-> Learning rate: " << learningRate << ", Momentum: " << momentum << endl;
            for(int cycle=0; cycle<maxIterations; cycle++){
                cout << "\t> Simulating cycle " << cycle << "/" << maxIterations << " " << endl;
                
                int progressbar_length = stoi(params["progressbar_length"]);
                if (VISUALIZE) cout << "> Training network:" << endl;
                else if(Console::SHOW_PROGRESS) {
                    Console::create_progressbar(progressbar_length);
                }
                    
                temp = bpNetwork->trainNetwork(learningRate, momentum, numTrainSamples, train_inputs, train_outputs);
                successes[cycle].first = temp.first;
                errors[cycle].first = temp.second;
                if(VISUALIZE) cout << "> Training done... " << endl << endl;

                if(VISUALIZE) cout << "> Testing network: " << endl;
                else if(Console::SHOW_PROGRESS){
                    Console::clear_line();
                    Console::create_progressbar(progressbar_length);
                }
                temp = bpNetwork->evaluateNetwork(numTestSamples, test_inputs, test_outputs);
                successes[cycle].second = temp.first;
                errors[cycle].second = temp.second;
                if(VISUALIZE) cout << "> Testing done... " << endl << endl;
                else if(Console::SHOW_PROGRESS) {
                    Console::clear_line();
                    Console::cursor_up();
                    Console::clear_line();
                }
                if(cycle != maxIterations-1 && cycle % VISUALIZATION_ITERATION_INTERVAL == 0) Console::ring_bell();
            }

            cout << endl << "Routine DONE..." << endl << endl;
            //system("./play_clip.sh");
        }

        void writeResults(){
            //Store reults in files
            stringstream out_errors, out_successes;
            out_errors.precision(4);
            out_successes.precision(2);
            for(int i=0; i<maxIterations; i++){
                out_errors << fixed << (i+1) << "\t" << errors[i].first << "\t" << errors[i].second << endl;
                out_successes << fixed << (i+1) << "\t" << successes[i].first << "%\t" << successes[i].second << "%" << endl;
            }
            Routine::writeResults(params["errors_out"], params["accuracy_out"], out_errors.str(), out_successes.str());
        }

        void readDataSetsFromFiles(){
            Routine::readDataSetsFromFiles(numInputNeurons, numOutputNeurons);
        }
};

#ifndef runner_cpp
int main(){
    cout << "bpRoutine.cpp" << endl;

    BackPropagationRoutine* routine = new BackPropagationRoutine("parameters.txt", ' ');
    if(stoi(routine->params["createDatasets"]) != 0){
        vector<string> labels;
        for(char c='A'; c<='Z'; c++)
            labels.push_back(string(1,c)); 
        routine->readUniformDataSet(labels);
    }
    routine->readDataSetsFromFiles();
    routine->run_routine();
    routine->writeResults();
    return 0;
}
#endif