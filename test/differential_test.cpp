#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "adapters.hpp"
#include "oracle.hpp"

using namespace std;

// ------------------------------------------------------------------
// Operations
// ------------------------------------------------------------------
struct Op {
    char type;            // U R S L D
    long long a = 0, b = 0;   // U: a=id b=score | R: a=id | S: a=k | L: a,b | D: a=id
};

string opText(const Op& op) {
    switch (op.type) {
        case 'U': return "U " + to_string(op.a) + " " + to_string(op.b);
        case 'R': return "R " + to_string(op.a);
        case 'S': return "S " + to_string(op.a);
        case 'L': return "L " + to_string(op.a) + " " + to_string(op.b);
        default:  return "D " + to_string(op.a);
    }
}

string joinIds(const vector<PlayerId>& ids) {
    string s;
    for (size_t i = 0; i < ids.size(); i++) { if (i) s += ' '; s += to_string(ids[i]); }
    return s;
}

// ------------------------------------------------------------------
// Configurations
// ------------------------------------------------------------------
enum Mode { RANDOM, SORTED_ASC, SORTED_DESC };

struct Config {
    string name;
    Mode mode;
    int sortedPlayers;     // players inserted in sorted order first (0 for RANDOM)
    int ops;               // random mixed operations (0 = use --ops)
    long long idMax;
    Score scoreMin, scoreMax;
};

vector<Config> allConfigs() {
    return {
        // few distinct scores -> lots of ties, many updates and deletes, negative scores
        {"ties",        RANDOM,      0,     0, 2000,  -25, 25},
        // wide scores, bigger tree, few ties
        {"wide",        RANDOM,      0,     0, 5000,  0, 1000000000LL},
        // sorted arrival: BST degenerates into a chain, then mixed operations
        {"sorted-asc",  SORTED_ASC,  10000, 20000, 10000, -10000, 10000},
        {"sorted-desc", SORTED_DESC, 10000, 20000, 10000, -10000, 10000},
    };
}

class Generator {
    mt19937_64 rng;
    unordered_set<PlayerId> present;   // approximate: lets S/L ask about sensible positions

    long long rnd(long long lo, long long hi) {
        return uniform_int_distribution<long long>(lo, hi)(rng);
    }

public:
    explicit Generator(uint64_t seed) : rng(seed) {}

    Op sortedInsert(const Config& c, int i) {
        present.insert(i);
        Score s = (c.mode == SORTED_ASC) ? i : -i;
        return {'U', i, s};
    }

    Op randomOp(const Config& c) {
        long long size = (long long)present.size();
        int roll = (int)rnd(0, 99);
        if (roll < 40) {                                  // U
            PlayerId id = rnd(1, c.idMax);
            present.insert(id);
            return {'U', id, rnd(c.scoreMin, c.scoreMax)};
        } else if (roll < 60) {                           // R (some ids absent)
            return {'R', rnd(1, c.idMax + 5), 0};
        } else if (roll < 75) {                           // S (0 and size+1, size+2 are invalid)
            return {'S', rnd(0, size + 2), 0};
        } else if (roll < 85) {                           // L (sometimes a > b or out of range)
            long long a = rnd(0, size + 1);
            long long len = (rnd(0, 3) == 0) ? rnd(0, size + 2) : rnd(0, 20);
            return {'L', a, a + len - rnd(0, 2)};
        } else {                                          // D (some ids absent)
            PlayerId id = rnd(1, c.idMax + 5);
            present.erase(id);
            return {'D', id, 0};
        }
    }
};

vector<Op> generate(const Config& c, uint64_t seed, int defaultOps) {
    Generator g(seed);
    vector<Op> ops;
    for (int i = 1; i <= c.sortedPlayers; i++) ops.push_back(g.sortedInsert(c, i));
    int n = c.ops ? c.ops : defaultOps;
    for (int i = 0; i < n; i++) ops.push_back(g.randomOp(c));
    return ops;
}

// ------------------------------------------------------------------
// The three things being compared.  Each apply() returns the line the
// program would print ("" if the operation prints nothing).
// ------------------------------------------------------------------
string oracleApply(Oracle& o, const Op& op) {
    switch (op.type) {
        case 'U': o.update(op.a, op.b); return "";
        case 'D': o.removePlayer(op.a); return "";
        case 'R': { long long r; return o.rank(op.a, r) ? to_string(r) : MSG_NOT_FOUND; }
        case 'S': { PlayerId id; return o.select(op.a, id) ? to_string(id) : MSG_INVALID; }
        default:  return joinIds(o.list(op.a, op.b));      // L clamps; empty result = empty line
    }
}

// A tree plus the id -> score map the application keeps (stands in for the hash table).
template <class Tree>
struct TreeHarness {
    Tree tree;
    unordered_map<PlayerId, Score> scoreOf;

    string apply(const Op& op) {
        switch (op.type) {
            case 'U': {
                auto it = scoreOf.find(op.a);
                if (it != scoreOf.end() && !tree.erase(op.a, it->second))
                    return "BUG: erase() failed for a player that is in the tree";
                tree.insert(op.a, op.b);
                scoreOf[op.a] = op.b;
                return "";
            }
            case 'D': {
                auto it = scoreOf.find(op.a);
                if (it == scoreOf.end()) return "";
                if (!tree.erase(op.a, it->second))
                    return "BUG: erase() failed for a player that is in the tree";
                scoreOf.erase(it);
                return "";
            }
            case 'R': {
                auto it = scoreOf.find(op.a);
                if (it == scoreOf.end()) return MSG_NOT_FOUND;
                int r = tree.rank(op.a, it->second);
                if (r == 0) return "BUG: rank() returned 0 for a player that is in the tree";
                return to_string(r);
            }
            case 'S': {
                PlayerId id;
                return tree.select((int)op.a, id) ? to_string(id) : MSG_INVALID;
            }
            default: {  // 'L'
                // Application policy (same as application.cpp): clamp to 1..size, then ask the tree.
                long long a = max<long long>(op.a, 1);
                long long b = min<long long>(op.b, tree.size());
                if (a > b) return "";                       // nothing left: empty line
                return joinIds(tree.range((int)a, (int)b));
            }
        }
    }
};

