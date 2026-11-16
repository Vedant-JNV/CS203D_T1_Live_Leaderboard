#!/usr/bin/env python3
"""Summarize benchmark_raw.csv and create report-ready plots."""
import csv
import os
import sys
import statistics
from collections import defaultdict

import matplotlib.pyplot as plt

INPUT = sys.argv[1] if len(sys.argv) > 1 else "results/benchmark_raw.csv"
BASE = os.path.splitext(os.path.basename(INPUT))[0]
RESULTS_DIR = os.path.dirname(os.path.abspath(INPUT)) or "."
SUMMARY_PATH = os.path.join(RESULTS_DIR, f"{BASE}_summary.csv")
OUTDIR = os.path.join(RESULTS_DIR, f"{BASE}_figures")
os.makedirs(OUTDIR, exist_ok=True)

with open(INPUT, newline="", encoding="utf-8") as f:
    rows = list(csv.DictReader(f))
if not rows:
    raise SystemExit(f"No benchmark rows found in {INPUT}")

numeric = ["insertion_total_ms", "insertion_ns_per_op", "rank_ns_per_op",
           "select_ns_per_op", "height", "rotations", "rotations_per_insert"]
groups = defaultdict(list)
for row in rows:
    groups[(row["structure"], row["arrival"], int(row["n"]))].append(row)

summary_path = SUMMARY_PATH
with open(summary_path, "w", newline="", encoding="utf-8") as f:
    fields = ["structure", "arrival", "n"]
    for col in numeric:
        fields += [col + "_mean", col + "_stdev"]
    writer = csv.DictWriter(f, fieldnames=fields)
    writer.writeheader()
    for (structure, arrival, n), group in sorted(groups.items()):
        result = {"structure": structure, "arrival": arrival, "n": n}
        for col in numeric:
            values = [float(r[col]) for r in group]
            result[col + "_mean"] = statistics.mean(values)
            result[col + "_stdev"] = statistics.stdev(values) if len(values) > 1 else 0.0
        writer.writerow(result)

summary = []
with open(summary_path, newline="", encoding="utf-8") as f:
    summary = list(csv.DictReader(f))

metrics = [
    ("insertion_ns_per_op", "Mean insertion time (ns per insertion)", "insertion_time"),
    ("rank_ns_per_op", "Mean rank query time (ns per operation)", "rank_time"),
    ("select_ns_per_op", "Mean select query time (ns per operation)", "select_time"),
    ("height", "Mean final tree height (nodes)", "tree_height"),
    ("rotations_per_insert", "Mean AVL rotations per insertion", "rotations_per_insert"),
]

for metric, ylabel, filename in metrics:
    for arrival in ("random", "sorted"):
        plt.figure()
        structures = ("AVL",) if metric == "rotations_per_insert" else ("AVL", "BST")
        for structure in structures:
            subset = sorted((r for r in summary
                             if r["arrival"] == arrival and r["structure"] == structure),
                            key=lambda r: int(r["n"]))
            if subset:
                xs = [int(r["n"]) for r in subset]
                ys = [float(r[metric + "_mean"]) for r in subset]
                plt.plot(xs, ys, marker="o", label=structure)
        plt.xscale("log")
        if metric not in ("height", "rotations_per_insert"):
            plt.yscale("log")
        plt.xlabel("Number of players (n)")
        plt.ylabel(ylabel)
        plt.title(f"{ylabel} — {arrival.capitalize()} arrival")
        plt.grid(True, which="both", alpha=0.3)
        plt.legend()
        plt.tight_layout()
        plt.savefig(os.path.join(OUTDIR, f"{filename}_{arrival}.png"), dpi=200)
        plt.close()

print(f"Wrote {summary_path}")
print(f"Wrote plots to {OUTDIR}/")
