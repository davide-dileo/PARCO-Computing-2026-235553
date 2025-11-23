import pandas as pd

def compute_speedup(df, matrix):
    """
    Computes speedup for all OMP configurations of a given matrix.
    Speedup = T_seq / T_p90
    """
    sub = df[df["matrix"] == matrix]

    # Sequential baseline
    T_seq = sub[sub["mode"] == "seq"]["p90"].iloc[0]

    omp_runs = sub[sub["mode"] == "omp"].copy()
    omp_runs["speedup"] = T_seq / omp_runs["p90"]

    return omp_runs, T_seq


def compute_efficiency(df):
    """
    Efficiency = speedup / threads
    Adds an 'efficiency' column assuming speedup was computed.
    """
    df = df.copy()
    df["efficiency"] = df["speedup"] / df["threads"]
    return df