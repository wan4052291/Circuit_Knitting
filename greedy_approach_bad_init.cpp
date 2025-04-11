#include<iostream>
#include"BenchMarkParser.h"
#include"QCircuitManager.h"
#include<vector>
#include<string>
#include<unordered_map>
#include<deque>
#include <cmath>
#include<algorithm>
#include <fstream>

// todo list : 改成總共使用到的qubit不超過上限就好

/* choose most gate each teleportation */
// 每遇到一個cross partition的gate就列出所有的可能
// ex cross gate (0,7) {0,1,2,3} {4,5,6,7}
// 0 <-> 4
// 0 <-> 5
// 0 <-> 6
// 7 <-> 1
// 7 <-> 2
// 7 <-> 3
// gate cutting -> 不動

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

void printNodeInfo(node node){
    cout<<"Node ID : "<<node.id<<endl;
    cout<<"Gate ID : "<<node.gateId<<endl;
    cout<<"Partition : "<<endl;
    for(int i = 0;i<node.qubitPartition.size();i++){
        cout<<"qubit "<< i <<" in "<<node.qubitPartition[i]<<endl;        
    }
}


class PartitionManager{
public : 
    PartitionManager(QuantumCircuit circuit,int qubitNumber,int maxQubitCount){
        this->qubitNumber = qubitNumber;
        this->maxQubitCount = maxQubitCount;
        pair<vector<int>,vector<int>> p = initialPartition(circuit,qubitNumber,maxQubitCount);
        this->partitionArr = p.first;
        this->partitionArrSize = p.second;
        this->circuit = circuit;
        this->wireCutting = 0;
        this->gateCutting = 0;
    }

    PartitionManager(QuantumCircuit circuit,int qubitNumber,int maxQubitCount , string path){
        this->qubitNumber = qubitNumber;
        this->maxQubitCount = maxQubitCount;
        pair<vector<int>,vector<int>> p = initialPartitionFromFile(path);
        this->partitionArr = p.first;
        this->partitionArrSize = p.second;
        this->circuit = circuit;
        this->wireCutting = 0;
        this->gateCutting = 0;
    }

    vector<int> getPartitionArr(){
        return this->partitionArr;
    }

    pair<vector<int>,vector<int>> initialPartition(QuantumCircuit circuit,int qubitNumber,int maxQubitCount){    
        // Partition Array
        //  0  1  2  3  4  5,...,qubitNumber
        // [0  0  0  1  1  1]  Partition Number
        vector<int> partitionArr = vector<int>(qubitNumber,0);
        vector<int> partitionArrSize = vector<int>();
        int partitionQubitCount = (int)(maxQubitCount*0.6);
        for(int i = 0;i<partitionArr.size();i++){
            partitionArr[i] = (int)(i/partitionQubitCount);
            if(partitionArrSize.size() == (int)(i/partitionQubitCount)){
                partitionArrSize.push_back(0);
            }
            partitionArrSize[(int)(i/partitionQubitCount)]++;
        }
    
        return {partitionArr,partitionArrSize};
    }

    pair<vector<int>,vector<int>> initialPartitionFromFile(string path){    
        cout<<"./partition_folder/4/"+path<<endl;
        ifstream infile("./partition_folder/4/"+path);
        int element_count, partition_count;
        int element_id, partition_id;
        vector<int> element_to_partition;
        vector<int> partitionArrSize;

        if (!infile) {
            cerr << "無法開啟檔案 input.txt" << endl;
            return {};
        }
    
        infile >> element_count >> partition_count;
        
        element_to_partition = vector<int>(element_count,0);
        partitionArrSize = vector<int>(partition_count,0);
        
        for (int i = 0; i < element_count; ++i) {
            infile >> element_id >> partition_id;
            cout<<element_id<<" "<<partition_id<<endl;
            element_to_partition[element_id] = partition_id;
            partitionArrSize[partition_id]++;
        }
    
        // 測試輸出
        // cout << "element_to_partition vector: { ";
        // for (int pid : element_to_partition) {
        //     cout << pid << " ";
        // }
        // cout << "}" << endl;
        cout<<element_to_partition.size()<<endl;
        cout<<partitionArrSize.size()<<endl;
        
        return {element_to_partition,partitionArrSize};
    }

