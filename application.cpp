#include "avl.hpp"
#include "bst.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;
using leaderboard::BST;

// A custom separate-chaining hash table: player ID -> current score.
// This is implemented from scratch rather than using unordered_map.
class PlayerScoreTable {
private:
    struct Entry {
        long long id;
        long long score;
        Entry* next;
        Entry(long long playerId, long long playerScore, Entry* nextEntry = nullptr)
            : id(playerId), score(playerScore), next(nextEntry) {}
    };

    vector<Entry*> buckets;
    size_t count;

    size_t bucketIndex(long long id) const {
        // Convert to unsigned before modulo so negative IDs don't cause a
        // negative bucket index.
        return static_cast<size_t>(static_cast<unsigned long long>(id)) % buckets.size();
    }

public:
    explicit PlayerScoreTable(size_t bucketCount = 100003)
        : buckets(bucketCount, nullptr), count(0) {}

    ~PlayerScoreTable() {
        clear();
    }

    PlayerScoreTable(const PlayerScoreTable&) = delete;
    PlayerScoreTable& operator=(const PlayerScoreTable&) = delete;

    bool get(long long id, long long& scoreOut) const {
        for (Entry* p = buckets[bucketIndex(id)]; p; p = p->next) {
            if (p->id == id) {
                scoreOut = p->score;
                return true;
            }
        }
        return false;
    }

    void set(long long id, long long score) {
        size_t index = bucketIndex(id);
        for (Entry* p = buckets[index]; p; p = p->next) {
            if (p->id == id) {
                p->score = score;
                return;
            }
        }
        buckets[index] = new Entry(id, score, buckets[index]);
        ++count;
    }

    bool erase(long long id) {
        size_t index = bucketIndex(id);
        Entry* current = buckets[index];
        Entry* previous = nullptr;

        while (current) {
            if (current->id == id) {
                if (previous) previous->next = current->next;
                else buckets[index] = current->next;
                delete current;
                --count;
                return true;
            }
            previous = current;
            current = current->next;
        }
        return false;
    }

    size_t size() const {
        return count;
    }

    void clear() {
        for (Entry*& head : buckets) {
            while (head) {
                Entry* next = head->next;
                delete head;
                head = next;
            }
        }
        count = 0;
    }
};

static void printUsage(const char* programName) {
    cerr << "Usage: " << programName << " --structure A|B\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3 || string(argv[1]) != "--structure" ||
        (string(argv[2]) != "A" && string(argv[2]) != "B")) {
        printUsage(argv[0]);
        return 1;
    }

    const bool useAVL = (string(argv[2]) == "A");
    AVL_Tree avl;
    BST bst;
    PlayerScoreTable scores;

    char command;
    while (cin >> command) {
        if (command == 'U') {
            long long id;
            long long score;
            if (!(cin >> id >> score)) return 1;

            long long oldScore;
            if (scores.get(id, oldScore)) {
                if (useAVL) avl.erase(id, oldScore);
                else bst.erase(id, oldScore);
            }

            if (useAVL) avl.insert(id, score);
            else bst.insert(id, score);

            scores.set(id, score);
        }
        else if (command == 'D') {
            long long id;
            if (!(cin >> id)) return 1;

            long long oldScore;
            if (scores.get(id, oldScore)) {
                if (useAVL) avl.erase(id, oldScore);
                else bst.erase(id, oldScore);
                scores.erase(id);
            }
            // Deleting an absent ID produces no output.
        }
        else if (command == 'R') {
            long long id;
            if (!(cin >> id)) return 1;

            long long score;
            if (!scores.get(id, score)) {
                // The project specification does not define absent-ID output.
                // Use NOT_FOUND consistently in both modes.
                cout << "NOT_FOUND\n";
                continue;
            }

            // The hash table has already confirmed the player exists, so both trees
            // answer with a single score-only walk.
            if (useAVL) cout << avl.rank(score) << '\n';
            else cout << bst.rank(score) << '\n';
        }
        else if (command == 'S') {
            int k;
            if (!(cin >> k)) return 1;

            if (useAVL) {
                AVL_Tree::Player player;
                if (avl.select(k, player)) cout << player.id << '\n';
                else cout << "-1\n";
            } else {
                cout << bst.select(k) << '\n';
            }
        }
        else if (command == 'L') {
            int a, b;
            if (!(cin >> a >> b)) return 1;

            // Clamp to the valid positions so both structures behave identically
            // (the AVL range() rejects a < 1, while the BST range() clamps).
            const int total = static_cast<int>(scores.size());
            if (a < 1) a = 1;
            if (b > total) b = total;

            bool first = true;
            if (a > b) {
                // Empty range: print an empty line.
            } else if (useAVL) {
                vector<AVL_Tree::Player> players = avl.range(a, b);
                for (const auto& player : players) {
                    if (!first) cout << ' ';
                    cout << player.id;
                    first = false;
                }
            } else {
                vector<long long> ids = bst.range(a, b);
                for (long long id : ids) {
                    if (!first) cout << ' ';
                    cout << id;
                    first = false;
                }
            }
            cout << '\n';
        }
        else {
            cerr << "Unknown command: " << command << '\n';
            return 1;
        }
    }

    return 0;
}
