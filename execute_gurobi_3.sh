#!/bin/bash

# 設定編譯選項
clear
g++ -std=c++17 -I /trainingData/lab401b/gurobi951/linux64/include circuit_knitting_gurobi.cpp BenchMarkParser.cpp QCircuitManager.cpp -o circuit_knitting_2 -L /trainingData/lab401b/gurobi951/linux64/lib -lgurobi_c++ -lgurobi95 -lm
./circuit_knitting_2 aqft_10.qasm