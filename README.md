#  Live_leaderboard
 Live leaderboard  with rank queries
## Application (leaderboard program)

`application.cpp` replays an operation file against either tree:
- `--structure A` uses the AVL tree (`Structure A/avl.cpp`)
- `--structure B` uses the unbalanced BST (`Structure B/bst.cpp`)

Both structures must print identical output. A custom separate-chaining hash
table (player id -> current score) is built into `application.cpp`.

### Requirements
A C++17 compiler (`g++`). On Windows, install MSYS2/MinGW-w64 and add it to PATH.

### Build (run from the repo root)
```
g++ -std=c++17 -O2 -I"Structure A" -I"Structure B" application.cpp "Structure A/avl.cpp" "Structure B/bst.cpp" -o leaderboard
```
This produces `leaderboard.exe` on Windows and `leaderboard` on Linux/Mac.

### Run
Windows PowerShell:
```
Get-Content ops.txt | .\leaderboard.exe --structure A
Get-Content ops.txt | .\leaderboard.exe --structure B
```
Linux / Mac / Git Bash:
```
./leaderboard --structure A < ops.txt
./leaderboard --structure B < ops.txt
```

### Input format (one command per line)
| Command | Meaning |
|---|---|
| `U <id> <score>` | set a player's score (adds the player if new) |
| `R <id>` | print the player's rank (ties share a rank: 1, 2, 2, 4); `NOT_FOUND` if absent |
| `S <k>` | print the id at position k (1 = top); `-1` if out of range |
| `L <a> <b>` | print the ids at positions a..b, space-separated (clamped to valid range) |
| `D <id>` | remove the player (no output if absent) |

### Expected output for the sample `ops.txt`
```
2
2
1 2 3 4
1 3 4
```
Running with `--structure A` and `--structure B` must give exactly this.

### Notes
- Ordering: higher score first; ties by lower player id.
- Sorted-order input makes Structure B a long chain, so it is much slower than A on large inputs (this is expected).