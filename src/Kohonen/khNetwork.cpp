    #include "../Helper/neuralNetwork.h"

    class ConnectionMapper{
        public:
            //3 dimensions: 1- inputs map to 2-d array of neurons
            Connection**** connections;
            ConnectionMapper(InputLayer* inputs, NeuronLayer** map, int numInputs, int width, int height){
                connections = new Connection***[numInputs];
                for(int input=0; input<numInputs; input++){
                    connections[input] = new Connection**[height];
                    for(int y=0; y<height; y++){
                        connections[input][y] = new Connection*[width];
                        for(int x=0; x<width; x++){
                            connections[input][y][x] = addConnections(inputs->getNeuron(input), map[y]->getNeuron(x), ((double) rand() / (RAND_MAX))*1);
                        }
                    }
                }
            }

            void setConnection(int input, int x, int y, Connection* conn){
                connections[input][y][x] = conn;
            }

            Connection* getConnection(int input, int x, int y){
                return connections[input][y][x];
            }
    };

    //The network representation in neuron layers
    class KohonenNetwork : public NeuralNetwork{
        public:
            InputLayer* inputLayer;
            NeuronLayer** neuronMap;
            int INPUT_VECTOR_SIZE, MAP_HEIGHT, MAP_WIDTH;
            ConnectionMapper* connections;

            KohonenNetwork(int C, int N, int M){
                this-> INPUT_VECTOR_SIZE = C;
                this-> MAP_HEIGHT = N;
                this-> MAP_WIDTH = M;

                //init rand seed
                srand(time(NULL));

                inputLayer = new InputLayer("inputs", C);
                neuronMap = new NeuronLayer*[N];

                //create map of neurons: N neuronLayers of size M
                for(int i=0; i<N; i++){
                    string layerName = "layer-"+to_string(i);
                    neuronMap[i] = new NeuronLayer(layerName, M);
                }

                //connect inputs to neurons
                //create connection mapper
                connections = new ConnectionMapper(inputLayer, neuronMap, INPUT_VECTOR_SIZE, MAP_WIDTH, MAP_HEIGHT); 
            }
            
            double calcGainTerm(double initialGainTerm, int cycle, int maxCycles){
                return (initialGainTerm * (1.0 - (cycle * 1.0 / maxCycles)));
            }

            double calcNewWidth(double initialWidth,int cycle, int maxCycles){
                return (initialWidth / exp(cycle * 1.0 / maxCycles));
            }

            int euclidianDistance(int x1, int y1, int x2, int y2){
                return sqrt(pow(x1-x2, 2) + pow(y1-y2, 2));
            }

            //calculates the neighbourhood size
            double neighbourhoodFuct(int x1, int y1, int x2, int y2, double funcWidth){
                return exp(- (pow(euclidianDistance(x1,y1,x2,y2), 2) / (2 * pow(funcWidth,2))));
                //return 1.0 / (funcWidth - euclidianDistance(x1,y1,x2,y2));
            }

            void updateSingleWeight(int input, int x, int y, double value, double gain_term, double hci){
                double weight = connections->getConnection(input, x,y)->weight;
                double diff = value - weight; 
                double prod = gain_term * hci * diff;
                //if(!additive) prod*=-1;
                connections->getConnection(input,x,y)->weight += prod;
            }

            //for every neighbouring node update values
            void updateValues(double* values, double gain_term, int winner_x, int winner_y, double funcWidth){
                for(int i=0; i<INPUT_VECTOR_SIZE; i++){
                    for(int y=0; y<MAP_HEIGHT; y++){
                        for(int x=0; x<MAP_WIDTH; x++){
                            double hci = neighbourhoodFuct(x,y,winner_x,winner_y, funcWidth);
                            updateSingleWeight(i, x, y, values[i], gain_term, hci);
                        }
                    }
                }
            }

            double* normalize(int* inputs, int size, int min, int max){
                double* normalized = new double[size];
                for(int i=0; i<size; i++){
                    normalized[i] = ((inputs[i]-min)*1.0 / (max-min));
                }
                return normalized;
            }

            double* normalize(int* inputs, int size){
                double* normalized = new double[size];
                int max = INT32_MIN, min = INT32_MAX;
                for(int i=0; i<size; i++){
                    if(inputs[i] > max)
                        max = inputs[i];
                    if(inputs[i] < min)
                        min = inputs[i];
                }
                for(int i=0; i<size; i++){
                    normalized[i] = ((inputs[i]-min)*1.0 / (max-min));
                }
                return normalized;
            }

            double distanceVectorNode(double* normalized_input, int x, int y){
                double d=0;
                for(int i=0; i<INPUT_VECTOR_SIZE; i++){
                    d+= pow(normalized_input[i] - connections->getConnection(i,x,y)->weight ,2);
                }
                return d;
            }

            //for specified normalized input vector find winner node
            pair< pair<int,int>, double> simulate(double* normalized_inputs){
                pair<int,int> winner = make_pair(INT32_MAX,INT32_MAX);
                double distances[MAP_HEIGHT][MAP_WIDTH] = {0};

                for(int i=0; i<INPUT_VECTOR_SIZE; i++){
                    inputLayer->getNeuron(i)->value = normalized_inputs[i];
                }

                //for every neuron calc total distance from inputs
                for(int y=0; y<MAP_HEIGHT; y++){
                    for(int x=0; x<MAP_WIDTH; x++){
                        list<Connection*> node_conns = neuronMap[y]->getNeuron(x)->connsLeft;
                        for(Connection* conn : node_conns){
                            Input* input = (Input*) (conn->left);
                            double d = pow(input->value - conn->weight ,2);
                            if(conn->weight < 0){
                                cout << input->value << " - " << conn->weight << " => " << d << endl;
                                exit(0);
                            }
                            distances[y][x] += d;
                        }
                    }
                }

                double min = __LONG_LONG_MAX__;
                //find winner
                for(int y=0; y<MAP_HEIGHT; y++){
                    for(int x=0; x<MAP_WIDTH; x++){
                        if(distances[y][x] < min){
                            min = distances[y][x];
                            winner.first = x;
                            winner.second = y;
                        }
                    }
                }
                
                //cout << fixed << "winner: " << winner.first << "," << winner.second<< " -  ditance: " << min <<endl;
                return make_pair(winner,min);
            }

            pair< pair<int,int>, double>* trainNetwork(int** inputs, int inputSize, double gainTerm, double funcWidth){                
                pair< pair<int,int>, double>* winners = new pair< pair<int,int>, double>[inputSize];
                double* normalized_inputs;
                for(int input=0; input<inputSize; input++){
                    if(Console::SHOW_PROGRESS){
                        Console::clear_line();
                        cout << "\t> Training data... " << flush;
                        Console::update_progressbar(input+1, inputSize);
                    }
                    normalized_inputs = normalize(inputs[input], INPUT_VECTOR_SIZE);
                    //find winner node for specified input vector
                    winners[input] = simulate(normalized_inputs);
                    
                    //update values for winner
                    updateValues(normalized_inputs, gainTerm, winners[input].first.first, winners[input].first.second, funcWidth);
                } 

                //for(int i=0; i<inputSize; i++)
                //cout << fixed << "winner: " << winners[i].first.first << "," << winners[i].first.second<< " -  ditance: " << winners[i].second << endl;
                return winners;
            }

            pair< pair<int,int>, double>* evaluateNetwork(int** inputs, int inputSize){
                pair< pair<int,int>, double>* winners = new pair< pair<int,int>, double>[inputSize];;
                double* normalized_inputs;
                for(int input=0; input<inputSize; input++){
                    if(Console::SHOW_PROGRESS){
                        Console::clear_line();
                        cout << "\t> Testing data... " << flush;
                        Console::update_progressbar(input+1, inputSize);
                    }

                    normalized_inputs = normalize(inputs[input], INPUT_VECTOR_SIZE);
                    //find winner node for specified input vector
                    winners[input] = simulate(normalized_inputs);
                }
                return winners;
            }

            /*
            * requires a test set, its target outputs and the number of samples in the set
            * returns a 2-D array that represents the label (it's targetOutputs, and its distance from the node) of each node of the map
            */
            pair<int*, double*>*** label(int** data, int** targetOutputs, int numSamples){
                //initialize labels
                //label contains the targetOutputs and the dinstance of the closest sample
                double minDistances[MAP_HEIGHT][MAP_WIDTH];
                pair<int*, double*>*** labels = new pair<int*,double*>**[MAP_HEIGHT];
                for(int y=0; y<MAP_HEIGHT; y++){
                    labels[y] = new pair<int*,double*>*[MAP_WIDTH];
                    for(int x=0; x<MAP_WIDTH; x++){
                        labels[y][x] = new pair<int*,double*>((int*)NULL, (double*)NULL);
                        minDistances[y][x] = __DBL_MAX__;
                    }
                }

                double d;
                double* normalized_data;
                pair< pair<int,int>, double> winner;
                for(int i=0; i<numSamples; i++){
                    normalized_data = normalize(data[i], INPUT_VECTOR_SIZE);
                    for(int y=0; y<MAP_HEIGHT; y++){
                        for(int x=0; x<MAP_WIDTH; x++){
                            if(Console::SHOW_PROGRESS){
                                Console::clear_line();
                                cout << "\t> Labeling node (" << x << "," << y << ")... " << flush;
                            }
                            d = distanceVectorNode(normalized_data, x, y);
                            if(minDistances[y][x] > d){
                                minDistances[y][x] = d;
                                labels[y][x]->first = targetOutputs[i];
                                labels[y][x]->second = normalized_data;
                            }
                        }
                    }
                }
                /*
                for(int y=0; y<MAP_HEIGHT; y++){
                    for(int x=0; x<MAP_WIDTH; x++){
                        cout << minDistances[y][x] <<  " > " << labels[y][x]->first << "|" << labels[y][x]->second << "\t";
                    }
                    cout << endl;
                }*/
                return labels;
            }

            /*
            * Compares an array of '0' and '1' to find which has the first instance of 1
            * Returns 1 for B, -1 for A and 0 for BOTH 
            */
            int compareOutputs(int* A, int* B, int size){ 
                for(int i=0; i<size; i++){
                    if(A[i]==1 && B[i]==1){
                        return 0;
                    }
                    if(A[i]>B[i]){
                        return -1;
                    }
                    if(A[i]<B[i]){
                        return 1;
                    }
                }
                return 0;
            }

            void fineTune(double gainTerm, pair<int*,double*>*** labels, int** data, int** targetOutputs, int numSamples, int numTargetOutputs){
                double* normalized_data;
                pair< pair<int,int>, double> winner;
                for(int i=0; i<numSamples; i++){
                    if(Console::SHOW_PROGRESS){
                        Console::clear_line();
                        cout << "\t> Fine tuning (LVQ)... " << flush;
                        Console::update_progressbar(i+1, numSamples);
                    }
                    normalized_data = normalize(data[i], INPUT_VECTOR_SIZE);
                    winner = simulate(normalized_data);
                    int x = winner.first.first;
                    int y = winner.first.second;
                    if(labels[y][x]->second == NULL){
                        cout << "No label given to node: (" <<x << "," <<y << ")" << endl;
                        exit(0);
                    }
                    for(int input=0; input<INPUT_VECTOR_SIZE; input++)
                        updateSingleWeight(input, x, y, labels[y][x]->second[input], gainTerm, 1);
                }
            }

            //Prints the network values (for Debugging)
            void printNetwork(){
                for(int i=0; i<MAP_HEIGHT; i++){
                    cout << neuronMap[i]->to_string() << endl;
                }
            }

            //Prints the network structure (for Debugging)
            void printNeurons(){
            }

            string networkDescription(){
                stringstream ret;
                ret << "Kohonen SOM network:" << endl;
                ret << "\t- Input Vector Size: " << INPUT_VECTOR_SIZE << endl;
                ret << "\t- Map Height: " << MAP_HEIGHT << endl;
                ret << "\t- Map Width: " << MAP_WIDTH << endl;
                return ret.str();
            }
    };