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

        //iniitalizes global variables from 'parameters.txt' file
        void init(){
            int input_vector_size = stoi(params["numInputNeurons"]);
            int map_width = stoi(params["mapWidth"]);
            int map_height = stoi(params["mapHeight"]);
            khNetwork = new KohonenNetwork(input_vector_size, map_width, map_height);
            network = khNetwork;

            this->numInputNeurons = stoi(params["numInputNeurons"]);
            this->numOutputNeurons = stoi(params["numOutputNeurons"]);
            this->maxIterations = stoi(params["maxIterations"]);
            this->INITIAL_LEARNING_RATE = stod(params["learningRate"]);
            this->INITIAL_NEIGHBOURHOOD_WIDTH = stod(params["neighbourhoodWidth"]);
        }

    public:
        //Creates a new Kohonen Routine from a parameters file with given delimeter
        KohonenRoutine(string fileName, char delimeter): Routine(fileName, delimeter){
            init();
        }

        //Creates a new Kohonen Routine from given paramaters in map form
        KohonenRoutine(unordered_map<string, string> params): Routine(params){
            init();
        }
        
        //returns the label ('1') from a list of zeroes ('0')
        int getLabelIndex(int* targetOutputs, int size){
            for(int i=0; i<size; i++){
                if(targetOutputs[i] == 1) return i;
            }
            return -1;
        }
        
        
        /* 
         * Trains and Tests network repeateadly for 'maxIterations'.
         * Logs error of each iteration.
         * At the end of 'maxIteration' cycles then labeling ocurrs,
         * after than fine-tuning (LVQ) ocurrs and after that labeling
         * happens once again to check if LVQ had an effect on the results
         */
        pair<int*, double*> ***pre_lvq_labels,  ***labels;
        pair<double, double>* errors;
        void run_routine(){
            double** normalized_train_inputs = khNetwork->normalizeInputs(train_inputs, numTrainSamples);
            double** normalized_test_inputs = khNetwork->normalizeInputs(train_inputs, numTestSamples);
            
            errors = new pair< double,double >[maxIterations + 1];
            pair< pair<int,int>,double >* temp;
            pair< pair< pair<int,int>,double >*, double>* ret_temp;
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
                cout << cycleTracker->get_tracked_time(maxIterations-cycle) << endl;
                    
                if (VISUALIZE) cout << endl << "> Training network:" << endl;
                else if(Console::SHOW_PROGRESS) {
                    Console::create_progressbar(progressbar_length);
                }

                //Train network
                ret_temp = khNetwork->trainNetwork(normalized_train_inputs, numTrainSamples, gainTerm, functWidth);    
                temp = ret_temp->first;
                errors[cycle].first = ret_temp->second/numTrainSamples;

                if(VISUALIZE) cout << "> Training done... " << endl << endl;

                if(VISUALIZE) cout << "> Testing network: " << endl;
                else if(Console::SHOW_PROGRESS){
                    Console::clear_line();
                    Console::create_progressbar(progressbar_length);
                }

                //Test network
                ret_temp = khNetwork->evaluateNetwork(normalized_test_inputs, numTestSamples);
                temp = ret_temp->first;
                errors[cycle].second = ret_temp->second/numTestSamples;

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

            if(Console::SHOW_PROGRESS){
                Console::clear_line();
            }
            
            //label the output neurons
            cout << "-> Labeling Nodes" << endl;
            pre_lvq_labels = khNetwork->label(test_inputs, test_outputs, numTestSamples);

            //Fine tune (LVQ) labels
            khNetwork->fineTune(INITIAL_LEARNING_RATE/2, pre_lvq_labels, train_inputs, train_outputs, numTrainSamples, numTargetOutputs);
            if(Console::SHOW_PROGRESS){
                Console::clear_line();
            }
            
            //Label again
            cout << "-> Labeling Nodes Again" << endl;
            labels = khNetwork->label(test_inputs, test_outputs, numTestSamples);

            //Run an evaluation to see resulting errors
            pair<pair< pair<int,int>, double>*, double>*  evaluation = khNetwork->evaluateNetwork(normalized_test_inputs, numTestSamples);
            if(Console::SHOW_PROGRESS) {
                Console::clear_line();
            }
            errors[maxIterations] = make_pair(0.0, evaluation->second/numTestSamples);

            cout << endl << "-> Post training error: " << errors[maxIterations-1].second << endl;
            cout << "-> Post LVQ Evaluation error: " << errors[maxIterations].second << endl;
        }

        /*
         * Writes logged data into files
         */
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
                    out_pre_labels << static_cast<char>('A' + getLabelIndex(pre_lvq_labels[y][x]->first, numTargetOutputs)) << "\t";
                    out_labels << static_cast<char>('A' + getLabelIndex(labels[y][x]->first, numTargetOutputs)) << "\t";
                }   
                out_pre_labels << endl;
                out_labels << endl;
            }
            writeFile(params["labels_prelvq_out"], out_pre_labels.str());
            writeFile(params["labels_out"], out_labels.str());
        }

        /*
         * Reads the data set, creating the input and output dataset with given 'ratio' parameter
         * and given input set and output set file names.
         */
        void readDataSetsFromFiles(){
            Routine::readDataSetsFromFiles(numInputNeurons, numOutputNeurons);
        }
};