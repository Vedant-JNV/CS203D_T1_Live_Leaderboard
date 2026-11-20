// Define the header guard to prevent multiple inclusions of this header file
#ifndef AVL_HPP
#define AVL_HPP

// Include necessary standard library headers
#include <vector>
using namespace std;

// Define the AVL_Tree class, which implements an AVL tree to store Player objects
class AVL_Tree
{

    // Define the Player struct to represent a player with an ID and score
public:
    struct Player
    {
        long long id;
        long long score;
    };

private:
    // Define the Node struct to represent a node in the AVL tree
    struct Node
    {
        // Each node contains a Player object, pointers to left and right child nodes,
        // the height of the node, and the size of the subtree rooted at this node
        Player player;
        Node *left;
        Node *right;
        int height;
        int subtreeSize;

        // Constructor to initialize a Node with a Player object
        Node(Player p)
            : player(p), left(nullptr), right(nullptr),
              height(1), subtreeSize(1) {}
    };

    // Pointer to the root node of the AVL tree, initialized to nullptr
    Node *root = nullptr;

    // Counts the total number of rotations performed by this AVL tree
    unsigned long long rotationCount = 0;

    // Metadata helpers
    int getHeight(Node *node) const;
    int getSize(Node *node) const;
    void update(Node *node);
    int getBalance(Node *node) const;

    // Comparison function to determine the ordering of Player objects in the AVL tree
    bool comesBefore(const Player &a, const Player &b) const;

    // Rotations and balancing
    Node *rotateLeft(Node *node);
    Node *rotateRight(Node *node);
    Node *rebalance(Node *node);

    // Internal operations
    Node *insertNode(Node *node, Player player);
    Node *eraseNode(Node *node, Player player);
    Node *findNode(Player player) const;
    Node *findMin(Node *node) const;
    void destroy(Node *node);
    void rangeNode(Node *node, int a, int b, int baseRank, vector<Player> &result) const;

    friend struct InvariantChecker;

public:
    AVL_Tree() = default;
    ~AVL_Tree();

    AVL_Tree(const AVL_Tree &) = delete;
    AVL_Tree &operator=(const AVL_Tree &) = delete;

    // Public operations
    void insert(long long id, long long score);
    bool erase(long long id, long long score);
    // Check if a player with the given id and score exists in the tree
    bool contains(long long id, long long score) const;
    // Select the k-th player in the sorted order and store it in result
    bool select(int k, Player &result) const;
    // Competition rank for a score: 1 + number of players with a strictly higher score.
    // Single root-to-leaf walk. It does not check that a player with this score exists
    // (the caller, e.g. the application's hash table, is expected to have done that).
    int rank(long long score) const;

    // Return a vector of players whose scores are within the range [a, b]
    vector<Player> range(int a, int b) const;

    int size() const;
    int height() const;

    // Return the total number of rotations performed
    unsigned long long getRotationCount() const;

    // Reset the rotation counter
    void resetRotationCount();
};

// End of the header guard
#endif
