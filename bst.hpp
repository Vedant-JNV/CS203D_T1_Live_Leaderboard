
#pragma once

#include <vector>

namespace leaderboard {

class BST {
public:
    BST();
    ~BST();
    BST(const BST&) = delete;
    BST& operator=(const BST&) = delete;

    // Add entry (id, score). Returns false if that exact entry already exists.
    bool insert(int id, long long score);

    // Remove entry (id, score). Returns false if it is not present.
    // (The application looks up the player's current score in the hash table.)
    bool erase(int id, long long score);

    // True if entry (id, score) is in the tree.
    bool contains(int id, long long score) const;

    // 1-based position of entry (id, score) in the order, or 0 if absent.
    int position(int id, long long score) const;

    // Competition rank (1, 2, 2, 4): 1 + number of entries with a strictly
    // higher score. Depends only on the score.
    int rank(long long score) const;

    // Player id at position k (1 = top), or -1 if k is out of range.
    int select(int k) const;

    // Ids at positions a..b (clamped to [1, size]), in order. Empty if a > b.
    std::vector<int> range(int a, int b) const;

    int  size() const;     // number of entries
    int  height() const;   // number of nodes on the longest root-to-leaf path (0 if empty)
    void clear();

    // Invariant checker: BST order and correct subtree size at every node.
    bool validate() const;

private:
    struct Node {
        int       id;
        long long score;
        int       size;
        Node*     left;
        Node*     right;
    };

    Node* root_;

    static int sz(const Node* n) { return n ? n->size : 0; }
    // true if (s1,id1) comes strictly before (s2,id2) in position order
    static bool before(long long s1, int id1, long long s2, int id2) {
        return s1 > s2 || (s1 == s2 && id1 < id2);
    }
};

}  // namespace leaderboard