
#include "QCircuitManager.h"



QuantumCircuit::QuantumCircuit(){

}

QuantumCircuit::QuantumCircuit(const vector<QuantumGate>& gates) {
    gateSet = gates;    
    int l = calculateLayers();
    this->totalLayers = l;
}

// �p��q�l�q�����h��
int QuantumCircuit::calculateLayers() {
    int n = gateSet.size();
    vector<int> in_degree(n, 0);          // �����C�Ӹ`�I���J��
    vector<vector<int>> dependencies(n); // �x�s�̿����Y��

    // �c�y�̿����Y
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            // �ˬd i �M j �O�_�ϥάۦP�� qubit
            for (int q1 : gateSet[i].qubits_used) {
                if (find(gateSet[j].qubits_used.begin(), gateSet[j].qubits_used.end(), q1) != gateSet[j].qubits_used.end()) {
                    dependencies[j].push_back(i); // j ���V i
                    in_degree[i]++;
                    break;
                }
            }
        }
    }

    // �ݼ��Ƨǭp��h��
    queue<int> q;
    int maxLayer = 0;

    // �N�J�׬� 0 ���`�I�[�J���C
    for (int i = 0; i < n; ++i) {
        if (in_degree[i] == 0) {
            q.push(i);
            gateSet[i].layer = 0;
        }
    }

    // BFS �p��h��
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

    return maxLayer + 1; // �h�Ʊq 0 �}�l�A�ݥ[ 1
}

// ����q�l�h���h��
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