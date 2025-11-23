import sys
import numpy as np
import matplotlib.pyplot as plt
from load_data import load_csv

TARGET_MATRIX = "pdb1HYS.mtx"
THREADS = [24, 96]


def plot_chunk(df):
    # Filtra la matrice, solo dynamic
    sub = df[
        (df["matrix"] == TARGET_MATRIX) &
        (df["schedule"] == "dynamic")
        ]

    # Raggruppa per prendere il p90 migliore per ogni combinazione
    grouped = (
        sub.groupby(["threads", "chunk"])["p90"]
        .min()
        .reset_index()
    )

    plt.figure(figsize=(8, 6))

    for T in THREADS:
        gT = grouped[grouped["threads"] == T]
        gT = gT.sort_values("chunk")

        plt.plot(
            gT["chunk"], gT["p90"],
            marker="o", label=f"{T} threads"
        )

        # aggiungi etichette numeriche
        for x, y in zip(gT["chunk"], gT["p90"]):
            plt.text(x, y, f"{y:.3f}", fontsize=8, ha="center", va="bottom")

    plt.title(f"Chunk size effect — {TARGET_MATRIX} (schedule=dynamic)")
    plt.xlabel("Chunk size")
    plt.ylabel("Time p90 (s)")
    plt.grid(True, linestyle="--", alpha=0.4)
    plt.legend()

    out = f"plots/chunk_effect_{TARGET_MATRIX}.png"
    plt.savefig(out, dpi=200)
    plt.close()
    print(f"[Saved] {out}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 plot_chunk_effect.py benchmark.csv")
        exit(1)

    df = load_csv(sys.argv[1])
    plot_chunk(df)
