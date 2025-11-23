#include "read_mtx.hpp"
#include "coo.hpp"
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <numeric>
#include <ostream>

using namespace std;

// Import mmio C library correctly in C++
extern "C" {
    #include "mmio/mmio.h"
}

COOMatrix read_matrix_market(const string& filename)
{
    FILE* f = fopen(filename.c_str(), "r");
    if (!f) {
        throw runtime_error("Cannot open file: " + filename);
    }

    MM_typecode matcode;
    if (mm_read_banner(f, &matcode) != 0) {
        fclose(f);
        throw runtime_error("Could not process Matrix Market banner.");
    }

    if (!mm_is_sparse(matcode)) {
        fclose(f);
        throw runtime_error("Error: Matrix is not sparse.");
    }

    int rows, cols, nnz;
    if (mm_read_mtx_crd_size(f, &rows, &cols, &nnz) != 0) {
        fclose(f);
        throw runtime_error("Could not read matrix dimensions.");
    }

    COOMatrix M;
    M.rows = rows;
    M.cols = cols;

    vector<int> r, c;
    vector<double> v;

    r.reserve(nnz * 2);
    c.reserve(nnz * 2);
    v.reserve(nnz * 2);

    // --- read COO entries ---
    for (int i = 0; i < nnz; i++) {
        int ri, ci;
        double val;

        int ret = fscanf(f, "%d %d %lf", &ri, &ci, &val);
        if (ret != 3) {
            fclose(f);
            throw runtime_error(
                "Error while reading COO entry at line " + to_string(i+1)
            );
        }

        ri--;  // convert to 0-based
        ci--;

        r.push_back(ri);
        c.push_back(ci);
        v.push_back(val);

        // If matrix is symmetric, store also the transposed entry
        if (mm_is_symmetric(matcode) && ri != ci) {
            r.push_back(ci);
            c.push_back(ri);
            v.push_back(val);
        }
    }

    fclose(f);

    // update nnz after symmetry expansion
    M.nnz = (int) r.size();
    M.row = r;
    M.col = c;
    M.val = v;

    // --------- SORT COO BY (row, then col) ----------
    vector<int> order(M.nnz);
    iota(order.begin(), order.end(), 0);

    sort(order.begin(), order.end(), [&](int a, int b) {
        if (M.row[a] != M.row[b])
            return M.row[a] < M.row[b];
        return M.col[a] < M.col[b];
    });

    // reorder COO arrays
    vector<int> row_sorted(M.nnz);
    vector<int> col_sorted(M.nnz);
    vector<double> val_sorted(M.nnz);

    for (int i = 0; i < M.nnz; i++) {
        row_sorted[i] = M.row[order[i]];
        col_sorted[i] = M.col[order[i]];
        val_sorted[i] = M.val[order[i]];
    }

    M.row = std::move(row_sorted);
    M.col = std::move(col_sorted);
    M.val = std::move(val_sorted);

    return M;
}