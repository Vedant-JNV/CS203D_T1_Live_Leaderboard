# CS 203D Project - Live Leaderboard with Rank Queries

**PS : Size-Augmented AVL Tree vs Unbalanced Binary Search Tree**

A leaderboard system that maintains player scores and efficiently supports rank queries, positional selection, range listing, insertion, updates, and deletion. This project compares a self-balancing AVL Tree with an ordinary unbalanced Binary Search Tree (BST), both augmented with subtree sizes.

## Team Members and Roles

| Member | Role | Roll No. | GitHub |
|---|---|---|---|
| Vedant | Structure A - AVL Tree | 250021019 | Vedant-JNV |
| Somya | Structure B - BST | 250021018 | somyama4753l-lang |
| Imran | Application and Interface | 250051009 | imran51107 |
| Swarnika | Testing, Oracle and Invariant Checker | 250005048 | LadyRhanes |
| Aarushi | Measurement and Benchmarking | 250051001 | Aarushi-1408 |

---

## 1. Project Overview

The project implements a live leaderboard for an online game where player scores are continuously updated. The leaderboard supports:

- Finding the rank of a player.
- Finding the player at a given position.
- Listing players within a range of positions.
- Updating player scores.
- Removing players.

Two data structures are implemented and compared:

- **Structure A:** Size-augmented AVL Tree, which maintains balance using rotations.
- **Structure B:** Size-augmented unbalanced Binary Search Tree, which does not perform balancing.

Both structures use identical ordering rules and support the same leaderboard operations.

### Ordering Rules

Players are ordered by:

1. Higher score first.
2. For equal scores, lower player ID first.

Players with equal scores receive the same competition rank, and the next rank is skipped accordingly.

| Player ID | Score | Rank |
|---:|---:|---:|
| 1 | 100 | 1 |
| 2 | 90 | 2 |
| 3 | 90 | 2 |
| 4 | 80 | 4 |

The ranking follows competition ranking: **1, 2, 2, 4**.

---

## 2. Project Structure

```text
Live_leaderboard/
│
├── application.cpp
├── ops.txt
├── gui.py
│
├── Structure A/
│   ├── avl.cpp
│   ├── avl.hpp
│   └── test.cpp
│
├── Structure B/
│   ├── bst.cpp
│   └── bst.hpp
│
├── test/
│   ├── oracle.cpp
│   ├── oracle.hpp
│   ├── adapters.hpp
│   ├── invariant.hpp
│   ├── differential_test.cpp
│   ├── rotation_sim.hpp
│   ├── types.hpp
│   └── worst_case.cpp
│
├── bench/
│   ├── benchmark.cpp
│   ├── plot_results.py
│   ├── run_benchmarks.sh
│   └── run_benchmarks.bat
│
└── results/
    ├── benchmark_raw.csv
    ├── benchmark_raw_summary.csv
    └── benchmark_raw_figures/
```

---

## 3. Requirements

- **Language:** C++
- **Standard:** C++17
- **Compiler:** g++ (MinGW-w64/MSYS2 on Windows, or GCC on Linux)
- **Python:** Python 3
- **Python package:** matplotlib
- **GUI:** Tkinter

On Windows, a MinGW-w64 installation with `g++` added to PATH is recommended.

Install the plotting dependency:

```bash
python -m pip install matplotlib
```

Verify that Tkinter is available:

```bash
python -m tkinter
```

---

## 4. Building the Application

Run the following command from the project root directory:

```bash
g++ -std=c++17 -O2 -I"Structure A" -I"Structure B" application.cpp "Structure A/avl.cpp" "Structure B/bst.cpp" -o run.exe
```

This builds the leaderboard application and produces `run.exe` on Windows.

---

## 5. Running the Command-Line Application

The application reads operations from standard input and prints the result of each query to standard output.

### Windows PowerShell

Run Structure A (AVL):

```powershell
Get-Content ops.txt | .\run.exe --structure A
```

Run Structure B (BST):

```powershell
Get-Content ops.txt | .\run.exe --structure B
```

### Linux / macOS / Git Bash

Run Structure A:

```bash
./run --structure A < ops.txt
```

Run Structure B:

```bash
./run --structure B < ops.txt
```

Both structures are expected to produce identical outputs for the same supported input.

---

## 6. Input Format

Each operation is supplied on a separate line.

| Command | Description |
|---|---|
| `U <id> <score>` | Insert a new player or update an existing player's score. |
| `R <id>` | Print the player's rank. Prints `NOT_FOUND` if the player does not exist. |
| `S <k>` | Print the player ID at position `k`, where 1 represents the top position. Prints `-1` if out of range. |
| `L <a> <b>` | Print player IDs occupying positions `a` through `b`, separated by spaces. The range is clamped to valid positions. |
| `D <id>` | Delete the player. If the player does not exist, no output is produced. |

