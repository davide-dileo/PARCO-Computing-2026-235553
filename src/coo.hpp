#pragma once
#include <vector>

using namespace std;

struct COOMatrix {
    int rows;
    int cols;
    int nnz;
    vector<int> row;
    vector<int> col;
    vector<double> val;
};