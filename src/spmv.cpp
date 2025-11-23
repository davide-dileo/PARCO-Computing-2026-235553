#include "spmv.hpp"
#include <omp.h>
#include <stdexcept>

using namespace std;

// parte sequenziale identica ai reference implementations (MKL, cuSPARSE, ecc.)
vector<double> spmv_seq(const CSRMatrix& A, const vector<double>& x)
{
    if (static_cast<int>(x.size()) < A.cols) throw runtime_error("spmv_seq: input vector size mismatch");
    vector<double> y(A.rows, 0.0);

    for (int i = 0; i < A.rows; i++) {
        double sum = 0.0;
        int start = A.row_ptr[i];
        int end   = A.row_ptr[i + 1];
        for (int k = start; k < end; k++) {
            sum += A.val[k] * x[A.col_idx[k]];
        }
        y[i] = sum;
    }
    return y;
}

static omp_sched_t parse_schedule(const string& sched_str)
{
    if (sched_str == "static") return omp_sched_static;
    if (sched_str == "dynamic") return omp_sched_dynamic;
    if (sched_str == "guided") return omp_sched_guided;
    // default
    return omp_sched_static;
}

vector<double> spmv_omp(const CSRMatrix& A, const vector<double>& x, int threads, const string& schedule, int chunk)
{
    if (static_cast<int>(x.size()) < A.cols) throw runtime_error("spmv_omp: input vector size mismatch");
    if (threads <= 0) threads = 1;
    if (chunk <= 0) chunk = 1;

    vector<double> y(A.rows, 0.0);

    // set number of threads
    omp_set_num_threads(threads);

    // configure schedule
    omp_sched_t sched = parse_schedule(schedule);
    if (chunk > 0) {
        omp_set_schedule(sched, chunk);
    }

    // Parallelize over rows. Each y[i] is written by exactly one iteration -> no races.
    #pragma omp parallel for schedule(runtime)
    for (int i = 0; i < A.rows; ++i) {
        double sum = 0.0;
        int start = A.row_ptr[i];
        int end   = A.row_ptr[i + 1];
        for (int k = start; k < end; ++k) {
            sum += A.val[k] * x[A.col_idx[k]];
        }
        y[i] = sum;
    }
    return y;
}