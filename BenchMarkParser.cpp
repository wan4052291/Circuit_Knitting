#include "BenchMarkParser.h"

BenchMarkParser::BenchMarkParser(string benchMarkFile) {
    BenchMarkParser::layer = 0;
    gateFileReader(benchMarkFile);
}

void BenchMarkParser::gateFileReader(string benchMarkFile) {
    ifstream in;
    in.open(benchMarkFile);
    string inputTemp = "";
    string operation;
    string qubit;
    pair<int, int> gateNumber;
    int maxQubit = 0;

    for (int i = 0; i < 8; i++) {
        // cout << i << endl;
        in >> inputTemp;
        // cout << inputTemp << " ";
        if (i == 5) {
            logiNumber = extractNumbers(inputTemp).first;
        }
        if (i % 2 == 0 && i != 0) {
            // cout << endl;
        }
    }

    while (in >> operation >> qubit) {
        gateNumber = extractNumbers(qubit);
        originalGate.push_back({operation, gateNumber});
        if (gateNumber.first != INT_MIN && gateNumber.second != INT_MIN) {
            removedSingleGate.push_back(gateNumber);
        }
        maxQubit = max(gateNumber.first,maxQubit);
        maxQubit = max(gateNumber.second,maxQubit);
    }
    this->greatiestQubit = maxQubit;
}

pair<int, int> BenchMarkParser::extractNumbers(string input) {
    vector<int> numbers;
    regex re("q\\[(\\d+)\\]");
    auto words_begin = sregex_iterator(input.begin(), input.end(), re);
    auto words_end = sregex_iterator();

    for (sregex_iterator i = words_begin; i != words_end; ++i) {
        smatch match = *i;
        numbers.push_back(stoi(match[1].str()));
    }
    if (numbers.size() == 2) {
        return {numbers[0], numbers[1]};
    } else {
        return {numbers[0], INT_MIN};
    }
}

vector<pair<int, int>> BenchMarkParser::getRemoveArray() {
    return removedSingleGate;
}

vector<pair<int, int>> BenchMarkParser::getSingleGateArray() {
    return removedSingleGate;
}

int BenchMarkParser::getLogiNumber() {
    return logiNumber;
}

vector<pair<string, pair<int, int>>> BenchMarkParser::getOriginalGate() {
    return originalGate;
}

int BenchMarkParser::getLayer(){
    return layer;
}


int BenchMarkParser::getGreatiestNumber(){
    return greatiestQubit;
}
