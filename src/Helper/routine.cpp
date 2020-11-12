#ifndef routine_cpp
#define routine_cpp

#include <unordered_map>

#include "helper.cpp"
#include "neuralNetwork.h"

using namespace std;

class Routine{
    protected:
        NeuralNetwork* network;

    public:
        int maxIterations, numInputNeurons, numOutputNeurons;
        string required_params[12] = {"maxIterations", "numInputNeurons", "numOutputNeurons", "numHiddenLayers", "learningRate", "momentum", "train_test_ratio", "progressbar_length", "trainFile", "testFile", "errors_out", "accuracy_out"};
        unordered_map<string,string> params;
        
        Routine(string params_in, char delimeter){
            this->params = readParameters("parameters.txt", delimeter);
        }

        Routine(unordered_map<string, string> params){
            this->params = params;
        }

        virtual void run_routine() = 0; 
        virtual void writeResults() = 0;

        void writeResults(string error_out, string accuracy_out, string error_content, string accuracy_content){
            cout << "Writing error output to: " << error_out << endl;
            writeFile(error_out, error_content);
            cout << "Writing accuracy output to: " << accuracy_out << endl;
            writeFile(accuracy_out, accuracy_content);
        }

        virtual void readDataSetsFromFiles() = 0;

        void handleMissingParameter(string parameter){
            cout << "Parameter: \"" << parameter << "\" was not defined, program exiting..." << endl;
            exit(0);
        }
        void checkParameterExists(string parameter){
            if(params.find(parameter) == params.end()) handleMissingParameter(parameter);
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

        void readUniformDataSet(vector<string> labels){
            checkParameterExists("datasetFile");
            unordered_map<string, vector<vector<string>>> data = readLabeledVarSizeData(params["datasetFile"], ',');
            vector<pair<string, vector<string>>> trainset, testset;
            
            cout << "-> Creating train set and test set files from uniform, labeled dataset (" << params["datasetFile"] << ") -> (" << params["trainFile"] << "), (" << params["testFile"] << ") ..." << endl;  
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

        int **train_inputs, **train_outputs;
        int numTrainSamples;
        int **test_inputs, **test_outputs;
        int numTestSamples;
        void readDataSetsFromFiles(int numInputs, int numOutputs){
            cout << "-> Reading train set and test set from files (" << params["trainFile"] << "), (" << params["testFile"] << ") ..." << endl;
            //Mapping training data from file
            vector<vector<string>> train_data = readFixedSizeData(params["trainFile"], numInputs + numOutputs, ' ');
            numTrainSamples = train_data.size();
            train_inputs =  new int*[numTrainSamples];
            train_outputs =  new int*[numTrainSamples];
            network->setDataset(train_data, &train_inputs, &train_outputs, numInputs, numOutputs, numTrainSamples);

            //Mapping test data from file
            vector<vector<string>> test_data = readFixedSizeData(params["testFile"], numInputs + numOutputs, ' ');
            numTestSamples = test_data.size();
            test_inputs = new int*[numTestSamples];
            test_outputs = new int*[numTestSamples];
            network->setDataset(test_data, &test_inputs, &test_outputs, numInputs, numOutputs, numTestSamples);
        }
};
#endif