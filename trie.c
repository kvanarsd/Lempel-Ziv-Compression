#include "trie.h"
#include "code.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

TrieNode *trie_node_create(uint16_t code) {
    // allocate memory for the node
    // check that it allocated correctly with assert
    TrieNode *n = (TrieNode *) calloc(1, sizeof(TrieNode));
    assert(n);
    n->code = code;

    // referenced from digitalocean.com
    // set nodes children to null
    for (int i = 0; i < ALPHABET; i++) {
        n->children[i] = NULL;
    }

    return n;
}

void trie_node_delete(TrieNode *n) {
    // delete node
    free(n);
    n = NULL;
    return;
}

TrieNode *trie_create(void) {
    // create a trie with an empty code
    return trie_node_create(EMPTY_CODE);
}

void trie_reset(TrieNode *root) {
    // if root is empty do nothing
    if (root == NULL) {
        return;
    }

    // delete the root's children
    for (int i = 0; i < ALPHABET; i++) {
        trie_reset(root->children[i]);
        root->children[i] = NULL;
    }

    return;
}

void trie_delete(TrieNode *n) {
    // if n is empty do nothing
    if (n == NULL) {
        return;
    }

    // delete n and it's children
    for (int i = 0; i < ALPHABET; i++) {
        trie_delete(n->children[i]);
        n->children[i] = NULL;
    }
    trie_node_delete(n);
    return;
}

TrieNode *trie_step(TrieNode *n, uint8_t sym) {
    if (n == NULL) {
        return NULL;
    }

    return n->children[sym];
}
