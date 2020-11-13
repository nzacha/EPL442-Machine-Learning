
#include <stdio.h>
#include <unistd.h>

#include "bpNetwork.cpp"
#include "../Helper/routine.cpp"
#include "../Helper/console.h"

class BackPropagationRoutine: public Routine{
    private:
        BackPropagationNetwork* bpNetwork;

        int* hiddenLayerSizes;
        int numHiddenLayers;
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
        BackPropagationRoutine(unordered_map<string, string> params):Routine(params){
            init();
        }

        BackPropagationRoutine(string params_in, char delimeter):Routine(params_in, delimeter){
            init();
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
            TimeTracker* cycleTracker = new TimeTracker();
            for(int cycle=0; cycle<maxIterations; cycle++){
                cout << "\t> Simulating cycle " << cycle << "/" << maxIterations << " " << flush;
                cout << cycleTracker->get_tracked_time(maxIterations-cycle) << endl;

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