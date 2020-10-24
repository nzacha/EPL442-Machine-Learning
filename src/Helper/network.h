#include <list>
#include <stdlib.h>
#include <iostream>
#include <sstream>

#include "node.h"

using namespace std;

//Representation of the neuron layer
//Contains neuron list
class NeuronLayer{
    public:
        int size;
        string name; 
        Node** neurons;

        NeuronLayer(string name){
            this->name = name;
        }

        NeuronLayer(string name, int size): NeuronLayer(name){
            this->size = size;
            neurons = new Node*[size];

            for(int i=0; i<size; i++){
                string neuronName = name + "-" + std::to_string(i);
                neurons[i] = new Neuron(neuronName);
            }
        }

        //Returns the neuron at specified index
        Neuron* getNeuron(int index){
            return (Neuron*)neurons[index];
        }

        string to_string() { 
            ostringstream s;
            s << "NeuronLayer " << this->name <<  ": [" << endl;
            for(int i=0; i<size; i++)
                s << "\t" << neurons[i]->to_string() << endl;
            s << "]" << endl;
            return s.str();
        }
};

//Represents an input layer, which is a neuron layer
//Contains a list of input neurons
class InputLayer : public NeuronLayer{
    public: 
        InputLayer(string name, int size): NeuronLayer(name){
            this->name = name;
            this->size = size;
            neurons = new Node*[size];
            for(int i=0; i<size; i++){
                string neuronName = name + "-" + std::to_string(i);
                neurons[i] = new Input(neuronName);
            }
        }  
};
