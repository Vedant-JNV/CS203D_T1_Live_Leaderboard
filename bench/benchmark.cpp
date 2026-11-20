#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "../Structure A/avl.hpp"
#include "../Structure B/bst.hpp"

using Clock = std::chrono::steady_clock;
using namespace std;

// A visible side effect prevents the compiler from discarding query results.
static volatile long long benchmarkSink = 0;

struct Measurement {
    string structure;
    string arrival;
    int n;
    int run;
    uint64_t seed;
    double insertionTotalMs;
    double insertionNsPerOp;
    double rankNsPerOp;
    double selectNsPerOp;
    int height;
    unsigned long long rotations;
    double rotationsPerInsert;
};

static vector<long long> makeArrivalOrder(int n, const string& arrival, uint64_t seed) {
    vector<long long> scores(n);
    iota(scores.begin(), scores.end(), 1LL);

    if (arrival == "random") {
        mt19937_64 rng(seed);
        shuffle(scores.begin(), scores.end(), rng);
    }
    return scores;
}

static vector<int> makeQueryRanks(int n, int queryCount, uint64_t seed) {
    mt19937_64 rng(seed ^ 0x9E3779B97F4A7C15ULL);
    uniform_int_distribution<int> dist(1, n);
    vector<int> ranks(queryCount);
    for (int& k : ranks) k = dist(rng);
    return ranks;
}

static vector<long long> makeQueryScores(const vector<int>& ranks) {
    vector<long long> scores;
    scores.reserve(ranks.size());
    for (int k : ranks) scores.push_back(k);
    return scores;
}

static Measurement runAVL(int n, const string& arrival, int run,
                          uint64_t seed, int queryCount,
                          const vector<long long>& order,
                          const vector<int>& queryRanks,
                          const vector<long long>& queryScores) {
    AVL_Tree tree;

    const auto insertStart = Clock::now();
    for (long long score : order) {
        const long long id = 1000000000LL + score;
        tree.insert(id, score);
    }
    const auto insertEnd = Clock::now();

    // Query measurements are deliberately outside the insertion timer.
    int rankResult = 0;
    long long localChecksum = 0;
    const auto rankStart = Clock::now();
    for (long long score : queryScores) {
        rankResult = tree.rank(score);
        localChecksum += rankResult;
    }
    const auto rankEnd = Clock::now();

    AVL_Tree::Player selected{};
    const auto selectStart = Clock::now();
    for (int k : queryRanks) {
        if (tree.select(k, selected)) localChecksum += selected.id;
    }
    const auto selectEnd = Clock::now();
    benchmarkSink = benchmarkSink + localChecksum;

    const double insertMs = chrono::duration<double, milli>(insertEnd - insertStart).count();
    const double rankNs = chrono::duration<double, nano>(rankEnd - rankStart).count() / queryCount;
    const double selectNs = chrono::duration<double, nano>(selectEnd - selectStart).count() / queryCount;
    const auto rotations = tree.getRotationCount();

    return {"AVL", arrival, n, run, seed, insertMs,
            insertMs * 1.0e6 / n, rankNs, selectNs,
            tree.height(), rotations, static_cast<double>(rotations) / n};
}

static Measurement runBST(int n, const string& arrival, int run,
                          uint64_t seed, int queryCount,
                          const vector<long long>& order,
                          const vector<int>& queryRanks,
                          const vector<long long>& queryScores) {
    leaderboard::BST tree;

    const auto insertStart = Clock::now();
    for (long long score : order) {
        const long long id = 1000000000LL + score;
        if (!tree.insert(id, score)) {
            cerr << "Unexpected duplicate during BST insertion\n";
            exit(2);
        }
    }
    const auto insertEnd = Clock::now();

    int rankResult = 0;
    long long localChecksum = 0;
    const auto rankStart = Clock::now();
    for (long long score : queryScores) {
        rankResult = tree.rank(score);
        localChecksum += rankResult;
    }
    const auto rankEnd = Clock::now();

    const auto selectStart = Clock::now();
    for (int k : queryRanks) localChecksum += tree.select(k);
    const auto selectEnd = Clock::now();
    benchmarkSink = benchmarkSink + localChecksum;

    const double insertMs = chrono::duration<double, milli>(insertEnd - insertStart).count();
    const double rankNs = chrono::duration<double, nano>(rankEnd - rankStart).count() / queryCount;
    const double selectNs = chrono::duration<double, nano>(selectEnd - selectStart).count() / queryCount;

    return {"BST", arrival, n, run, seed, insertMs,
            insertMs * 1.0e6 / n, rankNs, selectNs,
            tree.height(), 0ULL, 0.0};
}

int main(int argc, char** argv) {
    const bool quickMode = (argc >= 2 && string(argv[1]) == "--quick");
    const vector<int> defaultSizes = quickMode
        ? vector<int>{100, 1000}
        : vector<int>{1000, 5000, 10000, 25000, 50000, 100000};
    const vector<uint64_t> seeds = {20261114ULL, 20261115ULL, 20261116ULL,
                                    20261117ULL, 20261118ULL};
    const int runs = static_cast<int>(seeds.size());
    const int queryCount = 200;
    const string outputPath = quickMode ? "results/benchmark_quick.csv"
        : ((argc >= 2) ? argv[1] : "results/benchmark_raw.csv");

    ofstream out(outputPath);
    if (!out) {
        cerr << "Could not open output file: " << outputPath << '\n';
        return 1;
    }

    out << "structure,arrival,n,run,seed,insertion_total_ms,insertion_ns_per_op,"
           "rank_ns_per_op,select_ns_per_op,height,rotations,rotations_per_insert\n";
    out << fixed << setprecision(6);

    for (int n : defaultSizes) {
        for (const string arrival : {"random", "sorted"}) {
            for (int run = 1; run <= runs; ++run) {
                const uint64_t seed = seeds[run - 1];
                const auto order = makeArrivalOrder(n, arrival, seed);
                const auto queryRanks = makeQueryRanks(n, queryCount, seed);
                const auto queryScores = makeQueryScores(queryRanks);

                cerr << "n=" << n << " arrival=" << arrival << " run=" << run
                     << " seed=" << seed << " ... " << flush;
                const Measurement avl = runAVL(n, arrival, run, seed, queryCount,
                                               order, queryRanks, queryScores);
                const Measurement bst = runBST(n, arrival, run, seed, queryCount,
                                               order, queryRanks, queryScores);

                auto write = [&](const Measurement& m) {
                    out << m.structure << ',' << m.arrival << ',' << m.n << ','
                        << m.run << ',' << m.seed << ',' << m.insertionTotalMs << ','
                        << m.insertionNsPerOp << ',' << m.rankNsPerOp << ','
                        << m.selectNsPerOp << ',' << m.height << ',' << m.rotations
                        << ',' << m.rotationsPerInsert << '\n';
                };
                write(avl);
                write(bst);
                out.flush();
                cerr << "AVL " << avl.insertionNsPerOp << " ns/insert, h="
                     << avl.height << "; BST " << bst.insertionNsPerOp
                     << " ns/insert, h=" << bst.height << '\n';
            }
        }
    }

    cerr << "Benchmark complete. Results: " << outputPath
         << " (checksum=" << benchmarkSink << ")\n";
    return 0;
}
