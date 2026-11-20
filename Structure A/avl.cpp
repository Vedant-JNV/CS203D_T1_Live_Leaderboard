// include the header file for the AVL tree implementation
#include "avl.hpp"
#include <algorithm>

// Return height of a node; empty tree has height 0
int AVL_Tree::getHeight(Node *node) const
{
    return node ? node->height : 0;
}

// Return subtree size; empty tree has size 0
int AVL_Tree::getSize(Node *node) const
{
    return node ? node->subtreeSize : 0;
}

// Update height and subtree size
void AVL_Tree::update(Node *node)
{
    // If the node is null, there's nothing to update
    if (!node)
        return;

    // Update the height of the node based on the heights of its children
    node->height = 1 + max(getHeight(node->left), getHeight(node->right));

    // Update the subtree size of the node based on the sizes of its children
    node->subtreeSize = 1 + getSize(node->left) + getSize(node->right);
}

// Balance factor: left height - right height
int AVL_Tree::getBalance(Node *node) const
{
    if (!node)
        return 0;
    return getHeight(node->left) - getHeight(node->right);
}

// Ordering: higher score first; ties use smaller ID first
bool AVL_Tree::comesBefore(const Player &a, const Player &b) const
{
    if (a.score != b.score)
        return a.score > b.score;
    return a.id < b.id;
}

// Left rotation
AVL_Tree::Node *AVL_Tree::rotateLeft(Node *node)
{
    ++rotationCount;
    // Store pointers
    Node *newRoot = node->right;
    Node *middle = newRoot->left;
    // Shift pointers
    newRoot->left = node;
    node->right = middle;
    // Update heights and sizes
    update(node);
    update(newRoot);

    return newRoot;
}

// Right rotation
AVL_Tree::Node *AVL_Tree::rotateRight(Node *node)
{
    ++rotationCount;
    // Store pointers
    Node *newRoot = node->left;
    Node *middle = newRoot->right;
    // Shift pointers
    newRoot->right = node;
    node->left = middle;
    // Update heights and sizes
    update(node);
    update(newRoot);

    return newRoot;
}

// Function to rebalance the AVL Tree
AVL_Tree::Node *AVL_Tree::rebalance(Node *node)
{

    // If the node is null, there's nothing to rebalance
    if (!node)
        return nullptr;

    // Update the height and subtree size of the node
    update(node);

    // Get the balance factor of the node
    int balance = getBalance(node);

    // Left-heavy
    if (balance > 1)
    {
        // If the left child is right-heavy, perform a left rotation on the left child
        if (getBalance(node->left) < 0)
            node->left = rotateLeft(node->left);
        // Perform a right rotation on the node
        return rotateRight(node);
    }

    // Right-heavy
    if (balance < -1)
    {
        // If the right child is left-heavy, perform a right rotation on the right child
        if (getBalance(node->right) > 0)
            node->right = rotateRight(node->right);
        // Perform a left rotation on the node
        return rotateLeft(node);
    }

    return node;
}

// Recursive insertion
AVL_Tree::Node *AVL_Tree::insertNode(Node *node, Player player)
{
    if (!node)
        return new Node(player);

    if (comesBefore(player, node->player))
        node->left = insertNode(node->left, player);
    else
        node->right = insertNode(node->right, player);

    return rebalance(node);
}

// Public insertion function
void AVL_Tree::insert(long long id, long long score)
{
    root = insertNode(root, {id, score});
}

// Find a node with the given player in the AVL tree
AVL_Tree::Node *AVL_Tree::findNode(Player player) const
{

    // Start from the root of the tree
    Node *current = root;

    // Traverse the tree until we find the node or reach a null pointer
    while (current)
    {

        // check if the current node's player matches the target player
        if (current->player.id == player.id && current->player.score == player.score)
            return current;

        // Move left or right based on the ordering of players
        if (comesBefore(player, current->player))
            current = current->left;
        else
            current = current->right;
    }
    // If we reach a null pointer, the player is not in the tree
    return nullptr;
}

// Public function to check if a player with the given id and score exists in the tree
bool AVL_Tree::contains(long long id, long long score) const
{
    return findNode({id, score}) != nullptr;
}

// Function to find left most node (minimum) in a subtree
AVL_Tree::Node *AVL_Tree::findMin(Node *node) const
{
    while (node && node->left)
        node = node->left;
    return node;
}

