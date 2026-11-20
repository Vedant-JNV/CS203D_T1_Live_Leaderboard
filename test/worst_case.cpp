#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "rotation_sim.hpp"

using namespace std;

// ---------------------------------------------------------------------------
// Insertion orders
// ---------------------------------------------------------------------------
vector<int> makeSorted(int n) {
    vector<int> keys(n);
    iota(keys.begin(), keys.end(), 1);
    return keys;
}

vector<int> makeReverse(int n) {
    vector<int> keys = makeSorted(n);
    reverse(keys.begin(), keys.end());
    return keys;
}

vector<int> makeRandom(int n, uint64_t seed) {
    vector<int> keys = makeSorted(n);
    mt19937_64 rng(seed);
    shuffle(keys.begin(), keys.end(), rng);
    return keys;
}

// Level by level: the middle key, then the middle of each half, and so on.
// The tree this builds is already balanced, so no rotation is ever needed.
vector<int> makeBalanced(int n) {
    vector<int> keys;
    deque<pair<int, int>> ranges;            // key ranges still to place (lo..hi)
    if (n > 0) ranges.push_back({1, n});
    while (!ranges.empty()) {
        auto [lo, hi] = ranges.front();
        ranges.pop_front();
        if (lo > hi) continue;
        int mid = (lo + hi) / 2;
        keys.push_back(mid);
        ranges.push_back({lo, mid - 1});
        ranges.push_back({mid + 1, hi});
    }
    return keys;
}

// 1, n, 2, n-1, 3, n-2, ...
vector<int> makeZigzag(int n) {
    vector<int> keys;
    for (int low = 1, high = n; low <= high; low++, high--) {
        keys.push_back(low);
        if (low != high) keys.push_back(high);
    }
    return keys;
}

// Repeats a pattern over blocks of `blockSize` keys.  pattern[j] says which key of the
// block (1..blockSize) comes next.  The last block may be shorter: we keep only the
// pattern entries that fit, in the same order, so every key is still used exactly once.
vector<int> makeBlocks(int n, const vector<int>& pattern) {
    vector<int> keys;
    int blockSize = (int)pattern.size();
    for (int start = 0; start < n; start += blockSize) {
        int inThisBlock = min(blockSize, n - start);
        for (int p : pattern)
            if (p <= inThisBlock) keys.push_back(start + p);
    }
    return keys;
}

vector<int> makeTriples(int n) { return makeBlocks(n, {3, 2, 1}); }
vector<int> makeBlock7(int n)  { return makeBlocks(n, {4, 2, 1, 7, 5, 3, 6}); }

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
const vector<string> ORDER_NAMES = {"balanced", "random", "sorted", "reverse", "zigzag", "triples", "block7"};

vector<int> makeOrder(const string& name, int n, uint64_t seed) {
    if (name == "sorted")   return makeSorted(n);
    if (name == "reverse")  return makeReverse(n);
    if (name == "random")   return makeRandom(n, seed);
    if (name == "balanced") return makeBalanced(n);
    if (name == "zigzag")   return makeZigzag(n);
    if (name == "triples")  return makeTriples(n);
    if (name == "block7")   return makeBlock7(n);
    cerr << "unknown order '" << name << "'\n";
    exit(2);
}

long long countRotations(const vector<int>& keys, int* heightOut = nullptr) {
    RotationSim sim;
    for (int k : keys) sim.insert(k);
    if (heightOut) *heightOut = sim.height();
    return sim.rotations();
}

// Sanity check: the order must use every key 1..n exactly once.
bool isPermutation(vector<int> keys, int n) {
    if ((int)keys.size() != n) return false;
    sort(keys.begin(), keys.end());
    for (int i = 0; i < n; i++)
        if (keys[i] != i + 1) return false;
    return true;
}

int main(int argc, char** argv) {
    int n = 100000;
    uint64_t seed = 1;
    string order, outFile;
    bool table = false;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        auto next = [&]() -> string {
            if (i + 1 >= argc) { cerr << "missing value after " << arg << "\n"; exit(2); }
            return argv[++i];
        };
        if (arg == "--n") n = stoi(next());
        else if (arg == "--seed") seed = stoull(next());
        else if (arg == "--order") order = next();
        else if (arg == "--out") outFile = next();
        else if (arg == "--table") table = true;
        else if (arg.rfind("--", 0) == 0) { cerr << "unknown option " << arg << "\n"; return 2; }
        // else: stray word (e.g. the program path passed by some Windows shells) -- ignore
    }

    if (table) {
        cout << "n = " << n << ", single rotation = 1, double rotation = 2\n";
        printf("%-10s %12s %12s %8s\n", "order", "rotations", "per insert", "height");
        for (const string& name : ORDER_NAMES) {
            if (name == "random") {          // average of 5 shuffles: seeds seed .. seed+4
                long long total = 0;
                int h = 0;
                for (int s = 0; s < 5; s++) total += countRotations(makeRandom(n, seed + s), &h);
                printf("%-10s %12lld %12.3f %8d   (average of 5 shuffles, seeds %llu-%llu)\n", "random",
                       total / 5, (double)(total / 5) / n, h,
                       (unsigned long long)seed, (unsigned long long)(seed + 4));
                continue;
            }
            vector<int> keys = makeOrder(name, n, seed);
            if (!isPermutation(keys, n)) { cerr << "BUG: order " << name << " is not a permutation\n"; return 1; }
            int h = 0;
            long long r = countRotations(keys, &h);
            printf("%-10s %12lld %12.3f %8d\n", name.c_str(), r, (double)r / n, h);
        }
        cout << "Upper bound: one insertion causes at most one rebalance, which costs at most 2 "
                "rotations, so no order can exceed 2 per insert.\n";
        return 0;
    }

    if (order.empty() || outFile.empty()) {
        cerr << "usage: worstcase --table [--n N]\n"
                "       worstcase --order NAME --n N [--seed S] --out FILE\n";
        return 2;
    }
    vector<int> keys = makeOrder(order, n, seed);
    if (!isPermutation(keys, n)) { cerr << "BUG: order is not a permutation\n"; return 1; }
    ofstream f(outFile);
    for (int k : keys) f << "U " << k << " " << k << "\n";
    cout << "wrote " << outFile << ": order " << order << ", n " << n
         << (order == "random" ? ", seed " + to_string(seed) : string())
         << ", " << countRotations(keys) << " rotations\n";
    return 0;
}
