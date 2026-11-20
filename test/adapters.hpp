#pragma once

#include <string>
#include <vector>

#include "types.hpp"
#include "avl.hpp"
#include "bst.hpp"
#include "invariant.hpp"

class AvlAdapter {
public:
    AVL_Tree impl;

    void insert(PlayerId id, Score score) { impl.insert(id, score); }
    bool erase(PlayerId id, Score score)  { return impl.erase(id, score); }

    int rank(PlayerId id, Score score) const {
        if (!impl.contains(id, score)) return 0;                // AVL::rank only takes the score
        return impl.rank(score);
    }
    bool select(int k, PlayerId& out) const {
        AVL_Tree::Player p;
        if (!impl.select(k, p)) return false;
        out = p.id;
        return true;
    }
    std::vector<PlayerId> range(int a, int b) const {
        std::vector<PlayerId> ids;
        if (a < 1 || b < a || b > impl.size()) return ids;     // strict, see note above
        for (const AVL_Tree::Player& p : impl.range(a, b)) ids.push_back(p.id);
        return ids;
    }
    int size() const   { return impl.size(); }
    int height() const { return impl.height(); }
    long long rotations() const { return static_cast<long long>(impl.getRotationCount()); }

    bool checkInvariants(std::string& why) const { return ::InvariantChecker::check(impl, why); }
};

class BstAdapter {
public:
    leaderboard::BST impl;

    void insert(PlayerId id, Score score) { impl.insert(id, score); }
    bool erase(PlayerId id, Score score)  { return impl.erase(id, score); }

    int rank(PlayerId id, Score score) const {
        if (!impl.contains(id, score)) return 0;                // BST::rank only takes the score
        return impl.rank(score);
    }
    bool select(int k, PlayerId& out) const {
        if (k < 1 || k > impl.size()) return false;             // BST::select returns -1 otherwise
        out = impl.select(k);
        return true;
    }
    std::vector<PlayerId> range(int a, int b) const {
        if (a < 1 || b < a || b > impl.size()) return {};       // BST::range would clamp; stay strict
        return impl.range(a, b);
    }
    int size() const   { return impl.size(); }
    int height() const { return impl.height(); }
    long long rotations() const { return 0; }                   // no rotations in a plain BST

    bool checkInvariants(std::string& why) const { return leaderboard::InvariantChecker::check(impl, why); }
};