### Example Input

```text
U 1 100
U 2 90
U 3 90
U 4 80
R 3
S 2
L 1 4
D 2
L 1 4
```

### Expected Output

```text
2
2
1 2 3 4
1 3 4
```

---

## 7. Data Structures

### Structure A - AVL Tree

The AVL Tree is a self-balancing Binary Search Tree. Each node stores player information, subtree size, and height.

After insertion or deletion, rotations are performed when necessary to maintain the AVL balance condition.

Subtree sizes allow rank and select queries to be performed without traversing the entire tree.

| Operation | Complexity |
|---|---|
| Insert | O(log n) |
| Delete | O(log n) |
| Rank | O(log n) |
| Select | O(log n) |
| Range listing | O(log n + k) approximately |

Here, `k` is the number of players returned in the range.

### Structure B - Unbalanced BST

The ordinary BST uses the same ordering and subtree-size augmentation as Structure A, but does not perform rotations or maintain balance.

Its performance depends on the order in which elements are inserted. In an unfavourable insertion order, the tree can become a chain.

| Operation | Average / Expected | Worst Case |
|---|---|---|
| Insert | O(h) | O(n) |
| Delete | O(h) | O(n) |
| Rank | O(h) | O(n) |
| Select | O(h) | O(n) |

Here, `h` is the height of the tree. In a reasonably balanced tree, h is logarithmic, but it can reach n in the worst case.

### Application and Hash Table

The application provides a common interface to both trees and replays the input operations.

A custom hash table using separate chaining stores player IDs and their current scores. When a player's score is updated, their previous entry is removed from the tree and the updated entry is inserted.

The hash table provides average O(1) player lookup, while the trees maintain leaderboard ordering and support rank-based operations.

---

## 8. Graphical User Interface

A graphical interface built using **Python Tkinter** has been added to make the project easier to interact with, demonstrate, test, and benchmark without manually using the terminal for every operation.

The GUI contains three tabs.

### Live Demo

Allows interactive demonstration of the leaderboard operations.

- Switch between AVL Tree (Structure A) and BST (Structure B).
- Add or update a player's score.
- Delete a player.
- Find a player's rank.
- Select the player at a given position.
- Query a range of leaderboard positions.
- Clear the demo and start again.
- View the leaderboard in a table showing rank, player ID, and score.

The GUI sends operations to the compiled C++ application. The leaderboard table is displayed in the GUI, while query results are generated by the C++ implementation.

### Testing

Runs the existing differential and invariant testing executable from the interface and displays the test output.

The test compares both tree implementations with the reference oracle and checks structural invariants.

### Benchmark

Launches the benchmark script from the GUI.

- Runs the benchmark workflow.
- Displays the benchmark output.
- Runs in the background to keep the GUI responsive.
- Generates benchmark figures.
- Displays the generated performance plots in a graphical dashboard.

### Running the GUI

From the project root:

```bash
python gui.py
```

Python and Tkinter must be installed and available.

The GUI does not replace the C++ data structures. It acts as a graphical interface to the existing application, testing, and benchmarking components.

---

## 9. Testing

Testing is performed using a reference oracle, differential testing, and invariant checking.

### Reference Oracle

A sorted array-based reference implementation is used to verify expected results. It provides a simpler correctness model, with linear-time updates.

### Differential Testing

Randomly generated operations are executed against:

- The reference oracle.
- Structure A (AVL Tree).
- Structure B (unbalanced BST).

The outputs are compared to identify any discrepancies.

The test supports configurable operation counts, seeds, test configurations, and invariant-checking frequency.

Build the differential test from the project root:

```bash
g++ -std=c++17 -O2 -I"Structure A" -I"Structure B" test/differential_test.cpp "Structure A/avl.cpp" "Structure B/bst.cpp" -o difftest.exe
```

Run:

```powershell
.\difftest.exe
```

### Custom Test Configurations

Run only the ties configuration:

```powershell
.\difftest.exe --only ties --ops 100000 --check-every 500
```

Run with a custom seed and operation count:

```powershell
.\difftest.exe --seed 7 --ops 200000
```

### Invariant Checker

The invariant checker verifies:

- Correct BST ordering.
- Correct subtree sizes.
- Correct stored heights in the AVL Tree.
- Valid AVL balance factors (−1, 0, or 1).
- Structural correctness of the trees after operations.