// Recursive deletion
AVL_Tree::Node *AVL_Tree::eraseNode(Node *node, Player player)
{
    // If the node is null, there's nothing to delete
    if (!node)
        return nullptr;

    // Traverse the tree to find the node to delete
    if (comesBefore(player, node->player))
    {
        node->left = eraseNode(node->left, player);
    }
    else if (comesBefore(node->player, player))
    {
        node->right = eraseNode(node->right, player);
    }
    // If the node to delete is found
    else
    {
        // If the node has no children or only one child, delete it and return the child
        if (!node->left || !node->right)
        {
            Node *child = node->left ? node->left : node->right;
            delete node;
            return child;
        }

        // If the node has two children, find the in-order successor (smallest in the right subtree)
        Node *successor = findMin(node->right);
        // Replace the node's player with the successor's player and delete the successor
        node->player = successor->player;
        node->right = eraseNode(node->right, successor->player);
    }
    // Rebalance the tree at every step of the recursion and return the updated node
    return rebalance(node);
}

// Public function to erase a player with the given id and score from the tree
bool AVL_Tree::erase(long long id, long long score)
{
    if (!contains(id, score))
        return false;

    root = eraseNode(root, {id, score});
    return true;
}

// Function to select the k-th player in the sorted order and store it in result
bool AVL_Tree::select(int k, Player &result) const
{

    // Check if k is within the valid range
    if (k < 1 || k > getSize(root))
        return false;

    // Start from the root of the tree
    Node *current = root;

    while (current)
    {
        int leftSize = getSize(current->left);

        // If the k-th player is found, store it in result and return true
        if (k == leftSize + 1)
        {
            result = current->player;
            return true;
        }
        // Move left or right based on the size of the left subtree
        else if (k <= leftSize)
        {
            // If k is less than or equal to the size of the left subtree, move to the left subtree
            current = current->left;
        }
        else
        {
            // If k is greater than the size of the left subtree, adjust k and move to the right subtree
            k -= leftSize + 1;
            current = current->right;
        }
    }

    // If we reach a null pointer, the k-th player is not found
    return false;
}

// Competition rank for a score: 1 + number of players with a strictly higher score.
// One root-to-leaf walk; the player's id is not needed because equal scores share a rank.
int AVL_Tree::rank(long long score) const
{
    Node *current = root;
    int count = 0;

    while (current)
    {
        // The current node and its whole left subtree all have scores >= current's score,
        // so if current's score is strictly greater, they all count; continue to the right.
        if (current->player.score > score)
        {
            count += 1 + getSize(current->left);
            current = current->right;
        }
        else
        {
            // Current score <= target score: nothing in the right subtree can be higher.
            current = current->left;
        }
    }

    return count + 1;
}

// Function to add in the vector , the players with rank in the range [a, b]
void AVL_Tree::rangeNode(Node *node, int a, int b, int baseRank, vector<Player> &result) const
{

    // If the node is null, there's nothing to process
    if (!node)
        return;

    // Calculate the current rank of the node based on the base rank and the size of the left subtree
    int currentRank = baseRank + getSize(node->left) + 1;

    // Visit left subtree if it may contain requested positions
    if (a < currentRank)
        rangeNode(node->left, a, b, baseRank, result);

    // Include current player if its position is in the range
    if (currentRank >= a && currentRank <= b)
        result.push_back(node->player);

    // Visit right subtree if it may contain requested positions
    if (b > currentRank)
        rangeNode(node->right, a, b, currentRank, result);
}

// Public function to return a vector of players whose ranks are within the range [a, b]
vector<AVL_Tree::Player> AVL_Tree::range(int a, int b) const
{
    // Create a vector to store the result
    vector<Player> result;

    // Check if the range is valid; if not, return an empty vector
    if (a < 1 || b < a || a > getSize(root))
        return result;

    // Adjust the upper bound if it exceeds the tree size
    if (b > getSize(root))
        b = getSize(root);

    // Call the recursive function to populate the result vector with players in the specified range
    rangeNode(root, a, b, 0, result);

    // Return the vector containing players whose ranks are within the specified range
    return result;
}

// Public function to return the size of the AVL tree
int AVL_Tree::size() const
{
    return getSize(root);
}

// Public function to return the height of the AVL tree
int AVL_Tree::height() const
{
    return getHeight(root);
}

// Recursive function to destroy the AVL tree and free memory
void AVL_Tree::destroy(Node *node)
{
    if (!node)
        return;

    destroy(node->left);
    destroy(node->right);
    delete node;
}

// Destructor for the AVL_Tree class; calls destroy to free memory
AVL_Tree::~AVL_Tree()
{
    destroy(root);
}

// Return the total number of rotations performed by this AVL tree
unsigned long long AVL_Tree::getRotationCount() const
{
    return rotationCount;
}

// Reset the rotation counter before a new measurement
void AVL_Tree::resetRotationCount()
{
    rotationCount = 0;
}
