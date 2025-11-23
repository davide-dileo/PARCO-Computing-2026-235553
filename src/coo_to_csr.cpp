#include "coo_to_csr.hpp"

using namespace std;

CSRMatrix coo_to_csr(const COOMatrix& A)
{
    CSRMatrix C;
    C.rows = A.rows;
    C.cols = A.cols;
    C.nnz  = A.nnz;

    C.row_ptr.assign(C.rows + 1, 0);

    // 1. count nnz per row
    for (int i = 0; i < A.nnz; i++) {
        C.row_ptr[A.row[i]]++;
    }

    // 2. exclusive prefix sum
    int sum = 0;
    for (int i = 0; i < C.rows; i++) {
        int tmp = C.row_ptr[i];
        C.row_ptr[i] = sum;
        sum += tmp;
    }
    C.row_ptr[C.rows] = sum;

    // 3. since COO is sorted, we can directly copy the arrays
    C.col_idx = A.col;
    C.val     = A.val;

    return C;
}