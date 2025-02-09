#include <iostream>
#include "BenchMarkParser.h"
#include "QCircuitManager.h"


using namespace std;

int main() {
    BenchMarkParser bp("QASM_example/4gt11_84.qasm");
    QuantumCircuit circuit;
    vector<pair<int, int>> singleGateArr = bp.getSingleGateArray();
    vector<pair<string, pair<int, int>>> gateSet = bp.getOriginalGate();
    vector<QuantumGate> gates;
    pair<string, pair<int, int>> gate;
    for (int i = 0; i < gateSet.size(); i++) {
        gate = gateSet[i];        
        // {"CNOT", {0, 1}, -1},        
        gates.push_back({gate.first,{gate.second.first,gate.second.second},-1});

    }
    circuit = QuantumCircuit(gates);
    circuit.printGateLayer();
    // circuit.printCircuit();
     

    // �Ы� QuantumCircuit ��H
    

    // �p��h��
    

    cout << "number of quantum circuit layer: " << circuit.getLayers() << endl;

    // ��X�C�Ӷq�l�h���h
    circuit.printGateLayer();

    return 0;
}
