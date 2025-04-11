#!/bin/bash

# 設定編譯選項
CXX=g++
CXXFLAGS="-std=c++17"
SOURCE_FILE="BenchMarkParser.cpp QCircuitManager.cpp TransformToPython.cpp"
OUTPUT_FILE="TransToPython"
filenames="adder_10 adder_20 aqft_10 aqft_15 aqft_20 aqft_25 bv_10 bv_15 bv_20 bv_25 supremacy_15 supremacy_20 supremacy_25"
# 執行編譯命令
echo "正在編譯程式..."
$CXX $CXXFLAGS $SOURCE_FILE -o $OUTPUT_FILE

# g++ -std=c++17 -I /trainingData/lab401b/gurobi951/linux64/include circuit_knitting_gurobi.cpp BenchMarkParser.cpp QCircuitManager.cpp -o circuit_knitting_2 -L /trainingData/lab401b/gurobi951/linux64/lib -lgurobi_c++ -lgurobi95 -lm

# 確認編譯是否成功
if [ $? -eq 0 ]; then
    echo "編譯成功，輸出檔案為 $OUTPUT_FILE"
else
    echo "編譯失敗，請檢查錯誤訊息。"
fi
echo "正在執行 $OUTPUT_FILE..."
for filename in $filenames; do
    file_with_ext="${filename}.qasm"
    if [ -f "./QASM_example_2/$file_with_ext" ]; then
        echo "檔案 $file_with_ext 存在，正在執行 $OUTPUT_FILE..."
        nohup ./$OUTPUT_FILE "$file_with_ext"
        if [ $? -eq 0 ]; then
            echo "執行成功！輸出記錄於 output_${filename}.log"
        else
            echo "執行失敗，請檢查 output_${filename}.log。"
        fi
    else
        echo "檔案 $file_with_ext 不存在，跳過..."
    fi
done