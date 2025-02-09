
#include "QCircuitManager.h"



QuantumCircuit::QuantumCircuit(){

}

QuantumCircuit::QuantumCircuit(const vector<QuantumGate>& gates) {
    gateSet = gates;    
    int l = calculateLayers();
    this->totalLayers = l;
}

// 計算量子電路的層數
int QuantumCircuit::calculateLayers() {
    int n = gateSet.size();
    vector<int> in_degree(n, 0);          // 紀錄每個節點的入度
    vector<vector<int>> dependencies(n); // 儲存依賴關係圖

    // 構造依賴關係
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            // 檢查 i 和 j 是否使用相同的 qubit
            for (int q1 : gateSet[i].qubits_used) {
                if (find(gateSet[j].qubits_used.begin(), gateSet[j].qubits_used.end(), q1) != gateSet[j].qubits_used.end()) {
                    dependencies[j].push_back(i); // j 指向 i
                    in_degree[i]++;
                    break;
                }
            }
        }
    }

    // 拓撲排序計算層數
    queue<int> q;
    int maxLayer = 0;

    // 將入度為 0 的節點加入隊列
    for (int i = 0; i < n; ++i) {
        if (in_degree[i] == 0) {
            q.push(i);
            gateSet[i].layer = 0;
        }
    }

    // BFS 計算層數
    while (!q.empty()) {
        int current = q.front();
        q.pop();
        int currentLayer = gateSet[current].layer;

        for (int next : dependencies[current]) {
            in_degree[next]--;
            gateSet[next].layer = max(gateSet[next].layer, currentLayer + 1);
            maxLayer = max(maxLayer, gateSet[next].layer);
            if (in_degree[next] == 0) {
                q.push(next);
            }
        }
    }

    return maxLayer + 1; // 層數從 0 開始，需加 1
}

// 獲取量子閘的層數
vector<QuantumGate> QuantumCircuit::getGates(){
    return gateSet;
}

int QuantumCircuit::getLayers(){
    return this->totalLayers;
}

void QuantumCircuit::printCircuit(){
    QuantumGate gate;
    for(int i = 0;i<gateSet.size();i++){
        gate = gateSet[i];
        cout<<gate.operation<<" "<<gate.qubits_used[0]<<" "<<gate.qubits_used[1]<<endl;
    }
}

void QuantumCircuit::printGateLayer(){
    for (const auto& gate : gateSet) {
        cout << gate.operation << " Gate (Qubits: ";
        for (int q : gate.qubits_used) {
            cout << q << " ";
        }
        cout << ") No. " << gate.layer << " layer" << endl;
    }   
}