#!/bin/bash

# 設定編譯選項
CXX=g++
CXXFLAGS="-std=c++17"
SOURCE_FILE="BenchMarkParser.cpp QCircuitManager.cpp main.cpp"
OUTPUT_FILE="program"


# 執行編譯命令
echo "正在編譯程式..."
$CXX $CXXFLAGS $SOURCE_FILE -o $OUTPUT_FILE

# 確認編譯是否成功
if [ $? -eq 0 ]; then
    echo "編譯成功，輸出檔案為 $OUTPUT_FILE"
    echo "正在執行 $OUTPUT_FILE..."
    
    # 執行產生的執行檔
    ./$OUTPUT_FILE
    
    # 檢查執行是否成功
    if [ $? -eq 0 ]; then
        echo "執行成功！"
    else
        echo "執行失敗，請檢查錯誤訊息。"
    fi
else
    echo "編譯失敗，請檢查錯誤訊息。"
fi
