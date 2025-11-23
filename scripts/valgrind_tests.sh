#!/bin/bash
#
# =============================================================
#  Valgrind Cachegrind tests for SpMV Project
#  NOTE: This script is NOT executable on the cluster.
#  You must copy/paste each command manually inside an
#  interactive session (qsub -I).
#
#  Purpose:
#   - Provide reproducible instructions to build a Valgrind-safe
#     version of the project (no AVX512)
#   - Provide the exact Cachegrind commands used in the report
#
# =============================================================


# -------------------------------------------------------------
# 1) Load required modules  (copy/paste inside interactive node)
# -------------------------------------------------------------
module purge
module load gcc91
module load cmake-3.15.4
module load numactl
# Valgrind is preinstalled on the nodes (valgrind 3.15.0)
which valgrind


# -------------------------------------------------------------
# 2) Build a Valgrind-safe version of the executable
#    This does NOT affect the main project build.
#    It creates a new folder: build_vg/
# -------------------------------------------------------------

mkdir -p build_vg
cd build_vg

cmake .. -DCMAKE_CXX_FLAGS="-O2 -march=skylake -mno-avx512f -mno-avx512bw -mno-avx512vl"
make -j

cd ..

# The executable to use for Valgrind is now:
#   build_vg/spmv


# -------------------------------------------------------------
# 3) SMALL matrix (baseline) – sequential version
# -------------------------------------------------------------
# 1138_bus.mtx is the small matrix

valgrind --tool=cachegrind --branch-sim=yes \
  ./build_vg/spmv --mode seq --matrix data/1138_bus.mtx --repeat 1 \
  > valgrind_seq_1138_bus.log 2>&1


# -------------------------------------------------------------
# 4) MEDIUM matrix – sequential baseline
# -------------------------------------------------------------
# bcsstk25.mtx is the medium matrix

valgrind --tool=cachegrind --branch-sim=yes \
  ./build_vg/spmv --mode seq --matrix data/bcsstk25.mtx --repeat 1 \
  > valgrind_seq_bcsstk25.log 2>&1


# -------------------------------------------------------------
# 5) IRREGULAR LARGE matrix – static scheduling (24 threads)
# -------------------------------------------------------------

export OMP_NUM_THREADS=24

valgrind --tool=cachegrind --branch-sim=yes \
  ./build_vg/spmv --mode omp --threads 24 --schedule static --chunk 0 \
  --matrix data/pdb1HYS.mtx --repeat 1 \
  > valgrind_pdb1HYS_static_24.log 2>&1


# -------------------------------------------------------------
# 6) IRREGULAR LARGE matrix – dynamic scheduling (24 threads)
# -------------------------------------------------------------

export OMP_NUM_THREADS=24

valgrind --tool=cachegrind --branch-sim=yes \
  ./build_vg/spmv --mode omp --threads 24 --schedule dynamic --chunk 1 \
  --matrix data/pdb1HYS.mtx --repeat 1 \
  > valgrind_pdb1HYS_dynamic_24.log 2>&1


# -------------------------------------------------------------
# 7) IRREGULAR LARGE matrix – static scheduling (96 threads)
# -------------------------------------------------------------

export OMP_NUM_THREADS=96

valgrind --tool=cachegrind --branch-sim=yes \
  ./build_vg/spmv --mode omp --threads 96 --schedule static --chunk 0 \
  --matrix data/pdb1HYS.mtx --repeat 1 \
  > valgrind_pdb1HYS_static_96.log 2>&1


# -------------------------------------------------------------
# 8) IRREGULAR LARGE matrix – dynamic scheduling (96 threads)
# -------------------------------------------------------------

export OMP_NUM_THREADS=96

valgrind --tool=cachegrind --branch-sim=yes \
  ./build_vg/spmv --mode omp --threads 96 --schedule dynamic --chunk 1 \
  --matrix data/pdb1HYS.mtx --repeat 1 \
  > valgrind_pdb1HYS_dynamic_96.log 2>&1
