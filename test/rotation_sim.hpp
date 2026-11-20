#pragma once

#include <algorithm>

class RotationSim {
    struct Node {
        int key;
        int height;
        Node* left;
        Node* right;
        explicit Node(int k) : key(k), height(1), left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;
    long long count = 0;

    static int heightOf(const Node* n) { return n ? n->height : 0; }
    static void refresh(Node* n) { n->height = 1 + std::max(heightOf(n->left), heightOf(n->right)); }

    Node* rotateLeft(Node* x) {          // x's right child moves up
        count++;
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        refresh(x);
        refresh(y);
        return y;
    }
    Node* rotateRight(Node* x) {         // x's left child moves up
        count++;
        Node* y = x->left;
        x->left = y->right;
        y->right = x;
        refresh(x);
        refresh(y);
        return y;
    }

    Node* insertNode(Node* n, int key) {
        if (!n) return new Node(key);
        if (key < n->key) n->left = insertNode(n->left, key);
        else              n->right = insertNode(n->right, key);
        refresh(n);

        int balance = heightOf(n->left) - heightOf(n->right);
        if (balance > 1) {                                            // left too tall
            if (heightOf(n->left->left) < heightOf(n->left->right))   // left-right case
                n->left = rotateLeft(n->left);
            return rotateRight(n);
        }
        if (balance < -1) {                                           // right too tall
            if (heightOf(n->right->right) < heightOf(n->right->left)) // right-left case
                n->right = rotateRight(n->right);
            return rotateLeft(n);
        }
        return n;
    }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

public:
    RotationSim() = default;
    ~RotationSim() { destroy(root); }
    RotationSim(const RotationSim&) = delete;
    RotationSim& operator=(const RotationSim&) = delete;

    void insert(int key) { root = insertNode(root, key); }   // keys must be distinct
    long long rotations() const { return count; }
    int height() const { return heightOf(root); }
};