    bool isCrossPartition(QuantumGate gate){
        int qubit1 = gate.qubits_used[0];
        int qubit2 = gate.qubits_used[1];
        // printPartitionArr();
        if(qubit1 != INT_MIN && qubit2 != INT_MIN && this->partitionArr[qubit1] != this->partitionArr[qubit2]){
            return true;
        }
    
        return false;
    }

    bool isCrossPartition(QuantumGate gate,vector<int> passPartitionArr){
        int qubit1 = gate.qubits_used[0];
        int qubit2 = gate.qubits_used[1];        
        // printPartitionArr();
        if(qubit1 != INT_MIN && qubit2 != INT_MIN && passPartitionArr[qubit1] != passPartitionArr[qubit2]){
            return true;
        }
    
        return false;
    }

    vector<vector<int>> calculatePotentialPartition(QuantumGate gate){
        vector<vector<int>> potentialPartitionArr;
        vector<int> tempArr = partitionArr;
        vector<int> sameWithQ1PartitionArr;
        vector<int> sameWithQ2PartitionArr;
        int q1 = gate.qubits_used[0];
        int q2 = gate.qubits_used[1];
        int q1Partition = tempArr[q1];
        int q2Partition = tempArr[q2];
    
        potentialPartitionArr.push_back(partitionArr);
        // cout<<"here"<<endl;
        // printPartitionArr();        
    
        // 找到與q1和q2相同partition的qubit
        for(int i = 0;i<tempArr.size();i++){
            if(tempArr[i] == q1Partition && i != q1){
                sameWithQ1PartitionArr.push_back(i);
            }else if(tempArr[i] == q2Partition && i != q2){
                sameWithQ2PartitionArr.push_back(i);
            }
        }

        // 與對方同partition的qubit進行交換
        for(int i = 0 ;i<sameWithQ1PartitionArr.size();i++){
            swap(tempArr[q2],tempArr[sameWithQ1PartitionArr[i]]);
            potentialPartitionArr.push_back(tempArr);
            swap(tempArr[q2],tempArr[sameWithQ1PartitionArr[i]]);
        }

        for(int i = 0 ;i<sameWithQ2PartitionArr.size();i++){
            swap(tempArr[q1],tempArr[sameWithQ2PartitionArr[i]]);
            potentialPartitionArr.push_back(tempArr);
            swap(tempArr[q1],tempArr[sameWithQ2PartitionArr[i]]);
        }


        // 其中一邊不為空,直接傳不交換
        if(this->partitionArrSize[q1Partition] < maxQubitCount){
            tempArr[q2] = q1Partition;
            potentialPartitionArr.push_back(tempArr);
            tempArr[q2] = q2Partition;
        }

        if(this->partitionArrSize[q2Partition] < maxQubitCount){
            tempArr[q1] = q2Partition;
            potentialPartitionArr.push_back(tempArr);
            tempArr[q1] = q1Partition;
        }



        // for(int i = 0;i<potentialPartitionArr.size();i++){
        //     for(int j = 0 ;j<potentialPartitionArr[i].size();j++){
        //         cout<<potentialPartitionArr[i][j]<<" ";
        //     }
        //     cout<<endl;
        // }
        
        
        return potentialPartitionArr;
    }

    int calculatePartitionBenefit(vector<QuantumGate> gateWithDag,int currIdx,vector<int> passPartitionArr){
        int count = 1;
        // cout<<passPartitionArr.size()<<endl;
        // for(int i = 0;i<passPartitionArr.size();i++){
        //     cout<<passPartitionArr[i]<<" ";
        // }
        // cout<<endl;
        
        for(int i = currIdx+1; i < gateWithDag.size(); i++){            
            if(isCrossPartition(gateWithDag[i],passPartitionArr)){
                break;
            }else{
                count++;            
            }
        }
        
        
        return count;
    }

    void updatePartitionArr(vector<int> updatePartitionArr){
        int count = 0;        
        vector<int> tempSizeArr = vector<int>(this->partitionArrSize.size(),0);
        for(int i = 0;i<updatePartitionArr.size();i++){
            if(this->partitionArr[i] != updatePartitionArr[i]){
                count++;
            }
            tempSizeArr[updatePartitionArr[i]]++;
        }

        if(updatePartitionArr == this->partitionArr){
            this->gateCutting++;
        }else{
            this->wireCutting += count;
        }

        this->partitionArr = updatePartitionArr;
        this->partitionArrSize = tempSizeArr;
        
    }

