#include "bst_tree.h"

void bst_tree::insert(int64_t data) {
    node* new_node = new node(data);

    // TODO: Naimplementujte zde vlaknove-bezpecne vlozeni do binarniho vyhledavaciho stromu
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
