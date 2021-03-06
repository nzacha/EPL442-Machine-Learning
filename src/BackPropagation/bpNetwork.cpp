#include "../Helper/neuralNetwork.h"

//The network representation in neuron layers
class BackPropagationNetwork : public NeuralNetwork{
    public:
        int numInputNeurons, numOutputNeurons, numHiddenLayers;
        int* hiddenLayerSizes;

        bool MAX_VAL_ENCODING = true;

        NeuronLayer* inputLayer;
        NeuronLayer** neuronLayers;
        NeuronLayer* outputs;
        Bias* bias;

        //Creates a neural network with the specified parameters
        //Receives the number of input neurons, the number of output neurons and the number of hidden layers as well as the number of
        //neurons of each layer
        BackPropagationNetwork(int numInputNeurons, int numOutputNeurons, int numHiddenLayers, int* hiddenLayerSizes){
            this->numInputNeurons = numInputNeurons;
            this->numOutputNeurons = numOutputNeurons;
            this->numHiddenLayers = numHiddenLayers;
            this->hiddenLayerSizes = hiddenLayerSizes;

            //init rand seed
            srand(time(NULL));

            //create network
            //create bias
            this->bias = new Bias("Bias");

            //create inputs
            inputLayer = new InputLayer("input", numInputNeurons);

            //create hidden layers
            neuronLayers = new NeuronLayer*[numHiddenLayers];
            for(int i=0; i<numHiddenLayers; i++){
                string name = "layer" + to_string(i);
                neuronLayers[i] = new NeuronLayer(name, hiddenLayerSizes[i]);
            }

            //create output layer
            outputs = new NeuronLayer("output", numOutputNeurons);

            //add connections from input to 1st hidden layer (input->1)
            for(int i=0; i<numInputNeurons; i++){
                for (int j=0; j<hiddenLayerSizes[0]; j++){
                    addConnections((Node*) inputLayer->getNeuron(i), (Node*) neuronLayers[0]->getNeuron(j), ((double) rand() / (RAND_MAX))*2 -1 );
                }
            }
            
            //add connection between hidden layers (1..n-1)
            for(int i=0; i<(numHiddenLayers-1); i++){
                for(int j=0; j<hiddenLayerSizes[i]; j++){
                    for(int k=0; k<hiddenLayerSizes[i+1]; k++){
                        addConnections((Node*) neuronLayers[i]->getNeuron(j), (Node*) neuronLayers[i+1]->getNeuron(k), ((double) rand() / (RAND_MAX))*2 -1 );
                    }
                }
            }
            //connect bias to hidden layers (1..n)
            for(int i=0; i<(numHiddenLayers); i++){
                for(int j=0; j<hiddenLayerSizes[i]; j++){
                    bias->connectNode(((Node*) neuronLayers[i]->getNeuron(j)));
                }
            }

            //add connections from last hidden layer to output (n->output)
            for(int i=0; i<numOutputNeurons; i++){
                for (int j=0; j<hiddenLayerSizes[numHiddenLayers-1]; j++){
                    addConnections((Node*) neuronLayers[numHiddenLayers-1]->getNeuron(j), (Node*) outputs->getNeuron(i), ((double) rand() / (RAND_MAX))*2 -1 );
                }
                bias->connectNode((Node*) outputs->getNeuron(i));
            }
        }

        //Simulates the forward pass, which calculates the output of each neuron in the network
        //Receives the input values for the simulation
        void simulate(int* inputs){
            //set input values
            for(int i=0; i<numInputNeurons; i++){
                inputLayer->getNeuron(i)->value = inputs[i];
            }
            //compute neuron layers forward pass
            for(int i=0; i<(numHiddenLayers); i++){
                for(int j=0; j<hiddenLayerSizes[i]; j++){
                    neuronLayers[i]->getNeuron(j)->computeNewValue();
                }
            }

            //compute outputs forward pass
            for(int i=0; i<numOutputNeurons; i++){
                outputs->getNeuron(i)->computeNewValue();
            }                   
        }

