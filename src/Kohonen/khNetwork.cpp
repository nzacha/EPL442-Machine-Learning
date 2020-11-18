    #include "../Helper/neuralNetwork.h"

    class ConnectionMapper{
        public:
            /*
             * Contains 3 dimensions of (Connection*)
             * The first contains the input vectors
             * The next 2 dimensions is the 2-D array of output nodes
             */
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

            //Sets the specified connection between input node, and output node (x,y)
            void setConnection(int input, int x, int y, Connection* conn){
                connections[input][y][x] = conn;
            }

            //Gets the connection between specified input node, and output node (x,y)
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

                //connect inputs to neurons - create connection mapper
                connections = new ConnectionMapper(inputLayer, neuronMap, INPUT_VECTOR_SIZE, MAP_WIDTH, MAP_HEIGHT); 
            }
            
            //Calculates the new gain term, based on given initial vlaue, current cycle and maxCycles
            double calcGainTerm(double initialGainTerm, int cycle, int maxCycles){
                return (initialGainTerm * (1.0 - (cycle * 1.0 / maxCycles)));
            }

            //Calculates the new width, based on given initial vlaue, current cycle and maxCycles
            double calcNewWidth(double initialWidth,int cycle, int maxCycles){
                return (initialWidth / exp(cycle * 1.0 / maxCycles));
            }

            //Returns the EuclideanDistance between 2 points, (x1,y1) and (x2,y2)
            int euclidianDistance(int x1, int y1, int x2, int y2){
                return sqrt(pow(x1-x2, 2) + pow(y1-y2, 2));
            }

            //calculates the neighbourhood size
            double neighbourhoodFuct(int x1, int y1, int x2, int y2, double funcWidth){
                return exp(- (pow(euclidianDistance(x1,y1,x2,y2), 2) / (2 * pow(funcWidth,2))));
                //return 1.0 / (funcWidth - euclidianDistance(x1,y1,x2,y2));
            }

            /*
             * Updates the weights of a single connection between specified input node, and output node (x,y)
             * The weight is calculated from given value, gain_term and neighbourhood function value
             */
            void updateSingleWeight(int input, int x, int y, double value, double gain_term, double hci, bool additive){
                if(additive)
                    connections->getConnection(input,x,y)->weight += gain_term * hci * (value - connections->getConnection(input, x,y)->weight);
                else
                    connections->getConnection(input,x,y)->weight -= gain_term * hci * (value - connections->getConnection(input, x,y)->weight);
            }

            /*
             * Updates values of all nodes in the map, based on their distance from the winner (x,y),
             * the given values, gain term and neighbourhood function width
             */
            void updateValues(double* values, double gain_term, int winner_x, int winner_y, double funcWidth){
                for(int y=0; y<MAP_HEIGHT; y++){
                    for(int x=0; x<MAP_WIDTH; x++){
                        double hci = neighbourhoodFuct(x,y,winner_x,winner_y, funcWidth);
                        for(int i=0; i<INPUT_VECTOR_SIZE; i++){
                            updateSingleWeight(i, x, y, values[i], gain_term, hci, true);
                        }
                    }
                }
            }
 
            /*
             * Normalizes a list of int inputs of given size into a list of doubles in the range of [0, 1]
             * With given minimum and maximum values
             */
            double* normalize(int* inputs, int size, int min, int max){
                double* normalized = new double[size];
                for(int i=0; i<size; i++){
                    normalized[i] = ((inputs[i]-min)*1.0 / (max-min));
                }
                return normalized;
            }

            /*
             * Normalizes a list of int inputs of given size into a list of doubles in the range of [0, 1]
             * With min and max values from the set given
             */
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

            double** normalizeInputs(int** inputs, int numInputs){
                double** retVal = new double*[numInputs];
                for(int i=0; i<numInputs; i++){
                    retVal[i] = normalize(inputs[i], INPUT_VECTOR_SIZE, 0, 15);
                }
                return retVal;
            }

            /*
             * Calculates the distance between normalized_input (given input vector values) and node (x,y)
             */
            double distanceVectorNode(double* normalized_input, int x, int y){
                double d=0;
                for(int i=0; i<INPUT_VECTOR_SIZE; i++){
                    d+= pow(normalized_input[i] - connections->getConnection(i,x,y)->weight ,2);
                }
                return d;
            }

            /*
             * Calculates winner (x,y) for given normalized inputs
             */
            pair< pair<int,int>, double> simulate(double* normalized_inputs){
                pair<int,int> winner = make_pair(INT32_MAX,INT32_MAX);
                double distances[MAP_HEIGHT][MAP_WIDTH] = {0};

                for(int i=0; i<INPUT_VECTOR_SIZE; i++){
                    inputLayer->getNeuron(i)->value = normalized_inputs[i];
                }

                double min = __LONG_LONG_MAX__, d;
                Connection* conn;
                //for every neuron calc total distance from inputs
                for(int y=0; y<MAP_HEIGHT; y++){
                    for(int x=0; x<MAP_WIDTH; x++){
                        d = 0;
                        for(int i=0; i<INPUT_VECTOR_SIZE;i++){
                            conn = connections->getConnection(i,x,y);
                            d+= pow(conn->left->value - conn->weight ,2);
                        }
                        if(d < min){
                            min = d;
                            winner.first = x;
                            winner.second = y;
                        }
                    }
                }
                
                //cout << fixed << "winner: " << winner.first << "," << winner.second<< " -  ditance: " << min <<endl;
                return make_pair(winner,min);
            }

            /*
             * Trains network with given inputs, initial gain term neighhbourhood width
             * returns a pair of values containing the winners and the error
             */
            pair<pair< pair<int,int>, double>*, double>* trainNetwork(double** inputs, int inputSize, double gainTerm, double funcWidth){                
                pair< pair<int,int>, double>* winners = new pair< pair<int,int>, double>[inputSize];
                pair< pair< pair<int,int>, double>*, double>* retVal = new pair< pair< pair<int,int>, double>*, double>(winners, 0);
                for(int input=0; input<inputSize; input++){
                    if(Console::SHOW_PROGRESS){
                        Console::clear_line();
                        cout << "\t> Training data... " << flush;
                        Console::update_progressbar(input+1, inputSize);
                    }
                    //find winner node for specified input vector
                    winners[input] = simulate(inputs[input]);
                    retVal->second += winners[input].second;
                    //update values for winner
                    updateValues(inputs[input], gainTerm, winners[input].first.first, winners[input].first.second, funcWidth);
                } 

                //for(int i=0; i<inputSize; i++)
                //cout << fixed << "winner: " << winners[i].first.first << "," << winners[i].first.second<< " -  ditance: " << winners[i].second << endl;
                return retVal;
            }

            /*
             * Tests network with given inputs
             * returns a pair of values containing the winners and the error
             */
            pair<pair< pair<int,int>, double>*, double>*  evaluateNetwork(double** inputs, int inputSize){
                pair< pair<int,int>, double>* winners = new pair< pair<int,int>, double>[inputSize];;
                pair< pair< pair<int,int>, double>*, double>* retVal = new pair< pair< pair<int,int>, double>*, double>(winners, 0);
                for(int input=0; input<inputSize; input++){
                    if(Console::SHOW_PROGRESS){
                        Console::clear_line();
                        cout << "\t> Testing data... " << flush;
                        Console::update_progressbar(input+1, inputSize);
                    }

                    //find winner node for specified input vector
                    winners[input] = simulate(inputs[input]);
                    retVal->second += winners[input].second;
                }
                return retVal;
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
                            d = distanceVectorNode(normalized_data, x, y);
                            if(minDistances[y][x] > d){
                                minDistances[y][x] = d;
                                labels[y][x]->first = targetOutputs[i];
                                labels[y][x]->second = normalized_data;
                            }
                        }
                    }
                }
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

            /*
             * Liner Vector Quantisation, receives a gain term, a map(2D) of labels, the data set, and the target inputs
             * Processes all data and fine-tunes output vectors based on target output
             */
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
                        updateSingleWeight(input, x, y, labels[y][x]->second[input], gainTerm, 1, false);
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

            //Returns the network description as a string
            string networkDescription(){
                stringstream ret;
                ret << "Kohonen SOM network:" << endl;
                ret << "\t- Input Vector Size: " << INPUT_VECTOR_SIZE << endl;
                ret << "\t- Map Height: " << MAP_HEIGHT << endl;
                ret << "\t- Map Width: " << MAP_WIDTH << endl;
                return ret.str();
            }
    };