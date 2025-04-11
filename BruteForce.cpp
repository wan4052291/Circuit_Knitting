#include<iostream>
#include"BenchMarkParser.h"
#include"QCircuitManager.h"
#include<vector>
#include<string>
#include<unordered_map>
#include<deque>
#include <cmath>

class node{
public: 
    vector<int> qubitPartition;
    int id;   
    int gateId; 
    node(int id,vector<int> qubitPartition,int gateId){
        this->id = id;
        this->qubitPartition = qubitPartition;
        this->gateId = gateId;
    }
};

bool isCrossPartition(QuantumGate gate,vector<int> partitionArr){    
    int qubit1 = gate.qubits_used[0];
    int qubit2 = gate.qubits_used[1];
    if(partitionArr[qubit1] != partitionArr[qubit2]){
        return true;
    }

    return false;
}

void printNodeInfo(node node){
    cout<<"Node ID : "<<node.id<<endl;
    cout<<"Gate ID : "<<node.gateId<<endl;
    cout<<"Partition : "<<endl;
    for(int i = 0;i<node.qubitPartition.size();i++){
        cout<<"qubit "<< i <<" in "<<node.qubitPartition[i]<<endl;        
    }
}

vector<int> initialPartition(QuantumCircuit circuit,int qubitNumber,int maxQubitCount){    
    vector<int> partitionArr = vector<int>(qubitNumber,0);
    int partitionQubitCount = (int)(maxQubitCount*0.6);
    for(int i = 0;i<partitionArr.size();i++){
        partitionArr[i] = (int)(i/partitionQubitCount);
    }

    return partitionArr;
}

void constructTree(vector<QuantumGate> gateWithDag,vector<int> partitionArr){
    vector<node> nodeArr = vector<node>();    
    unordered_map<int, vector<pair<int, double>>> adj;    
    vector<int> tempPartition;
    deque<node> dq;
    int nodeId = 0;
    node tempNode = node(nodeId,partitionArr,0);        
    nodeArr.push_back(tempNode);
    dq.push_back(tempNode);        
    nodeId++;    
    for(int i = 0;i<gateWithDag.size();i++){
        // cout<<dq.size()<<endl;
        int size = dq.size();
        int currentId = 0;
        for(int j = 0;j<size;j++){
            tempNode = dq.front();
            dq.pop_front();
            tempPartition = tempNode.qubitPartition;
            currentId = tempNode.id;
            int qubit1 = gateWithDag[i].qubits_used[0];
            int qubit2 = gateWithDag[i].qubits_used[1];
            int tempP = tempPartition[qubit1];
            if(isCrossPartition(gateWithDag[i],tempPartition)){

                // cout<<"Cross : "<<qubit1<<" "<<qubit2<<endl;
                tempPartition[qubit1] = tempPartition[qubit2];
                tempNode = node(nodeId,tempPartition,i+1);
                adj[currentId].push_back({nodeId,log(4)});
                nodeId++;
                // printNodeInfo(tempNode);
                nodeArr.push_back(tempNode);
                dq.push_back(tempNode);

                tempPartition[qubit2] = tempP;
                tempPartition[qubit1] = tempP;
                tempNode = node(nodeId,tempPartition,i+1);
                adj[currentId].push_back({nodeId,log(4)});
                nodeId++;
                // printNodeInfo(tempNode);
                nodeArr.push_back(tempNode);
                dq.push_back(tempNode);
            }else{
                // cout<<"No Cross : "<<qubit1<<" "<<qubit2<<endl;
                tempNode = node(nodeId,tempPartition,i+1); 
                adj[currentId].push_back({nodeId,0});
                nodeArr.push_back(tempNode);
                nodeId++;
                // printNodeInfo(tempNode);
                dq.push_back(tempNode);
                
            }
        }  
    }
    // cout<<"dq Size : "<<dq.size()<<endl;
    cout<<"----------------------Print Node Array ----------------"<<endl;
    // for(int i = 0;i<nodeArr.size();i++){        
    //     printNodeInfo(nodeArr[i]);
    //     cout<<"----------------------"<<endl;
    // }
    // cout<<"Log 4 "<<log(4)<<endl;
    // for (const auto &entry : adj) {
    //     int from = entry.first;
    //     const vector<pair<int, double>> &edges = entry.second;
    
    //     cout << "Node " << from << " has edges to:" << endl;
    //     for (const auto &edge : edges) {
    //         int to = edge.first;
    //         double weight = edge.second;
    //         cout << "  -> Node " << to << " with weight " << weight << endl;
    //     }
    // }


    unordered_map<int, double> minWeight;
    for (auto &n : nodeArr) {
        minWeight[n.id] = numeric_limits<double>::infinity();
    }
    minWeight[0] = 0;  // Root node weight is 0

    deque<int> q;
    q.push_back(0);
    
    while (!q.empty()) {
        int cur = q.front();
        q.pop_front();
        for (auto &neighbor : adj[cur]) {
            int nextNode = neighbor.first;
            // cout<<"Cur : "<<cur<<" Next Node : "<<nextNode<<endl;
            double weight = neighbor.second;
            if (minWeight[cur] + weight < minWeight[nextNode]) {
                minWeight[nextNode] = minWeight[cur] + weight;
                q.push_back(nextNode);
            }
        }
        cout<<"------------------"<<endl;
    }

    // Print minimum weight to each leaf node
    // cout << "---------------------- Min Path Weights ----------------" << endl;
    // for (auto &n : nodeArr) {
    //     if (adj.find(n.id) == adj.end()) {  // Leaf node
    //         cout << "Leaf Node ID " << n.id << " - Min Path Weight: " << minWeight[n.id] << endl;
    //     }
    // }
    for (const auto &entry : minWeight) {
        int key = entry.first;
        double value = entry.second;
        cout << "Key: " << key << ", Value: " << value << endl;
    }
}


int main(int argc, char* argv[]){
    string fileName;
    int qubitNumber;
    if(argc > 1){
        fileName = string(argv[1]);
    }else{
        fileName = "example.qasm";
    }
    cout<<fileName<<endl;
    BenchMarkParser bp("QASM_example/"+fileName); //example
    QuantumCircuit circuit;    
    vector<pair<string, pair<int, int>>> gateSet = bp.getOriginalGate();
    vector<QuantumGate> gates;
    vector<QuantumGate> gateWithDag;
    pair<string, pair<int, int>> gate;    
    vector<int> partitionArr;
    int maxQubitCount = 5;
    for (int i = 0; i < gateSet.size(); i++) {
        gate = gateSet[i];
        // {"CNOT", {0, 1}, -1},
        gates.push_back({gate.first,{gate.second.first,gate.second.second},-1});
    }
    circuit = QuantumCircuit(gates);
    qubitNumber = bp.getGreatiestNumber()+1;
    partitionArr = initialPartition(circuit,qubitNumber,maxQubitCount);
    for(int i = 0;i<partitionArr.size();i++){
        cout<<"qubit : "<<i<<" in "<<partitionArr[i]<<endl;
    }
    gateWithDag = circuit.getGates();
    constructTree(gateWithDag,partitionArr);
    // circuit.printCircuit();
    // circuit.printGateLayer();
    // cout<<qubitNumber<<endl;

    return 0;    
}