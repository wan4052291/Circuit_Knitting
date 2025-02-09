#include "gurobi_c++.h"
#include <iostream>
#include <tuple>
#include <vector>
#include <set>
#include <string>
#include <math.h>
#include "BenchMarkParser.h"
#include "QCircuitManager.h"
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
                if(gate.qubits_used.size() == 2 && qubit2 != INT_MIN){                    
                    // cout<<"Operation : "<<gate.operation<<" "<<qubit1<<" "<<qubit2<<endl;
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
    BenchMarkParser bp("QASM_example/"+fileName); //example
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
        gates.push_back({gate.first,{gate.second.first,gate.second.second},-1});

    }
    circuit = QuantumCircuit(gates);
    
    graph = Graph(bp.getGreatiestNumber()+1,circuit);    

    // print Cutting
    // cutSet = graph.getCutSet("all");
    // vector<int> cutTemp;    
    // for(int i = 0;i<cutSet.size();i++){
    //     cutTemp = cutSet[i];
    //     if(cutTemp.size() == 3){
    //         cout<<"Gate Cut "<<"Qubit{ "<<cutTemp[0]<<","<<cutTemp[1]<<" } "<<" at layer : "<< cutTemp[2]<<endl;
    //     }else{
    //         cout<<"Wire Cut "<<"Qubit 1 : "<<cutTemp[0]<<" Qubit 2 : "<<cutTemp[1]<<" Qubit 1 at layer : "<< cutTemp[2]<<" Qubit 2 at layer : "<< cutTemp[3]<<endl;
    //     }
    // }

    try {
        
        GRBEnv env = GRBEnv(true);
        env.set("LogFile", "mip.log");
        env.start();
        GRBModel model = GRBModel(env);
        model.set(GRB_IntParam_Threads, 96);
        model.set(GRB_DoubleParam_TimeLimit, 3600.0);
        model.set(GRB_IntParam_Presolve, 2);
        
        int numPartitions = 10;
        int numQubits = bp.getGreatiestNumber()+1;
        int layers = circuit.getLayers();
        int qubitLimit = 4;

        vector<vector<GRBVar>> m(numQubits*layers, vector<GRBVar>(numPartitions)); // m[v][p]
        map<pair<int,int>, GRBVar> C; // C[u][v]
        map<tuple<int,int,int>, GRBVar> y; // y[u][v][p] only wire cut
        map<tuple<int,int,int>, GRBVar> z; // z[u][v][p] only wire cut
        vector<QuantumGate> gateSet = circuit.getGates();

        // --------------------variable---------------------------
        cout<<"Create Varaible m - m_{v,p}"<<endl;
        for (int v = 0; v < numQubits*layers; v++) {
            for (int p = 0; p < numPartitions; p++) {
                m[v][p] = model.addVar(0, 1, 0, GRB_BINARY, "m_" + to_string(v) + "_" + to_string(p));
                // cout<<"m_" + to_string(v) + "_" + to_string(p)<<endl;
            }
        }

        cout<<"Create Varaible y - y_{u,v,p}"<<endl;
        for(int p = 0;p < numPartitions;p++){
            for (int i = 0; i < layers - 1; i++) {
                for (int j = 0; j < numQubits; j++) {
                    string key = to_string(j + numQubits * i) + "_" + to_string(j + numQubits * (i + 1)) + "_" + to_string(p);
                    y[{j + numQubits * i,j + numQubits * (i + 1),p}] = model.addVar(0, 1, 0, GRB_BINARY, "y_" + key);
                    // cout << "y_" + key << endl;
                }
            }
        }
        for (auto gate : gateSet) {
            int qubit1 = gate.qubits_used[0];
            int qubit2 = gate.qubits_used[1];
            if (qubit1 > qubit2){
                swap(qubit1, qubit2);
            }
            for (int p = 0; p < numPartitions; p++){
                if (gate.qubits_used.size() == 2 && qubit2 != INT_MIN) {
                    string key = to_string(gate.layer * numQubits + qubit1) + "_" + to_string(gate.layer * numQubits + qubit2) + "_" + to_string(p);
                    y[{gate.layer * numQubits + qubit1,gate.layer * numQubits + qubit2,p}] = model.addVar(0, 1, 0, GRB_BINARY, "y_" + key);
                    // cout << "y_" + key << endl;
                }
            }
        }

        cout<<"Create Varaible C - C_{u,v}"<<endl;
        for (int i = 0; i < layers - 1; i++) {
            for (int j = 0; j < numQubits; j++) {
                string key = to_string(j + numQubits * i) + "_" + to_string(j + numQubits * (i + 1));
                C[{j + numQubits * i,j + numQubits * (i + 1)}] = model.addVar(0, 1, 0, GRB_BINARY, "C_" + key);
                // cout << "C_" + key << endl;
            }
        }
        for (auto gate : gateSet) {
            int qubit1 = gate.qubits_used[0];
            int qubit2 = gate.qubits_used[1];
            if (qubit1 > qubit2){
                swap(qubit1, qubit2);
            }
            if (gate.qubits_used.size() == 2 && qubit2 != INT_MIN) {
                string key = to_string(gate.layer * numQubits + qubit1) + "_" + to_string(gate.layer * numQubits + qubit2);
                C[{gate.layer * numQubits + qubit1,gate.layer * numQubits + qubit2}] = model.addVar(0, 1, 0, GRB_BINARY, "C_" + key);
                // cout << "C_" + key << endl;
            }
        }

        cout<<"Create Varaible Z - Z^{p}_{u,v}"<<endl;
        for(int k = 0;k < numPartitions;k++){
            for (int i = 0; i < layers - 1; i++) {
                for (int j = 0; j < numQubits; j++) {
                    string key = to_string(j + numQubits * i) + "_" + to_string(j + numQubits * (i + 1))+ "_" + to_string(k);
                    z[{j + numQubits * i,j + numQubits * (i + 1),k}] = model.addVar(0, 1, 0, GRB_BINARY, "z_" + key);
                    // cout << "z_" + key << endl;
                }
            }
        }

        // -----------------constraint----------------------------
        //add constraint sigma m_{v,p} = 1
        for (int v = 0; v < layers*numQubits; v++) {
            GRBLinExpr sumM = 0;
            for (int p = 0; p < numPartitions; p++) {
                sumM += m[v][p];
            }
            model.addConstr(sumM == 1, "assign_" + to_string(v));
        }


        // // Add consistency constraint for y[u, v, p] 
        cout << "Add consistency constraint for y[u, v, p]" << endl;
        for (auto &[edge, var] : y) {
            int u = get<0>(edge);
            int v = get<1>(edge);
            int p = get<2>(edge);
            // cout<<"u : "<<to_string(u)<<" v : "<<to_string(v)<<endl;
            model.addConstr(var >= m[u][p] - m[v][p], "yuv_upper_" + to_string(u) + "_" + to_string(v)+ "_" + to_string(p));
            model.addConstr(var >= m[v][p] - m[u][p], "yuv_lower_" + to_string(u) + "_" + to_string(v)+ "_" + to_string(p));
            model.addConstr(var <= m[u][p] + m[v][p], "yuv_sum_" + to_string(u) + "_" + to_string(v)+ "_" + to_string(p));  
        }

        // Add consistency constraint for C[u, v] 
        cout << "Add consistency constraint for C[u, v]" << endl;
        for (auto &[edge, var] : C){
            int u = edge.first;
            int v = edge.second;
            GRBLinExpr sum_y = 0;

            for(int p = 0;p<numPartitions;p++){
                sum_y += y[{u,v,p}];
            }
            model.addConstr(2 * var >= sum_y, "C_"+ to_string(u) + "_"+ to_string(v) +"_ge");
            model.addConstr(2 * var <= sum_y, "C_"+ to_string(u) + "_"+ to_string(v) +"_le");

            
        }

        // Add consistency constraint for Z[u, v, p]
        cout << "Add consistency constraint for Z[u, v, p]" << endl;
        for (auto &[key, var1] : z) {
            int u = get<0>(key);
            int v = get<1>(key);
            int p = get<2>(key);
            model.addConstr(var1 <= C[{u,v}] , "Zuvp_m_u_" + to_string(u) + "_" + to_string(v) + "_" + to_string(p));
            model.addConstr(var1 <= m[v][p], "Zuvp_m_v_" + to_string(u) + "_" + to_string(v) + "_" + to_string(p));
            model.addConstr(var1 >= C[{u,v}] + m[v][p] - 1, "Zuvp_lower_" + to_string(u) + "_" + to_string(v) + "_" + to_string(p));
            // for (auto &[edge, var2] : C) {
            //     if(edge.second == v && edge.first == u){
                    
            //     }
            // }
        }

        // sigma m_{v,p} + sigma z_u_v_p <= N
        for(int i = 0;i < numPartitions; i++ ){
            GRBLinExpr sumM = 0;
            GRBLinExpr sumZ = 0;

            // m 
            for(int j = 0;j < numQubits; j++){
                sumM += m[j][i];
            }

            // z
            for (auto &[key, var] : z) {
                if (get<2>(key) == i) { 
                    sumZ += var;
                }
            }
            model.addConstr(sumM + sumZ <= qubitLimit, "partition_capacity_" + to_string(i));
        }

        model.update();
        // GRBConstr* constraints = model.getConstrs();
        // int numConstrs = model.get(GRB_IntAttr_NumConstrs);
        // cout<<"Number of constraint : "<<numConstrs<<endl;
        // for (int i = 0; i < numConstrs; i++) {
        //     std::cout << "Constraint " << i << ": " << constraints[i].get(GRB_StringAttr_ConstrName) << std::endl;
        // }

        GRBLinExpr W_C = 0; // (z[u,v,p] = 1)
        GRBLinExpr G_C = 0; // (C[u,v] = 1) - (z[u,v,p] = 1)
        double W_C_val = 0;
        double G_C_val = 0;

        for (auto &[key, var] : z) {
            W_C += var; // z[u,v,p]
        }

        // 
        for (auto &[edge, var] : C) {
            G_C += var; // C[u,v]
        }
        G_C -= W_C;
        GRBVar A = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "A");
        GRBVar B = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "B");
        GRBVar W_C_var = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "W_C_var");
        GRBVar G_C_var = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "G_C_var");
        // GRBVar W_C_log = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "W_C_log");
        // GRBVar G_C_log = model.addVar(-GRB_INFINITY, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "G_C_log");
        model.addConstr(W_C_var == W_C, "W_C_Constraint");
        model.addConstr(G_C_var == G_C, "G_C_Constraint");
        // model.addConstr(W_C_log == W_C_var * log(4), "W_C_Log_Constraint");
        // model.addConstr(G_C_log == G_C_var * log(6), "G_C_Log_Constraint");
        
        model.addGenConstrExpA(W_C_var, A,log(4),"Exp4");
        model.addGenConstrExpA(G_C_var, B,log(6),"Exp6");
        model.setObjective(A + B, GRB_MINIMIZE);
        model.update();
        model.optimize();
        model.write("model.lp");

        //check
        if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
            cout << "Optimal solution found!" << endl;
            // model.write("model.lp");
        } else if (model.get(GRB_IntAttr_Status) == GRB_INFEASIBLE) {
            cout << "Model is infeasible. Finding IIS..." << endl;
            model.computeIIS();
            model.write("infeasible.ilp");
            cout << "IIS written to 'infeasible.ilp'" << endl;
        } else if (model.get(GRB_IntAttr_Status) == GRB_UNBOUNDED) {
            cout << "Model is unbounded!" << endl;
        } else {
            cout << "Optimization ended with status: " << model.get(GRB_IntAttr_Status) << endl;
        }

        // print solution
        // print m 
        for (int v = 0; v < numQubits * layers; v++) {
            for (int p = 0; p < numPartitions; p++) {
                cout << "m[" << v << "][" << p << "] = "<< m[v][p].get(GRB_DoubleAttr_X) << endl;
            }
        }

        // print C
        for (const auto& [edge, var] : C) {
            int u = edge.first;
            int v = edge.second;
            if(var.get(GRB_DoubleAttr_X) == 1){
                cout << "C[" << u << "][" << v << "] = "<< var.get(GRB_DoubleAttr_X) << endl;
                // for(int p = 0; p < numPartitions; p++){
                //     cout << "z[" << u << "][" << v << "][" << p << "] = "<< z[u][v][p].get(GRB_DoubleAttr_X) << endl;
                // }
            }            
            
        }

        // print z
        // for (const auto& [key, var] : z) {
        //     int u = get<0>(key);
        //     int v = get<1>(key);
        //     int p = get<2>(key);
        //     cout << "z[" << u << "][" << v << "][" << p << "] = "<< var.get(GRB_DoubleAttr_X) << endl;
        // }

        // print Cut
        // calculate W_C
        for (auto &[key, var] : z) {
            W_C_val += var.get(GRB_DoubleAttr_X);
        }

        // calculate G_C
        for (auto &[edge, var] : C) {            
            G_C_val += var.get(GRB_DoubleAttr_X);
            if(var.get(GRB_DoubleAttr_X) == 1){
                int u = edge.first;
                int v = edge.second;
                cout<<"Cutting edge "<<u<<"->"<<v<<endl;
            }

        }
        // cout << "G_C = " << G_C_val << endl;
        G_C_val -= W_C_val;  // G_C = sum(C) - sum(z)

        cout << "W_C = " << W_C_val << endl;
        cout << "G_C = " << G_C_val << endl;

    } catch (GRBException &e) {
        cout << "Gurobi exception: " << e.getMessage() << endl;
    } catch (...) {
        cout << "Unknown exception occurred." << endl;
    }

    return 0;
}