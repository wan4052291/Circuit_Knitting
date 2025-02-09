#ifndef BENCHMARKPARSER_H
#define BENCHMARKPARSER_H

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <fstream>
#include <regex>
#include <algorithm>

#define INT_MIN (-2147483647 - 1)

using namespace std;

class BenchMarkParser {
private:
    vector<pair<string, pair<int, int>>> originalGate;
    vector<pair<int, int>> removedSingleGate;
    pair<int, int> extractNumbers(string input);
    int logiNumber;
    int layer;
    int greatiestQubit;

public:
    BenchMarkParser(string benchMarkFile);

    void gateFileReader(string benchMarkFile);

    vector<pair<int, int>> getRemoveArray();

    vector<pair<int, int>> getSingleGateArray();

    int getLogiNumber();

    vector<pair<string, pair<int, int>>> getOriginalGate();

    int getLayer();    

    int getGreatiestNumber();
};

#endif // BENCHMARKPARSER_H
