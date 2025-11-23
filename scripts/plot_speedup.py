import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys
import os

def compute_speedup(df, matrix):
    df_m = df[df["matrix"] == matrix]

    # baseline: seq tempo
    T1 = df_m[(df_m["mode"] == "seq")]["p90"].min()

    curves = {}

    for sched in ["static", "dynamic", "guided"]:
        df_s = df_m[
            (df_m["mode"] == "omp") &
            (df_m["schedule"] == sched) &
            (df_m["chunk"] == 1)       # usa chunk fisso
            ]

        # scegli tempo minimo per ogni numero di threads
        df_group = df_s.groupby("threads")["p90"].min()

        speedup = T1 / df_group
        curves[sched] = speedup

    return curves


def plot_speedup(df, matrix):
    curves = compute_speedup(df, matrix)

    # threads usati realmente in questa matrice
    threads = sorted(df[df["matrix"] == matrix]["threads"].unique())

    plt.figure(figsize=(8, 6))

    colors = {"static": "tab:blue", "dynamic": "tab:red", "guided": "tab:green"}

    # Disegna le curve per ogni tipo di scheduling
    for sched, series in curves.items():
        # "series" è indicizzato per numero di threads
        plt.plot(
            series.index,
            series.values,
            marker='o',
            label=sched,
            color=colors[sched]
        )

        # Etichette sui punti (numero threads)
        for x, y in zip(series.index, series.values):
            plt.text(x, y, str(x), fontsize=8, ha="center", va="bottom")

    # Titoli e assi
    plt.title(f"Speedup — {matrix}")
    plt.xlabel("Threads")
    plt.ylabel("Speedup")

    # Asse X: tutti i thread REALMENTE presenti
    plt.xticks(threads)

    # Asse Y: più tick (8 equidistanti)
    max_y = max(max(series.values) for series in curves.values())
    yticks = [round(v, 2) for v in list(np.linspace(0, max_y, 8))]
    plt.yticks(yticks)

    plt.grid(True, linestyle="--", alpha=0.4)
    plt.legend()

    os.makedirs("plots", exist_ok=True)
    out = f"plots/speedup_{matrix}.png"
    plt.savefig(out, dpi=200)
    plt.close()
    print(f"Saved: {out}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 plot_speedup.py <csv_file>")
        sys.exit(1)

    csv_file = sys.argv[1]

    df = pd.read_csv(csv_file)

    matrices = df["matrix"].unique()
    for m in matrices:
        plot_speedup(df, m)