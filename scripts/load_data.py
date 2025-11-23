import pandas as pd
import sys


def load_csv(path):
    """
    Loads the benchmark CSV produced by the PBS run.
    Normalizes modes and schedules, ensures numeric types.
    """
    df = pd.read_csv(path)

    # Clean mode column
    df["mode"] = df["mode"].str.strip().str.lower()

    # Ensure numeric columns
    df["threads"] = pd.to_numeric(df["threads"], errors="coerce")
    df["chunk"] = pd.to_numeric(df["chunk"], errors="coerce")
    df["p90"] = pd.to_numeric(df["p90"], errors="coerce")

    return df


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 load_data.py path/to/benchmark.csv")
        exit(1)

    df = load_csv(sys.argv[1])
    print(df.head())