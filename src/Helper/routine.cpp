#include <unordered_map>

#include "helper.cpp"
#include "neuralNetwork.h"

using namespace std;

class Routine{
    protected:
        NeuralNetwork* network;

    public:
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