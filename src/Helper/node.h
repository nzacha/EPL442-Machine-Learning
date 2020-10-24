#include <list>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <math.h>

using namespace std;

#define DEBUG false

class Node;

//Represents a connection between 2 nodes
class Connection{
    public:
        Node *left, *right;
        float weight, prevWeight = 0;
        
        Connection(Node* left, Node* right, float weight){
            this->left = left;
            this->right = right;
            this->weight = weight; 
        }

        string to_string();
};

//Represents a node of the network
class Node{
    public:
        string name;
        float value = 0, learningDelta;
        list <Connection*> connsLeft, connsRight;

        Node(string name){
            this->name = name;
        }

        //Adds a right connection to nodes right connection list
        void addRightPair(Connection* connection){
            connsRight.push_back(connection);
        }

        //Adds a left connection to nodes left connection list
        void addLeftPair(Connection* connection){
            connsLeft.push_back(connection);
        }

        float activationFunction(float value){
            return 1.0/(1 + exp(value));
        }

        //Computes the new output value of the node
        void computeNewValue(){
            std::list<Connection*>::iterator it;
            float sum = 0;
            for (it = connsLeft.begin(); it != connsLeft.end(); ++it){
                sum += (*it)->left->getValue() * (*it)->weight;
                //cout << "Connection '" << (*it)->left->name << " + " << name << "' value is: " << (*it)->weight << endl;
            }
            if (DEBUG) cout << "Neuron '" << name << "' value: " << value << " new value is: " << activationFunction(-sum) << endl;
            value = activationFunction(-sum);
        }

        //Computes the new weight of the value using specified learning rate and momentum values
        void computeNewWeights(float learningRate, float momentum){
            std::list<Connection*>::iterator it;
            for (it = connsLeft.begin(); it != connsLeft.end(); ++it){
                if (DEBUG) cout << "Connection [" << (*it)->left->name << ", " << (*it)->right->name << "] weight: " << (*it)->weight << " is now: " << (*it)->weight - learningRate * (*it)->right->learningDelta * value << endl;     
                (*it)->prevWeight = (*it)->weight;
                (*it)->weight = (*it)->weight - (learningRate * this->learningDelta * (*it)->left->getValue()) + (momentum * ((*it)->weight - (*it)->prevWeight)); 
            }   
        }

        //Returns the output value of the node
        float getValue(){
            return value;
        }

        string to_string() { 
            ostringstream s;
            
            s << "Node: " << name << ": [";
            s << "{left: ";
            for(Connection* conn: connsLeft){
                s << " " << conn->to_string();
            }
            s << "}, ";
            s << "{right: ";
            for(Connection* conn: connsRight){
                s << " " << conn->to_string();
            }
            s << "}]";

            return s.str();
        }
};

inline string Connection::to_string() { 
    ostringstream s;
    s<<"Connection: [left: " << left->name << ", right: " << right->name << ", weight: " << this->weight << "]";
    return s.str();
}

//Represents an input neuron of the network
class Input : public Node{
    public:
        Input(string name):Node(name){
        }
};

//Represend a neron of the network
class Neuron : public Node{
    public:
        Neuron(string name):Node(name){
        }

        //Computes the delta value of the neuron
        void computeDelta(){
            std::list<Connection*>::iterator it;
            float sum = 0;
            for (it = connsRight.begin(); it != connsRight.end(); ++it){
                sum += (*it)->weight * (*it)->right->learningDelta;
            }
            //cout << ", sum: " << sum;
            //cout << ", other: " << value * (1 - value);
            learningDelta = (value * (1 - value) * (sum));
            //cout << ", " << learningDelta << endl;
            if(DEBUG) cout << "Neuron " << name << " learning delta is: " << learningDelta << endl;
        }
};

//Represents an output neuron of the network
class Output : public Node{
    public:
        Output(string name):Node(name){
        }

        //Calculates the error of the output node with specified target output
        double calculateError(int targetOutput){
            return pow(targetOutput - value, 2) /2;
        }

        //Computes the delta value of the output node with specified target output
        void computeDelta(int targetOutput){           
            learningDelta = (value * (1 - value) * (value - targetOutput));
            if (DEBUG) cout << "Output "<< name << " learning delta is: " << learningDelta << " (" << value << ") " << endl;
        }
};

//Represents the bias of the network
class Bias : public Node{
    public:
        Bias(string name):Node(name){
            srand(time(NULL));
            this->value = -1;
        }

        //Connects node to the bias with a unique connection
        void connectNode(Node* node){
            Connection* connection = new Connection((Node*)this, node, ((double) rand() / (RAND_MAX)));
            node->addLeftPair(connection);
            this->addRightPair(connection);
        }
};