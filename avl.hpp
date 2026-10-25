// Define the header guard to prevent multiple inclusions of this header file
#ifndef AVL_HPP
#define AVL_HPP

// Include necessary standard library headers
#include <vector>
using namespace std;

// Define the AVL_Tree class, which implements an AVL tree to store Player objects
class AVL_Tree {

// Define the Player struct to represent a player with an ID and score
public:
    struct Player {
        int id;
        long score;
    };

private:
    // Define the Node struct to represent a node in the AVL tree
    struct Node {
        // Each node contains a Player object, pointers to left and right child nodes,
        // the height of the node, and the size of the subtree rooted at this node
        Player player;
        Node* left;
        Node* right;
        int height;
        int subtreeSize;

        // Constructor to initialize a Node with a Player object
        Node(Player p)
            : player(p), left(nullptr), right(nullptr),
              height(1), subtreeSize(1) {}
    };

    // Pointer to the root node of the AVL tree, initialized to nullptr
    Node* root = nullptr;

    // Metadata helpers
    int getHeight(Node* node) const;
    int getSize(Node* node) const;
    void update(Node* node);
    int getBalance(Node* node) const;

    // Comparison function to determine the ordering of Player objects in the AVL tree
    bool comesBefore(const Player& a, const Player& b) const;

    // Rotations and balancing
    Node* rotateLeft(Node* node);
    Node* rotateRight(Node* node);
    Node* rebalance(Node* node);

    // Internal operations
    Node* insertNode(Node* node, Player player);
    Node* eraseNode(Node* node, Player player);
    Node* findNode(Player player) const;
    void destroy(Node* node);

public:
    AVL_Tree() = default;
    ~AVL_Tree();

    AVL_Tree(const AVL_Tree&) = delete;
    AVL_Tree& operator=(const AVL_Tree&) = delete;

    // Public operations
    void insert(int id, long score);
    bool erase(int id, long score);
    // Check if a player with the given id and score exists in the tree
    bool contains(int id, long score) const;
    // Select the k-th player in the sorted order and store it in result
    bool select(int k, Player& result) const;
    // Find the rank of a player with the given id and score, and store it in result
    bool rank(int id, long score, int& result) const;

    // Return a vector of players whose scores are within the range [a, b]
    vector<Player> range(int a, int b) const;

    int size() const;
    int height() const;
};

// End of the header guard
#endif
