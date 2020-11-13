#include <stdio.h>
#include <unistd.h>

#include "khNetwork.cpp"
#include "../Helper/routine.cpp"
#include "../Helper/console.h"

class KohonenRoutine : public Routine{
    private:
        KohonenNetwork* khNetwork;

        int maxIterations;
        double INITIAL_LEARNING_RATE, INITIAL_NEIGHBOURHOOD_WIDTH;

        void init(){
            int input_vector_size = stoi(params["numInputNeurons"]);
            int map_width = stoi(params["mapWidth"]);
            int map_height = stoi(params["mapHeight"]);
            khNetwork = new KohonenNetwork(input_vector_size, map_width, map_height);
            network = khNetwork;

            this->maxIterations = stoi(params["maxIterations"]);
            this->INITIAL_LEARNING_RATE = stod(params["learningRate"]);
            this->INITIAL_NEIGHBOURHOOD_WIDTH = stod(params["neighbourhoodWidth"]);
        }

    public:
        KohonenRoutine(string fileName, char delimeter): Routine(fileName, delimeter){
            init();
        }

        KohonenRoutine(unordered_map<string, string> params): Routine(params){
            init();
        }
        
        int getLabelIndex(int* targetOutputs, int size){
            for(int i=0; i<size; i++){
                if(targetOutputs[i] == 1) return i;
            }
            return -1;
        }
        
        pair<int*, double*> ***pre_lvq_labels,  ***labels;
        pair<double, double>* errors;
        void run_routine(){
            errors = new pair< double,double >[maxIterations];
            pair< pair<int,int>,double >* temp;
            int numTargetOutputs = stoi(params["numOutputNeurons"]);
            
            double gainTerm = INITIAL_LEARNING_RATE;
            double functWidth = INITIAL_NEIGHBOURHOOD_WIDTH;
            cout << "-> " << khNetwork->networkDescription() << flush;
            cout << "-> Training Network For " << maxIterations << " cycles ..." << endl;
            cout << "\t- Initial Learning Term: " << INITIAL_LEARNING_RATE << endl;
            cout << "\t- Initial Neighbourhood Size: " << INITIAL_NEIGHBOURHOOD_WIDTH << endl;
            int progressbar_length = stoi(params["progressbar_length"]);
            TimeTracker* cycleTracker = new TimeTracker(); 
            for(int cycle=0; cycle<maxIterations; cycle++){
                cout << "> Simulating cycle " << cycle << "/" << maxIterations << " ";
                
                if (VISUALIZE) cout << endl << "> Training network:" << endl;
                else if(Console::SHOW_PROGRESS) {
                    cout << cycleTracker->get_tracked_time(maxIterations-cycle);
                    cout << endl;
                    Console::create_progressbar(progressbar_length);
                }
                temp = khNetwork->trainNetwork(train_inputs, numTrainSamples, gainTerm, functWidth);    
                errors[cycle].first = 0;
                for(int i=0; i<numTrainSamples; i++){
                    errors[cycle].first += temp[i].second;
                    //Console::create_progressbar(progressbar_length);cout << errors[cycle].first << " <- " << temp[i].second << endl;
                }
                errors[cycle].first /= numTrainSamples;

                if(VISUALIZE) cout << "> Training done... " << endl << endl;

                if(VISUALIZE) cout << "> Testing network: " << endl;
                else if(Console::SHOW_PROGRESS){
                    Console::clear_line();
                    Console::create_progressbar(progressbar_length);
                }
                temp = khNetwork->evaluateNetwork(test_inputs, numTestSamples);
                errors[cycle].second = 0;
                for(int i=0; i<numTestSamples; i++){
                    errors[cycle].second += temp[i].second;
                }
                errors[cycle].second /= numTestSamples;

                if(VISUALIZE) cout << "> Testing done... " << endl << endl;
                else if(Console::SHOW_PROGRESS) {
                    Console::clear_line();
                    Console::cursor_up();
                    Console::clear_line();
                }

                //update gain term and neighbourhood width
                gainTerm = khNetwork->calcGainTerm(INITIAL_LEARNING_RATE, cycle, maxIterations);
                functWidth = khNetwork->calcNewWidth(INITIAL_NEIGHBOURHOOD_WIDTH, cycle, maxIterations);

                if(cycle != maxIterations-1 && cycle % VISUALIZATION_ITERATION_INTERVAL == 0) Console::ring_bell();
            }
            //label the output neurons
            if(Console::SHOW_PROGRESS){
                Console::clear_line();
            }
            //Label map
            pre_lvq_labels = khNetwork->label(test_inputs, test_outputs, numTestSamples);
            if(Console::SHOW_PROGRESS){
                Console::clear_line();
                Console::create_progressbar(progressbar_length);
            }
            //Fine tune (LVQ) labels
            khNetwork->fineTune(INITIAL_LEARNING_RATE, pre_lvq_labels, train_inputs, train_outputs, numTrainSamples, numTargetOutputs);
            if(Console::SHOW_PROGRESS){
                Console::clear_line();
            }
            //Label again
            labels = khNetwork->label(test_inputs, test_outputs, numTestSamples);
            if(Console::SHOW_PROGRESS){
                Console::clear_line();
                Console::create_progressbar(progressbar_length);
            }
            //Run an evaluation
            pair<pair<int, int>, double>* evaluation = khNetwork->evaluateNetwork(test_inputs, numTestSamples);
            if(Console::SHOW_PROGRESS) {
                Console::clear_line();
            }
            double evaluation_error = 0;
            for(int i=0; i<numTestSamples; i++){
                evaluation_error += evaluation[i].second;
            }
            evaluation_error /= numTestSamples;
            
            cout << endl << "-> Post training error: " << errors[maxIterations-1].second << endl;
            cout << "-> Post LVQ Evaluation error: " << evaluation_error << endl;
        }

        void writeResults(){
            //Store reults in files
            stringstream out_errors;
            out_errors.precision(4);
            for(int i=0; i<maxIterations; i++){
                out_errors << fixed << (i+1) << "\t" << errors[i].first << "\t" << errors[i].second << endl;
            }
            writeFile(params["errors_out"], out_errors.str());
            
            int numTargetOutputs = stoi(params["numOutputNeurons"]);
            stringstream out_pre_labels, out_labels;
            for(int y=0; y<khNetwork->MAP_HEIGHT; y++){
                for(int x=0; x<khNetwork->MAP_WIDTH; x++){
                    out_pre_labels << getLabelIndex('A' + pre_lvq_labels[y][x]->first, numTargetOutputs) << "\t";
                    out_labels << getLabelIndex('A' + labels[y][x]->first, numTargetOutputs) << "\t";
                }   
                out_pre_labels << endl;
                out_labels << endl;
            }
            writeFile(params["labels_prelvq_out"], out_pre_labels.str());
            writeFile(params["labels_out"], out_labels.str());
        }

        void readDataSetsFromFiles(){
            Routine::readDataSetsFromFiles(khNetwork->INPUT_VECTOR_SIZE, stoi(params["numOutputNeurons"]));
        }
};

#ifndef runner_cpp
int main(){
    Console::SHOW_PROGRESS = true;
    cout << "khRoutine.cpp" << endl;

    bool createDatasets = true;

    KohonenRoutine* routine = new KohonenRoutine("parameters.txt", ' ');
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