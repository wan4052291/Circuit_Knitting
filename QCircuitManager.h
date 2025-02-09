
#ifndef QCIRCUITMANAGER_H
#define QCIRCUITMANAGER_H

#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <iostream>


using namespace std;

// �w�q�q�l�h���c
struct QuantumGate {
    string operation;          // �q�l�h�ާ@�W��
    vector<int> qubits_used;   // �ϥΪ� qubits
    int layer;                 // �h��
};

// QuantumCircuit ���O�ŧi
class QuantumCircuit {
public:
    QuantumCircuit(); // �غc�l
    QuantumCircuit(const vector<QuantumGate>& gates); // �غc�l

    int calculateLayers();                            // �p��q���h��
    vector<QuantumGate> getGates() ;      // ����q�l�h���h��
    void printCircuit();
    void printGateLayer();
    int getLayers();

private:
    vector<QuantumGate> gateSet; // �s��q�l�h
    int totalLayers = -1;
};

#endif // QUANTUMCIRCUIT_H
