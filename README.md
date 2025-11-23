# Sparse Matrix–Vector Multiplication Benchmarking Project

This repository contains the complete implementation and experimental framework for a performance study of **Sparse Matrix–Vector Multiplication (SpMV)** using **OpenMP**.  
The project evaluates scalability, scheduling strategies, NUMA effects, and cache behavior on a multi-socket HPC system.

The repository is designed to be **fully reproducible**.

---

## Repository Structure

```
PARCO-Computing-2026-235553/
│
├── data/                 # Input matrices (Matrix Market) - NOT verioned
├── plots/                # Generated figures
├── results/
│   ├── csv/              # Benchmark outputs
│   └── valgrind/         # Cachegrind logs
│
├── scripts/              # PBS job + plotting utilities
├── src/                  # C++ implementation (COO → CSR + SpMV)
├── report/               # Final project report
└── CMakeLists.txt
```

---

## 1. Project Overview

The project investigates the performance of OpenMP-parallel SpMV across:

- **Sparse matrices** of different sizes and sparsity patterns
- **Scheduling strategies:** `static`, `dynamic`, `guided`
- **Chunk sizes:** `0` (default), `1`, `4`, `16`
- **Thread counts:** up to 96 threads (full NUMA node)
- **Cache behavior** via Valgrind/Cachegrind
- **NUMA placement effects** on large matrices

The analysis focuses on:

- scaling and saturation effects
- irregular sparsity and load imbalance
- the sensitivity of dynamic scheduling to chunk size
- cache-miss patterns in large/irregular matrices

---

## 2. Input Matrices (Download Required)

The matrices used in this project are **not included in the repository** because several exceed GitHub’s file size limit (100 MB).  
You must download them manually from the **SuiteSparse Matrix Collection**:

➡ https://sparse.tamu.edu/

Download the following matrices and place them into the directory:

```
data/
```

### Required matrices:

| Name          | Size           | Nonzeros | Category            | Notes                           |
|---------------|----------------|----------|----------------------|----------------------------------|
| **1138_bus**   | 1k × 1k       | 4k       | Power network        | Small, regular                   |
| **bcsstk25**   | 15k × 15k     | 252k     | Structural           | Medium, moderately regular       |
| **G3_circuit** | 1.6M × 1.6M   | 7.6M     | Circuit simulation   | Large, irregular                 |
| **nlpkkt120**  | 3.5M × 3.5M   | 95M      | Optimization         | Very large, highly irregular     |
| **pdb1HYS**    | 36k × 36k     | 4.3M     | Weighted graph       | Medium-large, irregular          |

Your `data/` folder should look like:

```
data/
│
├── 1138_bus.mtx
├── bcsstk25.mtx
├── G3_circuit.mtx
├── nlpkkt120.mtx
└── pdb1HYS.mtx
```

---

## 3. Building the Project

### Linux / macOS / WSL2

```bash
git clone <repo>
cd PARCO-Computing-2026-235553
mkdir build && cd build
cmake ..
make -j
```

### Windows

Native Windows builds are possible but **not recommended** due to limited OpenMP and Valgrind support.

Recommended alternatives:

- **WSL2 (Ubuntu):** follow the Linux instructions above
- Any Linux-compatible environment with **GCC ≥ 9** and **CMake ≥ 3.15**

---

## 4. Running Benchmarks on the HPC Cluster

Benchmarks run on a **4-socket machine (96 cores total)** using PBS.

### Submit full experiment campaign

Ensure the **working directory is the project root**, then run:

```bash
qsub scripts/run_benchmark.pbs
```

The script:

- loads required modules (`gcc91`, `cmake`, `numactl`)
- compiles the project on the compute node
- runs all combinations of:
    - **threads:** `1,2,4,8,12,16,24,48,96`
    - **schedules:** `static`, `dynamic`, `guided`
    - **chunk sizes:** `0,1,4,16`
- writes benchmark results into:

```
results/csv/benchmark_YYYYMMDD_HHMMSS.csv
```

### NUMA policy

The PBS script sets:

```bash
export OMP_PLACES=cores
export OMP_PROC_BIND=true
```

to reduce thread migration and improve reproducibility.

---

## 5. Valgrind / Cachegrind

Valgrind was executed in **interactive mode**:

```bash
qsub -I -l select=1:ncpus=96:mem=8gb
```

Commands used to generate cache-miss logs are listed in:

```
scripts/valgrind_tests.sh
```

Generated logs are stored in:

```
results/valgrind/
```

---

## 6. Plot Generation

All plots are generated with Python.

Example (local machine or Google Colab):

```bash
pip install pandas matplotlib
python3 scripts/plot_speedup.py results/csv/<file>.csv
python3 scripts/plot_efficiency.py ...
python3 scripts/plot_schedule_comparison.py ...
python3 scripts/plot_chunk_effect.py ...
```

Outputs are stored in:

```
plots/
```

---

## Author and Course Information

**Author:** Davide Di Leo  
**Course:** Introduction to Parallel Computing (PARCO)  
**University:** University of Trento (UniTN)  
**Instructor:** Prof. Flavio Vella  
**Academic Year:** 2025/2026

This project was developed as the **D1 deliverable** on performance analysis of parallel algorithms.