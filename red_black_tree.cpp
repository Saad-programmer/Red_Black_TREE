#include<iostream>

struct Node {
    int data;
    Node* left;
    Node* right;
    bool color; // true for red, false for black

    Node(int value) : data(value), left(nullptr), right(nullptr), color(true) {}
    int isRed() const {
        return color;
    }
    int isBlack() const {
        return !color;
    }
};

void createNode(Node*& root, int value) {
    root = new Node(value);
}

void insert(Node*& root, int value) {
    Node* newNode = new Node(value);
    if (root == nullptr) {
        root = newNode;
        root->color = false; // Root is always black
    } else {
        if(value < root->data) {
            insert(root->left, value);
        } else {
            insert(root->right, value);
        }
    }
}

void displayTree(Node* root) {
    if (root == nullptr) {
        return;
    }
    displayTree(root->left);
    std::cout << "Node: " << root->data << ", Color: " << (root->color ? "Red" : "Black") << std::endl;
    displayTree(root->right);
}

void deleteTree(Node* root) {
    if (root == nullptr) {
        return;
    }
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

void styledDisplay(Node* root) {
    if (root == nullptr) {
        return;
    }
    styledDisplay(root->left);
    std::cout << "Node: " << root->data << ", Color: " << (root->color ? "Red" : "Black") << "Parent: "<< root <<std::endl;
    styledDisplay(root->right);
}
int main() {
    Node* root = nullptr;
    createNode(root, 59);
    /* createNode(root->left, 5);
    createNode(root->right, 15); */
    insert(root, 48);
    insert(root, 7);
    insert(root, 38);
    insert(root, 88);
    insert(root, 67);


    displayTree(root);

    // Clean up memory
    deleteTree(root);

    return 0;
}

/*

 alan@alan-HP-EliteBook-850-G8-Notebook-PC:~/Documents/ILISI2/Virtualisation/C++/REdBlack$ ./assignement

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: 1
Enter value: 59
R----59(BLACK)

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: 1
Enter value: 48
R----59(BLACK)
 L----48(RED)

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: 1
Enter value: 7
R----48(BLACK)
 L----7(RED)
 R----59(RED)

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: 1
Enter value: 38
R----48(BLACK)
 L----7(BLACK)
 | R----38(RED)
 R----59(BLACK)

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: 1
Enter value: 88
R----48(BLACK)
 L----7(BLACK)
 | R----38(RED)
 R----59(BLACK)
  R----88(RED)

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: 1
Enter value: 67
R----48(BLACK)
 L----7(BLACK)
 | R----38(RED)
 R----67(BLACK)
  L----59(RED)
  R----88(RED)

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: 4
R----48(BLACK)
 L----7(BLACK)
 | R----38(RED)
 R----67(BLACK)
  L----59(RED)
  R----88(RED)

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: 2
Enter value: 38
R----48(BLACK)
 L----7(BLACK)
 R----67(BLACK)
  L----59(RED)
  R----88(RED)

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: 3     
Enter value: 67
Found

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: 4
R----48(BLACK)
 L----7(BLACK)
 R----67(BLACK)
  L----59(RED)
  R----88(RED)

1.Insert
2.Delete
3.Search
4.Display
5.Exit
Choice: */