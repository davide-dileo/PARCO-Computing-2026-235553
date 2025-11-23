import sys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

from load_data import load_csv
from compute_metrics import compute_speedup


def compute_efficiency_fixed(df, matrix):
    """
    Restituisce un DataFrame con colonne:
    schedule | threads | efficiency
    (chunk=1 fisso)
    """

    df_m = df[(df["matrix"] == matrix)]

    # tempo sequenziale
    T1 = df_m[df_m["mode"] == "seq"]["p90"].min()

    # filtra solo omp, chunk = 1
    df_omp = df_m[
        (df_m["mode"] == "omp") &
        (df_m["chunk"] == 1)
        ]

    # per ogni combinazione (schedule, threads) scegli il p90 migliore
    grouped = df_omp.groupby(["schedule", "threads"])["p90"].min().reset_index()

    # calcolo speedup ed efficiency
    grouped["speedup"] = T1 / grouped["p90"]
    grouped["efficiency"] = grouped["speedup"] / grouped["threads"]

    return grouped


def plot_eff(df, matrix):

    eff = compute_efficiency_fixed(df, matrix)

    plt.figure(figsize=(8, 6))

    schedules = ["static", "dynamic", "guided"]
    colors = {
        "static": "tab:blue",
        "dynamic": "tab:red",
        "guided": "tab:green"
    }

    for sched in schedules:
        s = eff[eff["schedule"] == sched]
        if len(s) == 0:
            continue

        plt.plot(
            s["threads"], s["efficiency"],
            marker="o", label=sched, color=colors.get(sched, None)
        )

        # etichette dei punti
        for x, y in zip(s["threads"], s["efficiency"]):
            plt.text(x, y, str(x), fontsize=8, ha="center", va="bottom")

    # ticks x
    threads = sorted(eff["threads"].unique())
    plt.xticks(threads)

    # ticks y – più densi
    max_y = eff["efficiency"].max()
    yt = np.linspace(0, max_y, 8)
    plt.yticks(yt)

    plt.title(f"Parallel Efficiency — {matrix}")
    plt.xlabel("Threads")
    plt.ylabel("Efficiency")
    plt.grid(True, linestyle="--", alpha=0.4)
    plt.legend()

    out = f"plots/efficiency_{matrix}.png"
    plt.savefig(out, dpi=200)
    plt.close()
    print(f"[Saved] {out}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 plot_efficiency.py benchmark.csv")
        exit(1)

    df = load_csv(sys.argv[1])
    matrices = df["matrix"].unique()

    for m in matrices:
        plot_eff(df, m)
