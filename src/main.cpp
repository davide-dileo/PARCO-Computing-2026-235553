#include "read_mtx.hpp"
#include "coo.hpp"
#include "csr.hpp"
#include "coo_to_csr.hpp"
#include "spmv.hpp"
#include "utils/vector_utils.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <omp.h>
#include <filesystem>

using namespace std;
using namespace std::chrono;

// ------------------------------------------------------
// 90th percentile
// ------------------------------------------------------
double percentile90(vector<double>& v) {
    if (v.empty()) return 0;
    sort(v.begin(), v.end());
    int idx = static_cast<int>(0.9 * v.size());
    if (idx >= static_cast<int>(v.size())) idx = v.size() - 1;
    return v[idx];
}

// ------------------------------------------------------
// file utils: save/load binary double vector
// ------------------------------------------------------
bool save_vector_binary(const string &path, const vector<double> &v) {
    ofstream out(path, ios::binary);
    if(!out) return false;
    uint64_t n = v.size();
    out.write(reinterpret_cast<const char*>(&n), sizeof(n));
    out.write(reinterpret_cast<const char*>(v.data()), sizeof(double) * n);
    out.close();
    return true;
}

bool load_vector_binary(const string &path, vector<double> &v) {
    ifstream in(path, ios::binary);
    if(!in) return false;
    uint64_t n;
    in.read(reinterpret_cast<char*>(&n), sizeof(n));
    v.resize(n);
    in.read(reinterpret_cast<char*>(v.data()), sizeof(double) * n);
    in.close();
    return true;
}

// ------------------------------------------------------
// Extract filename only (no path)
// ------------------------------------------------------
string basename_from_path(const string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == string::npos) return path;
    return path.substr(pos + 1);
}

// ------------------------------------------------------
// Find reference result path for matrix
// ------------------------------------------------------
string ref_path_from_matrix(const string& matrix_filename) {
    string shortname = basename_from_path(matrix_filename);
    return "results/ref/" + shortname + ".ref";
}

// ------------------------------------------------------
// CLI helpers
// ------------------------------------------------------
string get_option(int argc, char** argv, const string& opt, const string& def = "") {
    for (int i = 1; i < argc - 1; i++)
        if (string(argv[i]) == opt) return string(argv[i + 1]);
    return def;
}

int get_option_int(int argc, char** argv, const string& opt, int def) {
    for (int i = 1; i < argc - 1; i++)
        if (string(argv[i]) == opt) return stoi(argv[i + 1]);
    return def;
}

