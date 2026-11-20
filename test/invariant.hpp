#pragma once

#include <algorithm>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "types.hpp"
#include "avl.hpp"
#include "bst.hpp"


namespace invariant_core {

inline bool before(Score s1, PlayerId i1, Score s2, PlayerId i2) {
    return s1 > s2 || (s1 == s2 && i1 < i2);
}

template <class V>
std::string describe(const typename V::Node* n) {
    return "(id=" + std::to_string(V::id(n)) + ", score=" + std::to_string(V::score(n)) + ")";
}

template <class V>
bool run(const typename V::Node* root, int reportedSize, int reportedHeight, std::string& why) {
    using Node = typename V::Node;
    auto sizeOf   = [](const Node* n) { return n ? V::size(n) : 0; };
    auto heightOf = [](const Node* n) { return n ? V::height(n) : 0; };

    if (!root) {
        if (reportedSize != 0)   { why = "empty tree but size() = " + std::to_string(reportedSize); return false; }
        if (reportedHeight != 0) { why = "empty tree but height() = " + std::to_string(reportedHeight); return false; }
        return true;
    }

    // ---- Pass 1: in-order walk; checks 1, 2, 3, 4, 6, 7 ----
    std::vector<const Node*> stack;
    const Node* cur = root;
    const Node* prev = nullptr;
    long long count = 0;
    std::unordered_set<PlayerId> seen;
    const std::string tooMany = "walk found more nodes than size() reports "
                                "(wrong size, or a pointer loop)";

    while (cur || !stack.empty()) {
        while (cur) {
            stack.push_back(cur);
            if ((long long)stack.size() > reportedSize) { why = tooMany; return false; }
            cur = V::left(cur);
        }
        cur = stack.back();
        stack.pop_back();
        ++count;
        if (count > reportedSize) { why = tooMany; return false; }

        // 1. order
        if (prev && !before(V::score(prev), V::id(prev), V::score(cur), V::id(cur))) {
            why = "order violated: " + describe<V>(prev) + " is followed by " + describe<V>(cur);
            return false;
        }
        // 2. unique ids
        if (!seen.insert(V::id(cur)).second) {
            why = "player id " + std::to_string(V::id(cur)) + " appears twice";
            return false;
        }
        // 3. size
        int expectSize = 1 + sizeOf(V::left(cur)) + sizeOf(V::right(cur));
        if (V::size(cur) != expectSize) {
            why = "node " + describe<V>(cur) + " stores size " + std::to_string(V::size(cur)) +
                  " but children give " + std::to_string(expectSize);
            return false;
        }
        if constexpr (V::hasHeight) {
            int hl = heightOf(V::left(cur)), hr = heightOf(V::right(cur));
            // 6. stored height
            if (V::height(cur) != 1 + std::max(hl, hr)) {
                why = "node " + describe<V>(cur) + " stores height " + std::to_string(V::height(cur)) +
                      " but children give " + std::to_string(1 + std::max(hl, hr));
                return false;
            }
            // 7. balance factor
            int bf = hl - hr;
            if (bf < -1 || bf > 1) {
                why = "node " + describe<V>(cur) + " has balance factor " + std::to_string(bf);
                return false;
            }
        }
        prev = cur;
        cur = V::right(cur);
    }

    // 4. count
    if (count != reportedSize) {
        why = "walk found " + std::to_string(count) + " nodes but size() = " + std::to_string(reportedSize);
        return false;
    }

    // ---- Pass 2: real height by depth-first walk; check 5 ----
    // Pass 1 passed, so the structure is a proper tree (no loops) and this terminates.
    int realHeight = 0;
    std::vector<std::pair<const Node*, int>> dfs;
    dfs.push_back({root, 1});
    while (!dfs.empty()) {
        auto [n, depth] = dfs.back();
        dfs.pop_back();
        realHeight = std::max(realHeight, depth);
        if (V::left(n))  dfs.push_back({V::left(n),  depth + 1});
        if (V::right(n)) dfs.push_back({V::right(n), depth + 1});
    }
    if (realHeight != reportedHeight) {
        why = "height() reports " + std::to_string(reportedHeight) + " but real height is " +
              std::to_string(realHeight);
        return false;
    }
    return true;
}

}  // namespace invariant_core

// ---------------------------------------------------------------------------
// Checker for the AVL tree: the global friend named in avl.hpp.
// ---------------------------------------------------------------------------
struct InvariantChecker {
    struct View {
        using Node = AVL_Tree::Node;
        static constexpr bool hasHeight = true;
        static const Node* left(const Node* n)  { return n->left; }
        static const Node* right(const Node* n) { return n->right; }
        static PlayerId id(const Node* n)       { return n->player.id; }
        static Score score(const Node* n)       { return n->player.score; }
        static int size(const Node* n)          { return n->subtreeSize; }
        static int height(const Node* n)        { return n->height; }
    };
    static bool check(const AVL_Tree& t, std::string& why) {
        return invariant_core::run<View>(t.root, t.size(), t.height(), why);
    }
};


namespace leaderboard {
struct InvariantChecker {
    struct View {
        using Node = BST::Node;
        static constexpr bool hasHeight = false;
        static const Node* left(const Node* n)  { return n->left; }
        static const Node* right(const Node* n) { return n->right; }
        static PlayerId id(const Node* n)       { return n->id; }
        static Score score(const Node* n)       { return n->score; }
        static int size(const Node* n)          { return n->size; }
        static int height(const Node*)          { return 0; }   // not stored
    };
    static bool check(const BST& t, std::string& why) {
        return invariant_core::run<View>(t.root_, t.size(), t.height(), why);
    }
};
}  // namespace leaderboard