// ------------------------------------------------------------------
// Running one configuration
// ------------------------------------------------------------------
string clip(const string& s) { return s.size() > 200 ? s.substr(0, 200) + "..." : s; }

void saveFailingOps(const vector<Op>& ops, size_t upTo) {
    ofstream f("failing_ops.txt");
    for (size_t i = 0; i <= upTo; i++) f << opText(ops[i]) << "\n";
}

bool runConfig(const Config& cfg, uint64_t seed, int defaultOps, int checkEvery) {
    vector<Op> ops = generate(cfg, seed, defaultOps);
    Oracle oracle;
    TreeHarness<AvlAdapter> avl;
    TreeHarness<BstAdapter> bst;

    auto t0 = chrono::steady_clock::now();
    long long queries = 0, invariantRuns = 0;
    const size_t last = ops.size() - 1;

    for (size_t i = 0; i < ops.size(); i++) {
        const Op& op = ops[i];
        string want = oracleApply(oracle, op);
        string gotA = avl.apply(op);
        string gotB = bst.apply(op);
        if (op.type == 'R' || op.type == 'S' || op.type == 'L') queries++;

        bool bad = (want != gotA) || (want != gotB) ||
                   oracle.size() != avl.tree.size() || oracle.size() != bst.tree.size();
        if (bad) {
            cout << "FAIL [" << cfg.name << "] seed " << seed << ", operation #" << (i + 1)
                 << ": " << opText(op) << "\n"
                 << "  oracle : " << clip(want) << "\n"
                 << "  AVL    : " << clip(gotA) << (want != gotA ? "   <-- differs" : "") << "\n"
                 << "  BST    : " << clip(gotB) << (want != gotB ? "   <-- differs" : "") << "\n"
                 << "  sizes  : oracle " << oracle.size() << ", AVL " << avl.tree.size()
                 << ", BST " << bst.tree.size() << "\n";
            saveFailingOps(ops, i);
            cout << "  operations up to the failure saved to failing_ops.txt\n";
            return false;
        }

        if (i < 300 || (i + 1) % checkEvery == 0 || i == last) {
            string why;
            invariantRuns++;
            if (!avl.tree.checkInvariants(why)) {
                cout << "FAIL [" << cfg.name << "] seed " << seed << ", after operation #" << (i + 1)
                     << " (" << opText(op) << "): AVL invariant broken: " << why << "\n";
                saveFailingOps(ops, i);
                return false;
            }
            if (!bst.tree.checkInvariants(why)) {
                cout << "FAIL [" << cfg.name << "] seed " << seed << ", after operation #" << (i + 1)
                     << " (" << opText(op) << "): BST invariant broken: " << why << "\n";
                saveFailingOps(ops, i);
                return false;
            }
        }
    }

    double secs = chrono::duration<double>(chrono::steady_clock::now() - t0).count();
    cout << "PASS [" << cfg.name << "] seed " << seed << ": " << ops.size() << " ops, "
         << queries << " queries compared, " << invariantRuns << " invariant checks, final size "
         << oracle.size() << ", height AVL " << avl.tree.height() << " / BST " << bst.tree.height()
         << ", AVL rotations " << (avl.tree.rotations() ? to_string(avl.tree.rotations()) : string("n/a")) << "  (" << secs << " s)\n";
    return true;
}

int main(int argc, char** argv) {
    uint64_t seed = 12345;
    int defaultOps = 100000;       // spec: at least 10^5 random operations
    int checkEvery = 500;
    string only, emitFile;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        auto next = [&]() -> string {
            if (i + 1 >= argc) { cerr << "missing value after " << arg << "\n"; exit(2); }
            return argv[++i];
        };
        if (arg == "--seed") seed = stoull(next());
        else if (arg == "--ops") defaultOps = stoi(next());
        else if (arg == "--check-every") checkEvery = max(1, stoi(next()));
        else if (arg == "--only") only = next();
        else if (arg == "--emit") emitFile = next();
        else if (arg.rfind("--", 0) == 0) { cerr << "unknown option " << arg << "\n"; return 2; }
        // else: a stray word that is not an option (some Windows shells pass the program's own
        // path as an extra argument when the folder name has spaces) -- ignore it.
    }

    vector<Config> configs = allConfigs();
    bool allOk = true, ran = false;
    for (size_t c = 0; c < configs.size(); c++) {
        if (!only.empty() && configs[c].name != only) continue;
        ran = true;
        uint64_t cfgSeed = seed + c;       // different seed per configuration, all recorded in output
        if (!emitFile.empty()) {
            ofstream f(emitFile);
            for (const Op& op : generate(configs[c], cfgSeed, defaultOps)) f << opText(op) << "\n";
            cout << "wrote " << emitFile << " (configuration " << configs[c].name
                 << ", seed " << cfgSeed << ")\n";
            return 0;
        }
        allOk = runConfig(configs[c], cfgSeed, defaultOps, checkEvery) && allOk;
    }
    if (!ran) { cerr << "no configuration named '" << only << "'\n"; return 2; }
    cout << (allOk ? "ALL PASSED\n" : "SOME TESTS FAILED\n");
    return allOk ? 0 : 1;
}

