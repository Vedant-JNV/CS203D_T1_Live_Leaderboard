#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p results

g++ -std=c++17 -O2 -DNDEBUG -Wall -Wextra \
    bench/benchmark.cpp \
    "Structure A/avl.cpp" \
    "Structure B/bst.cpp" \
    -o bench/benchmark

./bench/benchmark results/benchmark_raw.csv
python3 bench/plot_results.py results/benchmark_raw.csv
