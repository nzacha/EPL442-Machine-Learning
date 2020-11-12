#ifndef neuralNetwork_h
#define neuralNetwork_h

#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <time.h>
#include <unordered_map>
#include <vector>

#include "neuronLayer.h"
#include "console.h"

#define VISUALIZE false
#define VISUALIZE_ALL false
#define VISUALIZATION_ITERATION_INTERVAL 20 

using namespace std;

//Connects 2 nodes with specified weight
Connection* addConnections(Node* leftNode, Node* rightNode, float weight){
    Connection* connection = new Connection(leftNode, rightNode, weight);
    leftNode->addRightPair(connection);
    rightNode->addLeftPair(connection);
    return connection;
}

class NeuralNetwork{
    public:
        //Prints the network values (for Debugging)
        virtual void printNetwork()=0;
        //Prints the network structure (for Debugging)
        virtual void printNeurons()=0;

        //Fills input and output values retrieved from unlabeled map
        void setDataset(vector<vector<string>> dataValues, int*** inputValues, int*** outputValues, int numInputNeurons, int numOutputNeurons, int numInputs){
            int** inputs = *(inputValues);
            int** outputs = *(outputValues);
            for(int i=0; i<numInputs; i++){
                inputs[i] = new int[numInputNeurons];
                outputs[i] = new int[numOutputNeurons];
                vector<string> data = dataValues.at(i);
                for(int j=0; j<numInputNeurons; j++){
                    inputs[i][j] = stoi(data.at(j));
                }
                for(int j=numInputNeurons; j<numInputNeurons + numOutputNeurons; j++){
                    outputs[i][j-numInputNeurons] = stoi(data.at(j));
                }
            }
        }
};
#endif