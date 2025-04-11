
#include <iostream>
#include <tuple>
#include <vector>
#include <set>
#include <string>
#include <math.h>
#include "BenchMarkParser.h"
#include "QCircuitManager.h"
#include <fstream>
#include<map>
using namespace std;

class Graph{
    private : 
        QuantumCircuit qCircuit;
        vector<vector<int>> gateMatrix;

        //wire cut : qubit1  qubit2  layer1 layer2
        //gate cut : qubit1  qubit2  layer        
        vector<vector<int>> cutGraph;

        //wire cut : qubit1  qubit2  layer1 layer2
        vector<vector<int>> wireCutSet;

        //gate cut : qubit1  qubit2  layer
        vector<vector<int>> gateCutSet;

    public : 
        // constructor empty
        Graph(){};

        
        Graph(int logiNumber ,QuantumCircuit qCircuit){
            // this->gateMatrix = vector<vector<int>>(logiNumber,vector<int>(logiNumber,0));            
            this->qCircuit = qCircuit;
            vector<QuantumGate> gateSet = qCircuit.getGates();
            int layer = qCircuit.getLayers();
            int qubit1 = -1;
            int qubit2 = -1;
            string operation;
        
            // cout<<"Gate Matrix size : "<<gateMatrix.size()<<" "<<gateMatrix[0].size()<<endl;
            for(auto gate : gateSet){
                qubit1 = gate.qubits_used[0];
                qubit2 = gate.qubits_used[1];
                if(qubit1 > qubit2){
                    int temp = qubit1;
                    qubit1 = qubit2;
                    qubit2 = temp;
                }
                if(gate.qubits_used.size() == 2 && qubit2 != INT_MIN && qubit1 != INT_MIN){                    
                    cout<<"Operation : "<<gate.operation<<" "<<qubit1<<" "<<qubit2<<endl;
                    // cout<<gate.qubits_used[0]<<" "<<gate.qubits_used[1]<<endl;
                    // gateMatrix[gate.qubits_used[0]][gate.qubits_used[1]] = 1;
                    // gateMatrix[gate.qubits_used[1]][gate.qubits_used[0]] = 1;
                    cutGraph.push_back({gate.layer*(logiNumber)+qubit1,gate.layer*(logiNumber)+qubit2,gate.layer});
                    gateCutSet.push_back({gate.layer*(logiNumber)+qubit1,gate.layer*(logiNumber)+qubit2,gate.layer});
                }
            }

            for(int i = 0;i<layer-1;i++){
                for(int j = 0;j<logiNumber;j++){
                    cutGraph.push_back({j+logiNumber*i,j+logiNumber*(i+1),i,i+1});
                    wireCutSet.push_back({j+logiNumber*i,j+logiNumber*(i+1),i,i+1});
                }
            } 
        };

    vector<vector<int>> getCutSet(string str){
        if(str == "all"){
            return cutGraph;
        }else if(str == "gate"){
            return gateCutSet;
        }else if(str == "wire"){
            return wireCutSet;
        }
        return {};
    }

};


int main(int argc, char* argv[]) {
    string fileName;
    if(argc > 1){
        fileName = string(argv[1]);
    }else{
        fileName = "example.qasm";
    }
    cout<<fileName<<endl;
    BenchMarkParser bp("QASM_example_2/"+fileName); //example
    QuantumCircuit circuit;
    Graph graph;
    vector<pair<int, int>> singleGateArr = bp.getSingleGateArray();
    vector<pair<string, pair<int, int>>> gateSet = bp.getOriginalGate();
    vector<QuantumGate> gates;
    pair<string, pair<int, int>> gate;
    vector<vector<int>> cutSet;
    for (int i = 0; i < gateSet.size(); i++) {
        gate = gateSet[i];        
        // {"CNOT", {0, 1}, -1}, 
        // cout<<"Operation : "<<gate.first<<" Qubit 1 "<<gate.second.first<<" Qubit 2 "<<gate.second.second<<endl;       
        gates.push_back({gate.first,{gate.second.first,gate.second.second},-1});

    }
    circuit = QuantumCircuit(gates);
    vector<QuantumGate> gateWithDAG = circuit.getGates();
    int maxLayer = gateWithDAG[gateWithDAG.size()-1].layer;
    for(int i = 0;i<gateWithDAG.size();i++){
        QuantumGate qg = gateWithDAG[i];
        cout<<"Operation : "<<qg.operation<<" "<<qg.qubits_used[0]<<" "<<qg.qubits_used[1]<<" at layer "<<qg.layer<<endl;
    }

    map<pair<int, int>, double> edge_weights;

    for (const auto& gate : gateWithDAG) {
        if (gate.qubits_used.size() < 2 || gate.qubits_used[1] == INT_MIN || gate.qubits_used[0] == INT_MIN )
            continue; // 是 1-qubit gate，就跳過

        int q1 = gate.qubits_used[0];
        int q2 = gate.qubits_used[1];

        // 統一順序，小的放前面
        if (q1 > q2) swap(q1, q2);
        double layer_weight = static_cast<double>((maxLayer - gate.layer) + 1) / (maxLayer + 1);
        edge_weights[{q1, q2}] += layer_weight; // 權重 +1（你也可以根據 gate.operation 給不同權重）
    }

    cout << "Edge weights:" << endl;
    for (const auto& [edge, weight] : edge_weights) {
        cout << "Qubit " << edge.first << " - Qubit " << edge.second 
             << " : " << weight << endl;
    }

    ofstream outfile(".//weight_folder//"+fileName+"_weight.txt");
    if (!outfile) {
        cerr << "無法建立檔案 qubit_graph.txt" << endl;
        return 1;
    }

    for (const auto& [edge, weight] : edge_weights) {
        outfile << edge.first << " " << edge.second << " " << weight << endl;
    }

    outfile.close();
    cout << "寫入完成，結果存於 qubit_graph.txt" << endl;
    


}