#include <iostream>
#include <string>
#include <vector>
#include <limits>
using namespace std;

// ANSI styles
const string RESET   = "\033[0m";
const string RED_BG  = "\033[1;97;41m";   // red node: white bold on red
const string BLK_BG  = "\033[1;97;100m";  // black node: white bold on dark gray
const string DIM     = "\033[2m";
const string CYAN    = "\033[1;36m";
const string GREEN   = "\033[1;32m";
const string YELLOW  = "\033[1;33m";

struct Node {
    int data;
    bool color; // true = RED, false = BLACK
    Node *left, *right, *parent;
};

class RedBlackTree {
private:
    Node* root;
    Node* TNULL;

    void initializeNULLNode(Node* node, Node* parent) {
        node->data = 0;
        node->color = false;
        node->left = nullptr;
        node->right = nullptr;
        node->parent = parent;
    }

    void leftRotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left != TNULL) y->left->parent = x;
        y->parent = x->parent;
        if (x->parent == nullptr) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }

    void rightRotate(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        if (y->right != TNULL) y->right->parent = x;
        y->parent = x->parent;
        if (x->parent == nullptr) root = y;
        else if (x == x->parent->right) x->parent->right = y;
        else x->parent->left = y;
        y->right = x;
        x->parent = y;
    }

    void fixInsert(Node* k) {
        Node* u;
        while (k->parent && k->parent->color == true) {// parent existe et est rouge
            if (k->parent == k->parent->parent->right) {// parent est le fils droit du grand-parent
                u = k->parent->parent->left;// oncle recupéré
                if (u->color == true) {// si l'oncle est rouge
                    u->color = false;
                    k->parent->color = false;
                    k->parent->parent->color = true;
                    k = k->parent->parent;
                } else {
                    if (k == k->parent->left) {
                        k = k->parent;
                        rightRotate(k);
                    }
                    k->parent->color = false;
                    k->parent->parent->color = true;
                    leftRotate(k->parent->parent);
                }
            } else {
                u = k->parent->parent->right;
                if (u->color == true) {
                    u->color = false;
                    k->parent->color = false;
                    k->parent->parent->color = true;
                    k = k->parent->parent;
                } else {
                    if (k == k->parent->right) {
                        k = k->parent;
                        leftRotate(k);
                    }
                    k->parent->color = false;
                    k->parent->parent->color = true;
                    rightRotate(k->parent->parent);
                }
            }
            if (k == root) break;
        }
        root->color = false;
    }

    void transplant(Node* u, Node* v) {
        if (u->parent == nullptr) root = v;
        else if (u == u->parent->left) u->parent->left = v;
        else u->parent->right = v;
        v->parent = u->parent;
    }

    Node* minimum(Node* node) {
        while (node->left != TNULL) node = node->left;
        return node;
    }

    void fixDelete(Node* x) {
        Node* s;
        while (x != root && x->color == false) {
            if (x == x->parent->left) {
                s = x->parent->right;
                if (s->color == true) {
                    s->color = false;
                    x->parent->color = true;
                    leftRotate(x->parent);
                    s = x->parent->right;
                }
                if (s->left->color == false && s->right->color == false) {
                    s->color = true;
                    x = x->parent;
                } else {
                    if (s->right->color == false) {
                        s->left->color = false;
                        s->color = true;
                        rightRotate(s);
                        s = x->parent->right;
                    }
                    s->color = x->parent->color;
                    x->parent->color = false;
                    s->right->color = false;
                    leftRotate(x->parent);
                    x = root;
                }
            } else {
                s = x->parent->left;
                if (s->color == true) {
                    s->color = false;
                    x->parent->color = true;
                    rightRotate(x->parent);
                    s = x->parent->left;
                }
                if (s->right->color == false && s->left->color == false) {
                    s->color = true;
                    x = x->parent;
                } else {
                    if (s->left->color == false) {
                        s->right->color = false;
                        s->color = true;
                        leftRotate(s);
                        s = x->parent->left;
                    }
                    s->color = x->parent->color;
                    x->parent->color = false;
                    s->left->color = false;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = false;
    }

    // Colored badge for a node, e.g. " 15 "
    string badge(Node* n) {
        return (n->color ? RED_BG : BLK_BG) + " " + to_string(n->data) + " " + RESET;
    }

    // Prints the tree sideways-as-a-list with box-drawing lines.
    // Right child is listed first, left child second.
    void printNode(Node* node, const string& prefix, bool isLast, bool isRoot, const string& side) {
        cout << prefix;
        if (!isRoot) cout << DIM << (isLast ? "└── " : "├── ") << RESET;
        if (!side.empty()) cout << DIM << side << RESET << " ";
        cout << badge(node) << "\n";

        string childPrefix = prefix;
        if (!isRoot) childPrefix += DIM + string(isLast ? "    " : "│   ") + RESET;

        vector<pair<Node*, string>> kids;
        if (node->right != TNULL) kids.push_back({node->right, "R:"});
        if (node->left  != TNULL) kids.push_back({node->left,  "L:"});

        for (size_t i = 0; i < kids.size(); i++)
            printNode(kids[i].first, childPrefix, i == kids.size() - 1, false, kids[i].second);
    }

    int countNodes(Node* n) {
        if (n == TNULL) return 0;
        return 1 + countNodes(n->left) + countNodes(n->right);
    }

    int height(Node* n) {
        if (n == TNULL) return 0;
        int l = height(n->left), r = height(n->right);
        return 1 + (l > r ? l : r);
    }

public:
    RedBlackTree() {
        TNULL = new Node;
        initializeNULLNode(TNULL, nullptr);
        root = TNULL;
    }

    void insert(int key) {
        Node* node = new Node;
        node->parent = nullptr;
        node->data = key;
        node->left = TNULL;
        node->right = TNULL;
        node->color = true;

        Node* y = nullptr;
        Node* x = root;// 
        while (x != TNULL) {
            y = x;
            if (node->data < x->data) x = x->left;
            else x = x->right;
        }

        node->parent = y;
        if (y == nullptr) root = node;
        else if (node->data < y->data) y->left = node;
        else y->right = node;

        if (node->parent == nullptr) {
            node->color = false;
            return;
        }
        if (node->parent->parent == nullptr) return;
        fixInsert(node);
    }

    bool deleteNode(int key) {
        Node* z = root;
        Node* x;
        Node* y;

        while (z != TNULL) {
            if (z->data == key) break;
            if (key < z->data) z = z->left;
            else z = z->right;
        }
        if (z == TNULL) return false;

        y = z;
        bool y_original_color = y->color;

        if (z->left == TNULL) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == TNULL) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            y_original_color = y->color;
            x = y->right;
            if (y->parent == z) x->parent = y;
            else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        delete z;
        if (y_original_color == false) fixDelete(x);
        return true;
    }

    bool search(int key) {
        Node* temp = root;
        while (temp != TNULL) {
            if (key == temp->data) return true;
            temp = key < temp->data ? temp->left : temp->right;
        }
        return false;
    }

    void display() {
        cout << "\n" << CYAN << "┌─ Red-Black Tree ";
        cout << "─────────────────────────────" << RESET << "\n";
        if (root == TNULL) {
            cout << DIM << "  (empty)" << RESET << "\n";
        } else {
            printNode(root, "  ", true, true, "");
            cout << "\n  " << RED_BG << " red " << RESET << " "
                 << BLK_BG << " black " << RESET
                 << DIM << "   nodes: " << countNodes(root)
                 << "  height: " << height(root) << RESET << "\n";
        }
        cout << CYAN << "└──────────────────────────────────────────────" << RESET << "\n";
    }
};

int readInt(const string& prompt, int& out) {
    cout << prompt;
    if (!(cin >> out)) {
        if (cin.eof()) return -1;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return 0;
    }
    return 1;
}

int main() {
    RedBlackTree tree;
    int choice, value;

    while (true) {
        cout << "\n" << CYAN << "╭─────────── MENU ───────────╮" << RESET << "\n"
             << CYAN << "│" << RESET << "  1. Insert                 " << CYAN << "│" << RESET << "\n"
             << CYAN << "│" << RESET << "  2. Delete                 " << CYAN << "│" << RESET << "\n"
             << CYAN << "│" << RESET << "  3. Search                 " << CYAN << "│" << RESET << "\n"
             << CYAN << "│" << RESET << "  4. Display                " << CYAN << "│" << RESET << "\n"
             << CYAN << "│" << RESET << "  5. Exit                   " << CYAN << "│" << RESET << "\n"
             << CYAN << "╰────────────────────────────╯" << RESET << "\n";

        int r = readInt("Choice: ", choice);
        if (r == -1) return 0;
        if (r == 0) { cout << YELLOW << "Please enter a number." << RESET << "\n"; continue; }

        switch (choice) {
        case 1:
            if (readInt("Enter value: ", value) != 1) { cout << YELLOW << "Invalid value." << RESET << "\n"; break; }
            tree.insert(value);
            tree.display();
            break;
        case 2:
            if (readInt("Enter value: ", value) != 1) { cout << YELLOW << "Invalid value." << RESET << "\n"; break; }
            if (!tree.deleteNode(value)) cout << YELLOW << "Key not found" << RESET << "\n";
            tree.display();
            break;
        case 3:
            if (readInt("Enter value: ", value) != 1) { cout << YELLOW << "Invalid value." << RESET << "\n"; break; }
            if (tree.search(value)) cout << GREEN << "✔ Found" << RESET << "\n";
            else cout << YELLOW << "✘ Not Found" << RESET << "\n";
            break;
        case 4:
            tree.display();
            break;
        case 5:
            return 0;
        default:
            cout << YELLOW << "Invalid choice." << RESET << "\n";
        }
    }
}

/*
alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/REdBlack$ ./assignement2

╭─────────── MENU ───────────╮
│  1. Insert                 │
│  2. Delete                 │
│  3. Search                 │
│  4. Display                │
│  5. Exit                   │
╰────────────────────────────╯
Choice: 1
Enter value: 59

┌─ Red-Black Tree ─────────────────────────────
   59 

   red   black    nodes: 1  height: 1
└──────────────────────────────────────────────

╭─────────── MENU ───────────╮
│  1. Insert                 │
│  2. Delete                 │
│  3. Search                 │
│  4. Display                │
│  5. Exit                   │
╰────────────────────────────╯
Choice: 1
Enter value: 48

┌─ Red-Black Tree ─────────────────────────────
   59 
  └── L:  48 

   red   black    nodes: 2  height: 2
└──────────────────────────────────────────────

╭─────────── MENU ───────────╮
│  1. Insert                 │
│  2. Delete                 │
│  3. Search                 │
│  4. Display                │
│  5. Exit                   │
╰────────────────────────────╯
Choice: 1
Enter value: 7

┌─ Red-Black Tree ─────────────────────────────
   48 
  ├── R:  59 
  └── L:  7 

   red   black    nodes: 3  height: 2
└──────────────────────────────────────────────

╭─────────── MENU ───────────╮
│  1. Insert                 │
│  2. Delete                 │
│  3. Search                 │
│  4. Display                │
│  5. Exit                   │
╰────────────────────────────╯
Choice: 1
Enter value: 38

┌─ Red-Black Tree ─────────────────────────────
   48 
  ├── R:  59 
  └── L:  7 
      └── R:  38 

   red   black    nodes: 4  height: 3
└──────────────────────────────────────────────

╭─────────── MENU ───────────╮
│  1. Insert                 │
│  2. Delete                 │
│  3. Search                 │
│  4. Display                │
│  5. Exit                   │
╰────────────────────────────╯
Choice: 1
Enter value: 88

┌─ Red-Black Tree ─────────────────────────────
   48 
  ├── R:  59 
  │   └── R:  88 
  └── L:  7 
      └── R:  38 

   red   black    nodes: 5  height: 3
└──────────────────────────────────────────────

╭─────────── MENU ───────────╮
│  1. Insert                 │
│  2. Delete                 │
│  3. Search                 │
│  4. Display                │
│  5. Exit                   │
╰────────────────────────────╯
Choice: 1
Enter value: 67

┌─ Red-Black Tree ─────────────────────────────
   48 
  ├── R:  67 
  │   ├── R:  88 
  │   └── L:  59 
  └── L:  7 
      └── R:  38 

   red   black    nodes: 6  height: 3
└──────────────────────────────────────────────

╭─────────── MENU ───────────╮
│  1. Insert                 │
│  2. Delete                 │
│  3. Search                 │
│  4. Display                │
│  5. Exit                   │
╰────────────────────────────╯
Choice: 2
Enter value: 67

┌─ Red-Black Tree ─────────────────────────────
   48 
  ├── R:  88 
  │   └── L:  59 
  └── L:  7 
      └── R:  38 

   red   black    nodes: 5  height: 3
└──────────────────────────────────────────────

╭─────────── MENU ───────────╮
│  1. Insert                 │
│  2. Delete                 │
│  3. Search                 │
│  4. Display                │
│  5. Exit                   │
╰────────────────────────────╯
Choice: 3
Enter value: 7
✔ Found

╭─────────── MENU ───────────╮
│  1. Insert                 │
│  2. Delete                 │
│  3. Search                 │
│  4. Display                │
│  5. Exit                   │
╰────────────────────────────╯
Choice: 
*/