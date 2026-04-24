#include "bst_tree.h"

void bst_tree::insert(int64_t data) {
    auto new_node = new node(data);
    new_node->left = nullptr;
    new_node->right = nullptr;

    // Prázdný strom
    node* expected = nullptr;
    if (root.compare_exchange_strong(expected, new_node)) {
        return;
    }

    auto compare_node = root.load();

    while (true)
    {
        if (compare_node->data > new_node->data) {
            if (compare_node->left != nullptr) {
                compare_node = compare_node->left;
            } else {
                node* expected = nullptr;
                if (compare_node->left.compare_exchange_strong(expected, new_node)) {
                    break;
                }
                compare_node = compare_node->left.load();
            }
        } else {
            if (compare_node->right != nullptr) {
                compare_node = compare_node->right;
            } else {
                node* expected = nullptr;
                if (compare_node->right.compare_exchange_strong(expected, new_node)) {
                    break;
                }
                compare_node = compare_node->right.load();
            }
        }
    }
}

// Rekurzivni funkce pro pruchod stromu a dealokaci pameti prirazene jednotlivym uzlum
void delete_node(bst_tree::node* node) {
    if (node == nullptr) {
        return;
    }

    delete_node(node->left);
    delete_node(node->right);
    delete node;
}

bst_tree::~bst_tree() {
    delete_node(root);
}
