
#include "avl.hpp"
#include <cassert>
#include <iostream>

int main() {
    AVL_Tree tree;
    AVL_Tree::Player p;
    int r;

    // Test empty tree
    assert(tree.size() == 0);
    assert(tree.height() == 0);
    assert(!tree.select(1, p));

    // Insert players in an order that exercises AVL balancing
    tree.insert(4, 80);
    tree.insert(3, 90);
    tree.insert(2, 90);
    tree.insert(1, 100);

    assert(tree.size() == 4);
    assert(tree.height() <= 3);
    assert(tree.height() >= 1);

    // Test contains()
    assert(tree.contains(1, 100));
    assert(tree.contains(2, 90));
    assert(!tree.contains(99, 50));

    // Test select(): leaderboard positions are 1-based
    assert(tree.select(1, p));
    assert(p.id == 1 && p.score == 100);

    assert(tree.select(2, p));
    assert(p.id == 2 && p.score == 90);

    assert(tree.select(3, p));
    assert(p.id == 3 && p.score == 90);

    assert(tree.select(4, p));
    assert(p.id == 4 && p.score == 80);

    assert(!tree.select(0, p));
    assert(!tree.select(5, p));

    // Test competition ranking
    assert(tree.rank(1, 100, r) && r == 1);
    assert(tree.rank(2, 90, r) && r == 2);
    assert(tree.rank(3, 90, r) && r == 2);
    assert(tree.rank(4, 80, r) && r == 4);
    assert(!tree.rank(99, 50, r));

    // Test range(): positions 2 through 3, inclusive
    std::vector<AVL_Tree::Player> players = tree.range(2, 3);

    assert(players.size() == 2);
    assert(players[0].id == 2 && players[0].score == 90);
    assert(players[1].id == 3 && players[1].score == 90);

    // Test invalid ranges
    assert(tree.range(0, 2).empty());
    assert(tree.range(3, 2).empty());
    assert(tree.range(5, 6).empty());

    // Test range extending beyond the end
    players = tree.range(3, 10);
    assert(players.size() == 2);
    assert(players[0].id == 3);
    assert(players[1].id == 4);

    // Test erase() for a missing player
    assert(!tree.erase(99, 50));
    assert(tree.size() == 4);

    // Delete the root player
    assert(tree.erase(1, 100));
    assert(tree.size() == 3);
    assert(!tree.contains(1, 100));

    assert(tree.select(1, p));
    assert(p.id == 2 && p.score == 90);

    // Delete one of the tied-score players
    assert(tree.erase(3, 90));
    assert(tree.size() == 2);

    assert(tree.rank(2, 90, r) && r == 1);
    assert(tree.rank(4, 80, r) && r == 2);

    // Delete all remaining players
    assert(tree.erase(2, 90));
    assert(tree.erase(4, 80));

    assert(tree.size() == 0);
    assert(tree.height() == 0);
    assert(!tree.select(1, p));
    assert(tree.range(1, 5).empty());

    std::cout << "All basic AVL tests passed!" << std::endl;

    return 0;
}
