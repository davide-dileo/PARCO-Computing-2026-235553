import sys
import numpy as np
import matplotlib.pyplot as plt
from load_data import load_csv


THREADS_TO_COMPARE = [24, 96]
SCHEDULES = ["static", "dynamic", "guided"]


def plot_schedule(df, matrix):

    # Filtra la matrice, solo OMP, solo chunk=1
    sub = df[
        (df["matrix"] == matrix) &
        (df["mode"] == "omp") &
        (df["chunk"] == 1)
        ]

    # Raggruppa per (threads, schedule) e prendi il p90 migliore
    grouped = (
        sub.groupby(["threads", "schedule"])["p90"]
        .min()
        .reset_index()
    )

    # Considera solo i 3 schedule principali
    grouped = grouped[grouped["schedule"].isin(SCHEDULES)]

    plt.figure(figsize=(8, 6))

    colors = {
        "static": "tab:blue",
        "dynamic": "tab:red",
        "guided": "tab:green"
    }

    # Per ogni T, plottiamo un punto per ogni schedule
    for T in THREADS_TO_COMPARE:
        gT = grouped[grouped["threads"] == T]

        # ordina per schedule nel giusto ordine
        gT = gT.set_index("schedule").loc[SCHEDULES].reset_index()

        plt.plot(
            gT["schedule"],
            gT["p90"],
            marker="o",
            label=f"{T} threads"
        )

        # Etichette numeriche
        for x, y in zip(gT["schedule"], gT["p90"]):
            plt.text(x, y, f"{y:.3f}", fontsize=8, ha="center", va="bottom")

    plt.title(f"Schedules comparison — {matrix} (chunk = 1)")
    plt.xlabel("Schedule")
    plt.ylabel("Time p90 (s)")
    plt.grid(True, linestyle="--", alpha=0.4)
    plt.legend()

    out = f"plots/schedule_{matrix}.png"
    plt.savefig(out, dpi=200)
    plt.close()
    print(f"[Saved] {out}")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 plot_schedule_comparison.py benchmark.csv")
        exit(1)

    df = load_csv(sys.argv[1])
    matrices = df["matrix"].unique()

    for m in matrices:
        plot_schedule(df, m)
