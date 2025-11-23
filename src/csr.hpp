#pragma once
#include <vector>

using namespace std;

struct CSRMatrix {
    int rows;
    int cols;
    int nnz;
    vector<int> row_ptr;
    vector<int> col_idx;
    vector<double> val;
};