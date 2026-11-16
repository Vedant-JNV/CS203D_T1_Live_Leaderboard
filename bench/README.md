# Live Leaderboard Benchmark

## What it measures

For each tree (AVL and unbalanced BST), each arrival order (random and sorted),
and each input size (1,000; 5,000; 10,000; 25,000; 50,000; 100,000), the
program performs five independent runs and records:

- Total insertion time and average nanoseconds per insertion.
- Average nanoseconds per `rank` query and per `select` query (200 queries
  of each kind per run, performed after construction).
- Final tree height, measured after the timed section.
- Total AVL rotations and rotations per inserted player.
- The fixed seed used for each run.

Scores are unique integers from 1 to n. Sorted arrival inserts them in ascending
score order. Random arrival shuffles the same scores with the recorded seed.
IDs are unique and derived from scores. Therefore both trees receive exactly
the same player sequence in each paired run.

## Run

From the project root:

```bash
bash bench/run_benchmarks.sh
```

The script requires a C++17 compiler and Python 3 with matplotlib installed.

Outputs:

- `results/benchmark_raw.csv`: one row per tree per experiment/run.
- `results/benchmark_raw_summary.csv`: mean and sample standard deviation over
  the five runs.
- `results/benchmark_raw_figures/*.png`: insertion, rank, select, height, and AVL rotation
  plots, separately for random and sorted arrival.

## Methodological notes

- Data generation and shuffling occur before timing.
- Only insertion calls are inside the insertion timer.
- Rank and select are timed in separate sections after construction.
- Tree height is measured outside all timed sections. The BST implementation
  computes height by traversing its nodes, so including it in insertion time
  would contaminate the measurement.
- Each run constructs fresh trees. The AVL rotation counter therefore starts
  at zero for every run.
- `steady_clock` is used for elapsed time.
- The same query ranks are supplied to both structures in a paired run.
- The BST implementation's `insert` first calls `contains` and then traverses
  again to insert. The AVL `rank` implementation first calls `findNode` and
  then makes a second traversal to compute the rank, while the BST rank method
  makes one traversal. These are existing implementation details, not pure
  differences in the underlying data structures; disclose them when interpreting
  constant factors.
- Sorted BST insertion is quadratic in n and can take substantially longer at
  n=100,000. Do not interrupt the run merely because that case is slow.

## Interpretation

The height convention in both implementations counts nodes on the longest
root-to-leaf path (empty tree height 0). AVL height should grow logarithmically.
A BST built from sorted arrival should become a chain with height n; random
arrival generally produces a much shorter expected height, but is not guaranteed
to be balanced. Insertion, rank, and select follow a search path, so their
costs are related to the tree height h (AVL h = O(log n); BST O(h)).

Do not write numerical conclusions until you have run the benchmark on the
machine used for the report. Report the compiler, optimization flags, CPU,
operating system, and any background-load caveats alongside the figures.

## Windows (MinGW-w64 / g++)

From PowerShell or Command Prompt at the project root:

```powershell
python -m pip install matplotlib
bench\run_benchmarks.bat
```

If `python` is not on PATH, use the Python launcher (`py`) in the batch file's
plotting command.
