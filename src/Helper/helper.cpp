#ifndef helper_cpp
#define helper_cpp

#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#define DEBUG false

//removes new line character from string
void rm_nl(string &s) {
    bool changed = false;
    for(int i=0;i<s.length();i++) {
        if(s[i]=='\n' || s[i]=='\r'){
            s.erase(s.begin()+i);
            changed = true;
            break;
        }
    }
    if(changed) rm_nl(s);
}

//Creates a map of values containing the name of the parameter and its value
//Receives the file name of the parameters file and the delimeter separating the values
//Returns the map of values
unordered_map<string, string> readParameters(string fileName, char delimeter){
    ifstream myfile (fileName);
    stringstream ss;
    string line;
    //read file into stringstream
    if (myfile.is_open()){
        while (getline(myfile,line)){
            ss << line << endl;
        }
        myfile.close();
    } else {
        cout << "Unable to open file " << fileName << endl;
        cout << "Exiting..." << endl;
        exit(0);
    }

    unordered_map<string, string> umap;
    string param, value;
    //loop stringtream to build parameter map
    while(getline(ss, param, delimeter) && getline(ss, value, (char)'\n')){
        rm_nl(param);
        rm_nl(value);
        umap[param] = value;
    }

    if(DEBUG) {
        for (auto i : umap){
            cout << i.first << ": " << i.second << endl;
        }
    }
    return umap;
}

//Creates a map of values containing arrays of data
//Receives the file name, and a delimeter
//Each data entry must be labeled in the beggining
//Returns a map of values
unordered_map<string, vector<vector<string>>> readLabeledVarSizeData(string fileName, char delimeter){
    ifstream myfile (fileName);
    stringstream ss;
    string line, param, value;
    
    unordered_map<string, vector<vector<string>>> umap;
    //read file
    if (myfile.is_open()){
        while (getline(myfile, line)){
            //put line in stringstream
            ss << line << endl;

            //read first split of delimeter which is the parameter
            getline(ss, param, delimeter);
            rm_nl(param);
            vector<string> values;
            //split stringstream with given delimeter, these are the values
            while(getline(ss, value, delimeter)){
                rm_nl(value);
                values.push_back(value);
            }

            umap[param].push_back(values);
            //clear line buffer / stringstream
            ss.clear();
        }
        myfile.close();
    } else {
        cout << "Unable to open file " << fileName << endl;
        cout << "Exiting..." << endl;
        exit(0);
    }

    if(DEBUG) {
        for (auto i : umap){
            cout << i.first << ": \t";
            for (auto j : i.second){
                for (auto k : j)
                    cout << k << " ";
                cout << endl << "\t";
            }
            cout << endl;
        }
    }
    return umap;
}

//Creates a map of values containg data
//Receives the file name, the number of expected data and the delimeter separating the values
//Returns the map of values
vector<vector<string>> readFixedSizeData(string fileName, int row_size, char delimeter){
    ifstream myfile (fileName);
    stringstream ss;
    string line;
    
    int dataLength = 0;
    //read file into stringstream
    if (myfile.is_open()){
        while (getline(myfile,line)){
            ss << line << endl;
            dataLength++;
        }
        myfile.close();
    } else {
        cout << "Unable to open file " << fileName << endl;
        cout << "Exiting..." << endl;
        exit(0);
    }

    string value;
    vector<vector<string>> data;
    //loop stringtream to build parameter map
    for(int line=0; line<dataLength; line++){
        vector<string> row;
        for(int i=0; i<row_size-1; i++){
            getline(ss, value, delimeter);
            rm_nl(value);
            row.push_back(value);
        }
        getline(ss, value, '\n');
        rm_nl(value);
        row.push_back(value);
        data.push_back(row);
    }

    if(DEBUG) {
        for (auto i : data){
            for (auto j : i)
                cout << j << " ";
            cout << endl;
        }
    }

    return data;
}

//Writes a file with specified name and content
void writeFile(string fileName, string content){
    ofstream myfile (fileName);
    myfile << content;
    myfile.close();
}

#ifdef testing 
int main(){
    //cout << "reading parameters..." << endl << endl;
    //unordered_map<string, string> params = readParameters("parameters.txt", ' ');
    
    //cout << "reading var size set..." << endl << endl;
    unordered_map<string, vector<vector<string>>> varset = readLabeledVarSizeData("test.txt", ',');
    
    //cout << "reading fixed size set..." << endl << endl;
    //unordered_map<string, vector<string>> fixedset = readFixedSizeData("test.txt", 16, ',');
    
    vector<vector<int>> trainData;
    vector<vector<int>> testData;
    retrieveLabeledDataSet(varset, "D", 0.75f, &trainData, &testData);
   
    return 0;
}
#endif
#endif