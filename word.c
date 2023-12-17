#include "word.h"
#include "code.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

Word *word_create(uint8_t *syms, uint32_t len) {
    // create word
    Word *w = (Word *) calloc(1, sizeof(Word));
    if (w == NULL) {
        return NULL;
    }

    // create an array of syms for word
    w->syms = (uint8_t *) calloc(len, sizeof(uint8_t));

    if (len == 0) {
        w->syms = NULL;
    }
    if (w->syms == NULL && len != 0) {
        free(w->syms);
        free(w);
        return NULL;
    }

    for (uint32_t i = 0; i < len; i++) {
        w->syms[i] = syms[i];
    }
    w->len = len;
    return w;
}

Word *word_append_sym(Word *w, uint8_t sym) {
    // create an array of syms for word
    uint32_t len;
    if (w != NULL) {
        len = w->len;
        len += 1;
    } else {
        len = 1;
    }
    uint8_t syms[len];

    if (w != NULL) {
        for (uint32_t i = 0; i < w->len; i++) {
            syms[i] = w->syms[i];
        }
    }

    syms[len - 1] = sym;

    return word_create(syms, len);
}

void word_delete(Word *w) {
    // delete word's syms
    if (w->syms != NULL) {
        free(w->syms);
    }
    // delete word
    free(w);
    return;
}

WordTable *wt_create(void) {
    // allocate an array
    WordTable *wt = (WordTable *) calloc(MAX_CODE, sizeof(WordTable));
    // Create an empty word and set it at empty_code in the array
    wt[EMPTY_CODE] = word_create(NULL, 0);
    return wt;
}

void wt_reset(WordTable *wt) {
    // set all words other than the first empty word to NULL
    for (uint16_t i = 2; i < MAX_CODE; i++) {
        if (wt[i] != NULL) {
            word_delete(wt[i]);
            wt[i] = NULL;
        }
    }
    return;
}

void wt_delete(WordTable *wt) {
    // free all words in the table
    word_delete(wt[EMPTY_CODE]);
    wt_reset(wt);

    free(wt);
    return;
}
