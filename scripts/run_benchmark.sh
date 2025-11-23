#!/usr/bin/env bash
set -euo pipefail

# =============================================================
#  SpMV Benchmark (local execution)
#  Introduction to Parallel Computing (2025/26)
# =============================================================

# Resolve project root directory (works from anywhere)
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"

EXEC="$ROOT_DIR/build/spmv"
DATA_DIR="$ROOT_DIR/data"
OUT_DIR="$ROOT_DIR/results/csv"

# Create results folder if needed
mkdir -p "$OUT_DIR"

if [[ ! -f "$EXEC" ]]; then
    echo "ERROR: Executable not found at: $EXEC"
    echo "Compile first using:"
    echo "    mkdir build && cd build"
    echo "    cmake .."
    echo "    make -j"
    exit 1
fi

echo "============================================"
echo "      Running SpMV Benchmarks (local)"
echo "============================================"
echo "Executable: $EXEC"
echo "Data folder: $DATA_DIR"
echo ""

# --------------------------------------------
# MATRICES TO USE
# --------------------------------------------
MATRICES=(
    "1138_bus.mtx"
    "bcsstk25.mtx"
    "G3_circuit.mtx"
    "nlpkkt120.mtx"
    "pdb1HYS.mtx"
)

# --------------------------------------------
# Variables to test for parallel runs
# Adjust according to your machine
# --------------------------------------------
THREADS=(1 2 4 8 16)
SCHEDULES=("static" "dynamic" "guided")
CHUNKS=(0 1 4 16)   # 0 means omp default
REPEAT=10

TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
CSV_OUT="$OUT_DIR/benchmark_$TIMESTAMP.csv"

echo "Saving results to: $CSV_OUT"
echo ""

# Write CSV header
echo "matrix,mode,threads,schedule,chunk,repeat,p90,max_diff" > "$CSV_OUT"

# --------------------------------------------
# MAIN LOOP
# --------------------------------------------
for MAT in "${MATRICES[@]}"; do
    MATRIX_PATH="$DATA_DIR/$MAT"

    if [[ ! -f "$MATRIX_PATH" ]]; then
        echo "WARNING: Matrix not found: $MATRIX_PATH"
        continue
    fi

    echo ">>> Matrix: $MAT"

    # Sequential baseline once per matrix
    echo "  Running sequential..."
    $EXEC --mode seq --matrix "$MATRIX_PATH" --repeat "$REPEAT" --csv "$CSV_OUT"

    # Parallel runs
    for T in "${THREADS[@]}"; do
        for S in "${SCHEDULES[@]}"; do
            for C in "${CHUNKS[@]}"; do
                echo "  Running OMP: threads=$T schedule=$S chunk=$C"
                $EXEC --mode omp --matrix "$MATRIX_PATH" \
                      --threads "$T" \
                      --schedule "$S" \
                      --chunk "$C" \
                      --repeat "$REPEAT" \
                      --csv "$CSV_OUT"
            done
        done
    done

    echo ""
done

echo "============================================"
echo "     Benchmark Completed Successfully"
echo "============================================"
echo "CSV saved in: $CSV_OUT"