    void printPartitionArr(){
        vector<int> partitionArr = this->partitionArr;
        for(int i = 0;i<partitionArr.size();i++){
            cout<<"qubit : "<< i <<" in "<< partitionArr[i] <<endl;
        }
    }

    pair<int,int> getCuttingCount(){
        return {this->wireCutting,this->gateCutting};
    }

private :
    vector<int> partitionArr;
    vector<int> partitionArrSize;
    QuantumCircuit circuit;
    int partitionNumber;
    int qubitNumber;
    int maxQubitCount;
    int wireCutting;
    int gateCutting;
};

void greedyApproach(QuantumCircuit circuit,int qubitNumber,int maxQubitCount,string partitionFilename){
    int gateCutting = 0;
    int wireCutting = 0;
    // sorted with dag
    vector<QuantumGate> gateWithDag = circuit.getGates();
    QuantumGate qg;
    vector<vector<int>> potentialPartitionArr;
    PartitionManager pm = PartitionManager(circuit,qubitNumber,maxQubitCount,partitionFilename);


    for(int i = 0 ; i < gateWithDag.size() ; i++ ){
        // get 
        qg = gateWithDag[i];
        cout<<qg.operation<<" "<<qg.qubits_used[0]<<" "<<qg.qubits_used[1]<<" At layer : "<<qg.layer<<endl;

        if(qg.qubits_used[0] != INT_MIN && qg.qubits_used[1] != INT_MIN && pm.isCrossPartition(qg)){
            
            //get Potenetial partition
            potentialPartitionArr = pm.calculatePotentialPartition(qg);
            int maxBenefit = -1;
            int benefit = -1;
            vector<int> maxBenefitPartitionArr;

            //get most benefit partition 
            for(int j = 0;j < potentialPartitionArr.size();j++){
                vector<int> tempPartitionArr = potentialPartitionArr[j];
                //calculate how many gate can execute for each partition 
                benefit = pm.calculatePartitionBenefit(gateWithDag,i,tempPartitionArr); 
                cout << benefit << endl;
                if(benefit > maxBenefit){
                    maxBenefit = benefit;
                    maxBenefitPartitionArr = tempPartitionArr;
                }
                
                // pm.printPartitionArr();                
            }
            pm.updatePartitionArr(maxBenefitPartitionArr);            
        }
    }

    cout<<"Wire Cutting : "<<pm.getCuttingCount().first<<endl;
    cout<<"Gate Cutting : "<<pm.getCuttingCount().second<<endl;

}


int main(int argc, char* argv[]){
    string fileName;
    string partitionFilename;
    int qubitNumber;
    if(argc > 1){
        fileName = string(argv[1]);
    }else{
        fileName = "example.qasm";
    }
    cout<<fileName<<endl;
    BenchMarkParser bp("QASM_example_2/"+fileName); //example    
    QuantumCircuit circuit;
    vector<pair<string, pair<int, int>>> gateSet = bp.getOriginalGate();
    vector<QuantumGate> gates;
    vector<QuantumGate> gateWithDag;
    pair<string, pair<int, int>> gate;    
    vector<int> partitionArr;    
    int maxQubitCount = 5; // modified 
    partitionFilename = fileName+"_weight.txt_partition_result.txt";

    for (int i = 0; i < gateSet.size(); i++) {
        gate = gateSet[i];
        // {"CNOT", {0, 1}, -1},
        gates.push_back({gate.first,{gate.second.first,gate.second.second},-1});
    }
    circuit = QuantumCircuit(gates);
    qubitNumber = bp.getGreatiestNumber()+1;
    // partitionArr = initialPartition(circuit,qubitNumber,maxQubitCount);
    
    gateWithDag = circuit.getGates();

    // print gate information
    // for(int i = 0;i < gateWithDag.size() ; i++){
    //     QuantumGate gate = gateWithDag[i];
    //     cout<<gate.operation<<" "<<gate.qubits_used[0]<<" "<<gate.qubits_used[1]<<" at layer "<<gate.layer<<endl;
    // }

    //call greedy approach
    greedyApproach(circuit,qubitNumber,maxQubitCount,partitionFilename);

    return 0;    
}