### Worst-Case Testing

Additional tests examine unfavourable insertion orders and tree behaviour, including the degeneration of the unbalanced BST under sorted insertions.

---

## 10. Benchmarking

The benchmark compares AVL and BST performance for random and sorted insertion sequences.

### Experimental Parameters

- Input sizes: 1,000; 5,000; 10,000; 25,000; 50,000; and 100,000 players.
- Five runs per configuration.
- Random and sorted insertion orders.
- Recorded random seeds.
- 200 rank queries and 200 select queries per run after tree construction.

### Metrics

- Total insertion time.
- Average insertion time per player (ns).
- Average rank query time (ns).
- Average select query time (ns).
- Final tree height.
- Total AVL rotations.
- AVL rotations per insertion.

### Running the Benchmark

#### Linux / macOS / Git Bash

From the project root:

```bash
bash bench/run_benchmarks.sh
```

#### Windows

From PowerShell or Command Prompt:

```powershell
bench\run_benchmarks.bat
```

The benchmark script compiles the benchmark program, runs the experiments, generates raw CSV measurements, and creates plots.

Sorted BST insertion is quadratic in n and can take substantially longer at 100,000 elements. Do not interrupt the run merely because that case is slow.

### Benchmark Outputs

- `results/benchmark_raw.csv` - raw measurements from individual runs.
- `results/benchmark_raw_summary.csv` - mean and sample standard deviation over the five runs.
- `results/benchmark_raw_figures/` - generated plots for insertion time, rank time, select time, tree height, and AVL rotations, separately for random and sorted arrival.
<<<<<<< HEAD
- `results/team_original_run/` - an earlier benchmark run by the team on different hardware, used for comparison in the report.
=======
>>>>>>> 7cc6afcb87564b55a3c903e0a7663221f6cec80e

---

## 11. Benchmark Interpretation

The AVL Tree maintains logarithmic height through balancing, while the unbalanced BST depends on insertion order.

For random insertion, the ordinary BST can perform reasonably well, although it is not guaranteed to remain balanced.

For sorted insertion, the BST can degenerate into a chain of height n, causing insertion and query performance to deteriorate significantly. The AVL Tree avoids this by maintaining balance through rotations.

Benchmark results are hardware-dependent and should be interpreted alongside the raw measurements and figures.

---

## 12. Known Limitations

- The unbalanced BST can have O(n) height and O(n) individual operation time under unfavourable insertion orders.
- Sorted BST construction can take O(n²) total time and becomes very slow at large input sizes.
- Benchmark timings depend on hardware, compiler, optimization settings, and system load.
- Random insertion does not guarantee a balanced BST.
- The benchmark focuses on insertion, rank, select, height, and AVL rotations; it is not a complete real-world game server performance simulation.
- Range listing can take time proportional to the number of players returned, in addition to tree traversal overhead.
- Both structures are designed for the project requirements and are not intended to provide concurrent or thread-safe updates.
- The GUI requires Python and Tkinter to be available.
- Benchmark execution from the GUI may take considerable time for large input sizes.

---

## 13. Reproducibility

The benchmark implementation and plotting scripts are available in the `bench/` directory.

| File | Description |
|---|---|
| `benchmark.cpp` | Benchmark implementation |
| `plot_results.py` | Graph generation script |
| `run_benchmarks.sh` | Linux/macOS/Git Bash benchmark runner |
| `run_benchmarks.bat` | Windows benchmark runner |
| `results/benchmark_raw.csv` | Raw experimental measurements |
| `results/benchmark_raw_summary.csv` | Summary statistics |
| `results/benchmark_raw_figures/` | Generated performance graphs |

For a reproducible performance report, record the compiler and version, optimization flags, CPU, operating system, and background system load.

---

## 14. References

- Adelson-Velsky, G. M., and Landis, E. M. “An algorithm for the organization of information.” *Soviet Mathematics Doklady*, 3, 1259-1263, 1962.
- Cormen, T. H., Leiserson, C. E., Rivest, R. L., and Stein, C. *Introduction to Algorithms*, 3rd ed., MIT Press, 2009. Chapters 12, 13, and §14.1.
- Sedgewick, R., and Wayne, K. *Algorithms*, 4th ed., Addison-Wesley, 2011. §3.2.

---

## 15. AI and Tool Usage

AI tools were used for assistance with documentation, explanations, code review, debugging, GUI development, and test/benchmark interpretation. The project implementation was reviewed and modified by the team, and team members are responsible for understanding and explaining their own code and design decisions.

See `TOOLS.md` for the detailed tool-use declaration.
