#pragma once

#include "csr.hpp"
#include <vector>
#include <string>

using namespace std;

// Sequential SpMV (CSR)
vector<double> spmv_seq(const CSRMatrix& A, const vector<double>& x);

// Parallel SpMV using OpenMP.
// threads: number of OpenMP threads to use
// schedule: "static", "dynamic", "guided"
// chunk: chunk size (if <=0 a default of 1 is used)
vector<double> spmv_omp(const CSRMatrix& A, const vector<double>& x, int threads, const string& schedule, int chunk);