// ------------------------------------------------------
int main(int argc, char** argv)
{
    if (argc < 2) {
        cerr << "Usage:\n"
             << "  ./spmv --mode seq|omp --matrix file.mtx "
             << "[--threads T] [--schedule S] [--chunk C] "
             << "[--repeat R] [--csv out.csv]\n";
        return 1;
    }

    // Required
    string mode     = get_option(argc, argv, "--mode");
    string filename = get_option(argc, argv, "--matrix");
    string fname_short = basename_from_path(filename);

    if (mode != "seq" && mode != "omp") {
        cerr << "Error: --mode must be seq or omp\n";
        return 1;
    }
    if (filename.empty()) {
        cerr << "Error: --matrix required\n";
        return 1;
    }

    // Optional
    int threads  = get_option_int(argc, argv, "--threads", 1);
    string schedule = get_option(argc, argv, "--schedule", "static");
    int chunk    = get_option_int(argc, argv, "--chunk", 0);  // 0 = default OMP
    int repeat   = get_option_int(argc, argv, "--repeat", 10);
    string csv_file = get_option(argc, argv, "--csv", "");

    // Validate
    if (threads < 1) threads = 1;
    if (repeat < 1) repeat = 1;
    if (chunk < 0) chunk = 0;

    auto valid_sched = [](const string& s){
        return s=="static" || s=="dynamic" || s=="guided";
    };
    if (!valid_sched(schedule)) {
        cerr << "Warning: invalid schedule → static\n";
        schedule = "static";
    }

    cout << "Matrix: " << fname_short
         << "  Mode=" << mode
         << "  Repeat=" << repeat << "\n";

    if (mode == "omp") {
        cout << "Threads=" << threads
             << "  Schedule=" << schedule
             << "  Requested chunk=" << chunk   // show default as 0
             << "\n";
    }

    // Read & convert matrix
    COOMatrix A_coo = read_matrix_market(filename);
    CSRMatrix A     = coo_to_csr(A_coo);

    // Fixed input vector
    vector<double> x = generate_random_vector(A.cols, 12345);

    vector<double> times;
    vector<double> y;
    vector<double> y_seq;   // Will be used for correctness check

    // Sequential mode
    if (mode == "seq") {
        for (int r = 0; r < repeat; r++) {
            auto t0 = high_resolution_clock::now();
            y = spmv_seq(A, x);
            auto t1 = high_resolution_clock::now();
            times.push_back(duration_cast<microseconds>(t1 - t0).count() / 1000.0);
        }

        // Save reference automatically
        string ref_path = ref_path_from_matrix(filename);
        // create directory if not exists
        std::filesystem::create_directories("results/ref");

        if (!save_vector_binary(ref_path, y)) {
            cerr << "WARNING: failed to save reference: " << ref_path << "\n";
        } else {
            cout << "Saved reference: " << ref_path << "\n";
        }
    }

    // Parallel mode (omp)
    else {

        // Attempt to load reference
        string ref_path = ref_path_from_matrix(filename);
        bool loaded = load_vector_binary(ref_path, y_seq);

        if (!loaded) {
            cout << "Reference not found. Computing sequential reference...\n";
            y_seq = spmv_seq(A, x);

            // ensure directory exists
            std::filesystem::create_directories("results/ref");

            if (!save_vector_binary(ref_path, y_seq)) {
                cerr << "WARNING: failed to save reference\n";
            } else {
                cout << "Saved new reference: " << ref_path << "\n";
            }
        } else {
            cout << "Loaded reference: " << ref_path << "\n";
        }

        // parallel runs
        for (int r = 0; r < repeat; r++) {
            auto t0 = high_resolution_clock::now();
            y = spmv_omp(A, x, threads, schedule, chunk);
            auto t1 = high_resolution_clock::now();
            times.push_back(duration_cast<microseconds>(t1 - t0).count() / 1000.0);
        }
    }

    // Retrieve REAL OpenMP chunk used
    int real_chunk = chunk; // fallback
    if (mode == "omp") {
        omp_sched_t sched_out;
        int chunk_out;
        omp_get_schedule(&sched_out, &chunk_out);
        real_chunk = chunk_out;
    }
    cout << "Real chunk used: " << real_chunk << "\n";

    // Time result
    double p90 = percentile90(times);

    // Correctness test (only in omp mode)
    double max_diff = 0.0;

    if (mode == "omp") {
        for (size_t i = 0; i < y.size(); i++) {
            double d = fabs(y[i] - y_seq[i]);
            if (d > max_diff) max_diff = d;
        }
        cout << "Max abs diff seq vs omp = " << fixed << setprecision(15) << max_diff << "\n";
    }
    else {
        cout << "Correctness check: NA (seq mode)\n";
    }

    // CSV output
    if (!csv_file.empty()) {
        ofstream out(csv_file, ios::app);
        if (!out) {
            cerr << "Error opening CSV file\n";
            return 1;
        }

        out << fname_short << ","
            << mode << ","
            << (mode=="seq" ? 1 : threads) << ","
            << (mode=="seq" ? "na" : schedule) << ","
            << (mode=="seq" ? 0 : real_chunk) << ","
            << repeat << ","
            << fixed << setprecision(6) << p90 << ","
            << setprecision(12) << (mode=="seq" ? 0.0 : max_diff)
            << "\n";

        out.close();
    }

    return 0;
}