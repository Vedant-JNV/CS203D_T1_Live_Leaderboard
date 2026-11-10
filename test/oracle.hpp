// oracle.hpp -- Project 1 oracle: a sorted vector, linear-time updates.
// No trees, no hash tables. Deliberately simple so it is obviously correct.
//
// Vector order: highest score first; equal scores -> smaller id first.
#pragma once

#include <string>
#include <vector>
#include "types.hpp"

// Output texts for situations the spec does not define (agree with the team).
inline const std::string MSG_NOT_FOUND = "NOT_FOUND";  // R for an id that does not exist
inline const std::string MSG_INVALID   = "-1";         // S with a position out of range (same as application.cpp / README)

class Oracle {
    struct Entry { PlayerId id; Score score; };
    std::vector<Entry> v;   // always kept sorted

    static bool comesBefore(const Entry& a, const Entry& b) {
        if (a.score != b.score) return a.score > b.score;   // higher score first
        return a.id < b.id;                                  // tie: smaller id first
    }

    // Index of the entry with this id, or -1. Plain scan. O(n)
    int findIndex(PlayerId id) const {
        for (int i = 0; i < (int)v.size(); i++)
            if (v[i].id == id) return i;
        return -1;
    }

public:
    // U id score: remove old entry (if any), then insert at the right place. O(n)
    void update(PlayerId id, Score score) {
        removePlayer(id);
        Entry p{id, score};
        int pos = 0;
        while (pos < (int)v.size() && comesBefore(v[pos], p)) pos++;
        v.insert(v.begin() + pos, p);
    }

    // D id: returns false if the id was not present. O(n)
    bool removePlayer(PlayerId id) {
        int i = findIndex(id);
        if (i == -1) return false;
        v.erase(v.begin() + i);
        return true;
    }

    // R id: competition rank = 1 + number of players with a strictly higher score. O(n)
    bool rank(PlayerId id, long long& out) const {
        int i = findIndex(id);
        if (i == -1) return false;
        long long higher = 0;
        for (const Entry& p : v)
            if (p.score > v[i].score) higher++;
        out = higher + 1;
        return true;
    }

    // S k: id at POSITION k (1 = top). Position is the plain index, not the rank. O(1)
    bool select(long long k, PlayerId& out) const {
        if (k < 1 || k > (long long)v.size()) return false;
        out = v[k - 1].id;
        return true;
    }

    // L a b: ids at positions a..b, CLAMPED to the board: a is raised to 1 and b is
    // lowered to size. If nothing is left (a > b after clamping, or an empty board)
    // the result is empty. Never an error. O(b-a+1)
    std::vector<PlayerId> list(long long a, long long b) const {
        std::vector<PlayerId> out;
        if (a < 1) a = 1;
        if (b > (long long)v.size()) b = (long long)v.size();
        for (long long pos = a; pos <= b; pos++) out.push_back(v[pos - 1].id);
        return out;
    }

    int size() const { return (int)v.size(); }
};
