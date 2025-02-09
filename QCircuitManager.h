
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

// 定義量子閘結構
struct QuantumGate {
    string operation;          // 量子閘操作名稱
    vector<int> qubits_used;   // 使用的 qubits
    int layer;                 // 層數
};

// QuantumCircuit 類別宣告
class QuantumCircuit {
public:
    QuantumCircuit(); // 建構子
    QuantumCircuit(const vector<QuantumGate>& gates); // 建構子

    int calculateLayers();                            // 計算電路層數
    vector<QuantumGate> getGates() ;      // 獲取量子閘的層數
    void printCircuit();
    void printGateLayer();
    int getLayers();

private:
    vector<QuantumGate> gateSet; // 存放量子閘
    int totalLayers = -1;
};

#endif // QUANTUMCIRCUIT_H