        //Trains the network for 1 epoch (full training set)
        //Receives learning rate, momentum, number of samples in training set, and the actual inputs and target outputs of the training set
        //Returns a pair of values containing the error value and the accuracy percentage
        pair<double, double> trainNetwork(float learningRate, float momentum, int numInputs, int** inputs, int** targetOutputs){
            float errors = 0;
            int successes = 0;
            for(int inputIndex=0; inputIndex < numInputs; inputIndex++){
                if(VISUALIZE_ALL || VISUALIZE){ 
                    cout << "\t> Training: ";
                    for(int i=0; i<numInputNeurons; i++){
                        cout << inputs[inputIndex][i] << " ";
                    }
                    cout << "=> ";
                    for(int i=0; i<numOutputNeurons; i++){
                        cout << targetOutputs[inputIndex][i] << " ";
                    }
                    cout << endl;
                }else if(Console::SHOW_PROGRESS){
                    Console::clear_line();
                    cout << "\t> Training data... " << flush;
                    Console::update_progressbar(inputIndex+1, numInputs);
                }

                //forward pass            
                if (VISUALIZE_ALL) cout << "\t> Forward pass..." << endl;
                simulate(inputs[inputIndex]);
                
                if (VISUALIZE_ALL) cout << "\t> Backward pass (delta calculation)... " << endl;
                int max;
                double maxValue = 0;
                //set output values and compute backward pass
                for(int i=0; i<numOutputNeurons; i++){
                    Output* output = (Output*) outputs->getNeuron(i);
                    output->computeDelta(targetOutputs[inputIndex][i]);
                    
                    if(MAX_VAL_ENCODING){
                        if(output->value > maxValue) {
                            maxValue = output->value;
                            max = i;
                        }
                    }else{
                        //accumulate cycle succsses
                        max = (output->value >= 0.5) ? 1 : 0;
                        if(max == targetOutputs[inputIndex][i])
                            successes++;
                    }
                    //cout << output->value << "=" << targetOutputs[inputIndex][i] << endl;
                    //accumulate cycle error
                    errors += output->calculateError(targetOutputs[inputIndex][i]);
                }
                if(targetOutputs[inputIndex][max] == 1) successes++;
                //cout << "------------------------" << endl;

                //compute neuron layers backward pass
                for(int i=numHiddenLayers-1; i>=0; i--){
                    for(int j=0; j<hiddenLayerSizes[i]; j++){
                        neuronLayers[i]->getNeuron(j)->computeDelta();
                    }
                }

                //stage 2
                if (VISUALIZE_ALL) cout << "\t> Backward pass (new weight calculation)... " << endl;
                //compute new weights from neuron layers neurons
                for(int i=0; i<(numHiddenLayers); i++){
                    for(int j=0; j<hiddenLayerSizes[i]; j++){
                        neuronLayers[i]->getNeuron(j)->computeNewWeights(learningRate, momentum);
                    }
                }

                //compute new weights from outputs
                for(int i=0; i<numOutputNeurons; i++){
                    Output* output = ((Output*)outputs->getNeuron(i));
                    output->computeNewWeights(learningRate, momentum);
                }
            }
            
            pair<double, double> ret;
            ret.first = (successes * 1.0) / numInputs * 100;
            if(!MAX_VAL_ENCODING) ret.first /= numOutputNeurons;
            ret.second = (errors / numOutputNeurons) / numInputs;
            return ret;
        }

        //Evaluates the network using the test set
        //Receives the number of samples in the test set, and the actual inputs and target outputs of the test set
        //Returns a pair of values containg the error value and the accuracy percentage
        pair<double, double> evaluateNetwork( int numData, int** inputs, int** outputs){
            int count = 0;
            double error;
            for(int input=0; input<numData; input++){
                if(VISUALIZE) cout << "\t> Evaluating: ";
                else if(Console::SHOW_PROGRESS) {
                    Console::clear_line();
                    cout << "\t> Testing data... " << flush;
                    Console::update_progressbar(input+1, numData);
                }

                for(int i=0; i<numInputNeurons-1; i++) 
                    if(VISUALIZE) cout << inputs[input][i] << ", ";
                if(VISUALIZE) cout << inputs[input][numInputNeurons-1] << " => ";

                simulate(inputs[input]);
                
                int max;
                double maxValue =0;
                bool retVal = true;
                for(int i=0; i<numOutputNeurons; i++){
                    double res = (double) ((Output*) this->outputs->getNeuron(i))->value;
                    
                    if(MAX_VAL_ENCODING){
                        if(res > maxValue){
                            maxValue = this->outputs->getNeuron(i)->value;
                            max = i;
                        }
                    }else{
                        //accumulate cycle successes
                        max = (res >= 0.5) ? 1 : 0;
                        if(max == outputs[input][i])
                            count++;
                    }

                    error += ((Output*)this->outputs->getNeuron(i))->calculateError(outputs[input][i]);
                    if(VISUALIZE) 
                        if(i == numOutputNeurons-1){
                            printf("%.3f%% = %d", res, max);
                        }else{
                            printf("%.3f%% = %d, ", res, max); 
                        }
                }
                if(outputs[input][max]==1) count++;
                if(VISUALIZE) cout << endl;
            }

            pair<double, double> ret;
            ret.first = (count * 1.0) / numData * 100;
            if(!MAX_VAL_ENCODING) ret.first /= numOutputNeurons;
            ret.second = (error / numOutputNeurons) / numData;
            return ret;
        }

        //Prints the network values (for Debugging)
        void printNetwork(){
            cout << "Printing connections..." << endl;
            cout << bias->to_string() << endl;

            cout << inputLayer->to_string() << endl;

            for(int i=0; i<numHiddenLayers; i++){
                cout << neuronLayers[i]->to_string() << endl;
            }

            cout << outputs->to_string() << endl;
        }

        //Prints the network structure (for Debugging)
        void printNeurons(){
            cout << "Printing neurons..." << endl;
            cout << "Bias: " << bias->value << endl;
            cout << "Input layer: " << endl;
            for(int i=0; i<numInputNeurons; i++){
                cout << "\tInput " << i << ": " << inputLayer->getNeuron(i)->value << endl;
            }
            cout << endl;

            for(int i=0; i<numHiddenLayers; i++){
                cout << "Hidden layer: " << to_string(i) << endl;
                for(int j=0; j<hiddenLayerSizes[i]; j++){
                    cout << "\tNode " << j << ": " << neuronLayers[i]->getNeuron(j)->value << endl;
                }
            }
            cout << endl;

            cout << "Output layer: " << endl;
            for(int i=0; i<numOutputNeurons; i++){
                cout << "\tOutput " << i << ": " << outputs->getNeuron(i)->value << endl;
            }
            cout << endl;
        }
};