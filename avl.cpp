// include the header file for the AVL tree implementation
#include "avl.hpp"
#include <algorithm>

// Return height of a node; empty tree has height 0
int AVL_Tree::getHeight(Node* node) const {
    return node ? node->height : 0;
}

// Return subtree size; empty tree has size 0
int AVL_Tree::getSize(Node* node) const {
    return node ? node->subtreeSize : 0;
}

// Update height and subtree size
void AVL_Tree::update(Node* node) {
    // If the node is null, there's nothing to update
    if (!node) return;

    // Update the height of the node based on the heights of its children
    node->height = 1 + max(getHeight(node->left),getHeight(node->right));

    // Update the subtree size of the node based on the sizes of its children
    node->subtreeSize = 1 + getSize(node->left) + getSize(node->right);
}

// Balance factor: left height - right height
int AVL_Tree::getBalance(Node* node) const {
    if (!node) return 0;
    return getHeight(node->left) - getHeight(node->right);
}

// Ordering: higher score first; ties use smaller ID first
bool AVL_Tree::comesBefore(const Player& a, const Player& b) const {
    if (a.score != b.score)
        return a.score > b.score;
    return a.id < b.id;
}

// Left rotation
AVL_Tree::Node* AVL_Tree::rotateLeft(Node* node) {
    // Store pointers
    Node* newRoot = node->right;
    Node* middle = newRoot->left;
    // Shift pointers
    newRoot->left = node;
    node->right = middle;
    // Update heights and sizes
    update(node);
    update(newRoot);

    return newRoot;
}

// Right rotation
AVL_Tree::Node* AVL_Tree::rotateRight(Node* node) {
    // Store pointers
    Node* newRoot = node->left;
    Node* middle = newRoot->right;
    // Shift pointers
    newRoot->right = node;
    node->left = middle;
    // Update heights and sizes
    update(node);
    update(newRoot);

    return newRoot;
}
