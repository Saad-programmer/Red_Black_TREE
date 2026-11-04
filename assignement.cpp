 #include <iostream>
  using namespace std;

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
      node->color = false; // BLACK
      node->left = nullptr;
      node->right = nullptr;
      node->parent = parent;
    }

    // Left Rotation
    void leftRotate(Node* x) {
       Node* y = x->right;
       x->right = y->left;

        if (y->left != TNULL)
           y->left->parent = x;

        y->parent = x->parent;

        if (x->parent == nullptr)
           root = y;
        else if (x == x->parent->left)
           x->parent->left = y;
        else
           x->parent->right = y;

        y->left = x;
        x->parent = y;
    }

    // Right Rotation
    void rightRotate(Node* x) {
       Node* y = x->left;
    x->left = y->right;

    if (y->right != TNULL)
       y->right->parent = x;

    y->parent = x->parent;

    if (x->parent == nullptr)
       root = y;
    else if (x == x->parent->right)
       x->parent->right = y;
    else
       x->parent->left = y;

    y->right = x;
    x->parent = y;
}

// Fix Insertion
void fixInsert(Node* k) {
   Node* u;
   while (k->parent && k->parent->color == true) {
      if (k->parent == k->parent->parent->right) {
         u = k->parent->parent->left;

         if (u->color == true) {
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
      if (k == root)
         break;
    }
    root->color = false; // Root always BLACK
}

void transplant(Node* u, Node* v) {
  if (u->parent == nullptr)
     root = v;
  else if (u == u->parent->left)
     u->parent->left = v;
  else
     u->parent->right = v;
  v->parent = u->parent;
}

Node* minimum(Node* node) {
  while (node->left != TNULL)
     node = node->left;
  return node;
}

// Fix Deletion
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

void printTree(Node* root, string indent, bool last) {
  if (root != TNULL) {
     cout << indent;
     if (last) {
        cout << "R----";
        indent += " ";
     } else {
        cout << "L----";
        indent += "| ";
     }

        string sColor = (root->color) ? "RED" : "BLACK";
        cout << root->data << "(" << sColor << ")" << endl;

        printTree(root->left, indent, false);
        printTree(root->right, indent, true);
    }
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
    node->color = true; // RED

      Node* y = nullptr;
      Node* x = root;

      while (x != TNULL) {
        y = x;
        if (node->data < x->data)
           x = x->left;
        else
           x = x->right;
      }

      node->parent = y;
      if (y == nullptr)
         root = node;
      else if (node->data < y->data)
         y->left = node;
      else
         y->right = node;

      if (node->parent == nullptr) {
         node->color = false;
         return;
      }

      if (node->parent->parent == nullptr)
         return;

      fixInsert(node);
  }

  void deleteNode(int key) {
    Node* z = root;
    Node* x;
    Node* y;

      while (z != TNULL) {
        if (z->data == key)
           break;
        if (key < z->data)
           z = z->left;
        else
           z = z->right;
    }

    if (z == TNULL) {
       cout << "Key not found\n";
       return;
    }

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

        if (y->parent == z)
           x->parent = y;
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

    if (y_original_color == false)
       fixDelete(x);
}

bool search(int key) {
  Node* temp = root;
  while (temp != TNULL) {
     if (key == temp->data)
        return true;
     if (key < temp->data)
        temp = temp->left;
     else
            temp = temp->right;
         }
         return false;
     }

     void display() {
       if (root)
          printTree(root, "", true);
     }
};

int main() {
   RedBlackTree tree;
   int choice, value;

     while (true) {
       cout << "\n1.Insert\n2.Delete\n3.Search\n4.Display\n5.Exit\nChoice: ";
       cin >> choice;

         switch (choice) {
         case 1:
           cout << "Enter value: ";
           cin >> value;
           tree.insert(value);
           tree.display();
           break;
         case 2:
           cout << "Enter value: ";
           cin >> value;
           tree.deleteNode(value);
           tree.display();
           break;
         case 3:
           cout << "Enter value: ";
           cin >> value;
           if (tree.search(value))
              cout << "Found\n";
           else
              cout << "Not Found\n";
           break;
         case 4:
           tree.display();
           break;
         case 5:
           return 0;
         }
     }
}
