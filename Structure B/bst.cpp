#include "bst.hpp"

#include <algorithm>
#include <utility>

namespace leaderboard {

BST::BST() : root_(nullptr) {}

BST::~BST() { clear(); }

void BST::clear() {
    std::vector<Node*> st;
    if (root_) st.push_back(root_);
    while (!st.empty()) {
        Node* n = st.back();
        st.pop_back();
        if (n->left) st.push_back(n->left);
        if (n->right) st.push_back(n->right);
        delete n;
    }
    root_ = nullptr;
}

int BST::size() const { return sz(root_); }

int BST::height() const {
    int h = 0;
    std::vector<const Node*> level, next;
    if (root_) level.push_back(root_);
    while (!level.empty()) {
        ++h;
        next.clear();
        for (const Node* n : level) {
            if (n->left) next.push_back(n->left);
            if (n->right) next.push_back(n->right);
        }
        level.swap(next);
    }
    return h;
}

bool BST::contains(long long id, long long score) const {
    const Node* n = root_;
    while (n) {
        if (n->id == id && n->score == score) return true;
        n = before(score, id, n->score, n->id) ? n->left : n->right;
    }
    return false;
}

bool BST::insert(long long id, long long score) {
    if (contains(id, score)) return false;  // so sizes are only bumped on a real insert
    Node** link = &root_;
    while (*link) {
        Node* n = *link;
        ++n->size;
        link = before(score, id, n->score, n->id) ? &n->left : &n->right;
    }
    *link = new Node{id, score, 1, nullptr, nullptr};
    return true;
}

bool BST::erase(long long id, long long score) {
    if (!contains(id, score)) return false;

    // Walk down to the node, shrinking the size of every ancestor.
    Node** link = &root_;
    while (true) {
        Node* n = *link;
        if (n->id == id && n->score == score) break;
        --n->size;
        link = before(score, id, n->score, n->id) ? &n->left : &n->right;
    }

    Node* n = *link;
    if (!n->left) {
        *link = n->right;
        delete n;
    } else if (!n->right) {
        *link = n->left;
        delete n;
    } else {
        // Two children: replace by the in-order successor (leftmost node of
        // the right subtree) and unlink the successor.
        Node** sl = &n->right;
        while ((*sl)->left) {
            --(*sl)->size;
            sl = &(*sl)->left;
        }
        Node* s = *sl;
        *sl = s->right;
        --n->size;
        n->id = s->id;
        n->score = s->score;
        delete s;
    }
    return true;
}

int BST::position(long long id, long long score) const {
    const Node* n = root_;
    int count = 0;
    while (n) {
        if (n->id == id && n->score == score) return count + sz(n->left) + 1;
        if (before(score, id, n->score, n->id)) {
            n = n->left;
        } else {
            count += sz(n->left) + 1;
            n = n->right;
        }
    }
    return 0;
}

int BST::rank(long long score) const {
    // Count entries with score strictly greater than `score`.
    // If node.score > score, the node and its whole left subtree (scores >= node.score)
    // are counted and we go right; otherwise only the left subtree can hold such entries.
    const Node* n = root_;
    int higher = 0;
    while (n) {
        if (n->score > score) {
            higher += sz(n->left) + 1;
            n = n->right;
        } else {
            n = n->left;
        }
    }
    return higher + 1;
}

long long BST::select(int k) const {
    if (k < 1 || k > sz(root_)) return -1;
    const Node* n = root_;
    while (n) {
        int ls = sz(n->left);
        if (k <= ls) {
            n = n->left;
        } else if (k == ls + 1) {
            return n->id;
        } else {
            k -= ls + 1;
            n = n->right;
        }
    }
    return -1;  // unreachable when sizes are correct
}

std::vector<long long> BST::range(int a, int b) const {
    std::vector<long long> out;
    a = std::max(a, 1);
    b = std::min(b, sz(root_));
    if (a > b) return out;
    out.reserve(static_cast<size_t>(b - a + 1));

    // Descend to position a, stacking the ancestors that come after it.
    std::vector<const Node*> st;
    const Node* cur = root_;
    int k = a;
    while (cur) {
        int ls = sz(cur->left);
        if (k <= ls) {
            st.push_back(cur);
            cur = cur->left;
        } else if (k == ls + 1) {
            st.push_back(cur);
            break;
        } else {
            k -= ls + 1;
            cur = cur->right;
        }
    }

    // Iterative in-order walk for the next b-a+1 entries.
    int need = b - a + 1;
    while (need > 0 && !st.empty()) {
        const Node* n = st.back();
        st.pop_back();
        out.push_back(n->id);
        --need;
        for (const Node* c = n->right; c; c = c->left) st.push_back(c);
    }
    return out;
}

bool BST::validate() const {
    if (!root_) return true;

    // 1. BST order: in-order keys strictly increasing in position order.
    {
        std::vector<const Node*> st;
        const Node* cur = root_;
        const Node* prev = nullptr;
        while (cur || !st.empty()) {
            while (cur) {
                st.push_back(cur);
                cur = cur->left;
            }
            cur = st.back();
            st.pop_back();
            if (prev && !before(prev->score, prev->id, cur->score, cur->id)) return false;
            prev = cur;
            cur = cur->right;
        }
    }

    // 2. Sizes: size == 1 + size(left) + size(right) at every node (post-order;
    //    children are checked before their parent, so induction covers the tree).
    {
        std::vector<std::pair<const Node*, bool>> st;
        st.push_back({root_, false});
        while (!st.empty()) {
            auto [n, done] = st.back();
            st.pop_back();
            if (done) {
                if (n->size != 1 + sz(n->left) + sz(n->right)) return false;
            } else {
                st.push_back({n, true});
                if (n->left) st.push_back({n->left, false});
                if (n->right) st.push_back({n->right, false});
            }
        }
    }
    return true;
}

} 