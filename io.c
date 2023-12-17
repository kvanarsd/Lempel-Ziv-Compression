#include "io.h"
#include "endian.h"
#include "code.h"

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

static uint8_t buf_char[BLOCK];
static uint8_t buf_pair[BLOCK];

static uint32_t cur_char = 0;
static uint32_t pair_cur = 0;

uint64_t total_bits = 0;
uint64_t total_syms = 0;

// loop calls read() to make sure it reads to correct amount of bytes
// return number of bytes read
int read_bytes(int infile, uint8_t *buf, int to_read) {
    int i = 0;
    int readB = 0;
    while (i < to_read) {
        readB = read(infile, buf + i, to_read - i);
        i += readB;
        if (readB == 0) {
            break;
        }
    }
    return i;
}

// loop calls write() to make sure it reads to correct amount of bytes
// return number of bytes written
int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int i = 0;
    int writeB = 0;
    while (i < to_write) {
        writeB = write(outfile, buf + i, to_write - i);
        i += writeB;
        if (writeB == 0) {
            break;
        }
    }
    return i;
}

void read_header(int infile, FileHeader *header) {
    // read file header
    if ((total_bits += read_bytes(infile, (uint8_t *) header, sizeof(FileHeader)))
        != sizeof(FileHeader)) {
        printf("Failed to read infile header.\n");
        exit(EXIT_FAILURE);
    }
    total_bits *= 8;

    // check endianess
    if (!little_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }

    // compare magic number
    if (header->magic != MAGIC) {
        printf("Invalid magic number from infile.\n");
        exit(EXIT_FAILURE);
    }
    return;
}

void write_header(int outfile, FileHeader *header) {
    // check endianess
    if (!little_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }

    // write file header
    if ((total_bits += write_bytes(outfile, (uint8_t *) header, sizeof(FileHeader)))
        != sizeof(FileHeader)) {
        printf("Failed to write outfile header.\n");
        exit(EXIT_FAILURE);
    }
    total_bits *= 8;
    return;
}

bool read_sym(int infile, uint8_t *sym) {
    static size_t buf_len = 0; // amount of bytes in buffer
    //static size_t cur_pos = 0; // current position in buffer

    // if there are no symbols in the buffer
    // or there are no symbols left in the buffer
    // read next block
    if (cur_char >= buf_len) {
        buf_len = read_bytes(infile, buf_char, BLOCK);
        cur_char = 0;
    }

    // return false if no bytes were read
    if (buf_len == 0) {
        return false;
    }

    // read symbol from buffer into sym
    *sym = buf_char[cur_char];
    cur_char++;
    total_syms++;

    // if reached the end of buffer reset length of buffer and current position
    if (cur_char >= buf_len) {
        buf_len = 0;
        cur_char = 0;
    }

    return true;
}
void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    // put code bits into buffer
    for (int i = 0; i < bitlen; i++) {
        buf_pair[pair_cur / 8] |= ((code >> i) & 1) << (pair_cur % 8);
        pair_cur++;
        total_bits++;
        // write buffer to outfile
        if (pair_cur >= BLOCK * 8) {
            write_bytes(outfile, buf_pair, BLOCK);
            pair_cur = 0;
            memset(buf_pair, 0, BLOCK);
        }
    }

    // put sym bits into buffer
    for (uint32_t i = 0; i < 8; i++) {
        buf_pair[pair_cur / 8] |= ((sym >> i) & 1) << (pair_cur % 8);
        pair_cur++;
        total_bits++;
        // write buffer to outfile
        if (pair_cur >= BLOCK * 8) {
            write_bytes(outfile, buf_pair, BLOCK);
            pair_cur = 0;
            memset(buf_pair, 0, BLOCK);
        }
    }
    //total_syms++;

    return;
}

void flush_pairs(int outfile) {
    // write out rest of buffer
    uint32_t leftovers = pair_cur / 8;
    // if number of bits in buffer is not divisible by 8 add one
    if (pair_cur % 8 != 0) {
        leftovers++;
    }
    write_bytes(outfile, buf_pair, leftovers);
    return;
}

bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    static size_t buf_len = 0; // amount of bytes in buffer
    static size_t cur = 0; // current position in buffer

    // if there are no pairs in the buffer
    // or there are no bytes left in the buffer
    // read next block
    if (cur >= buf_len * 8) {
        buf_len = read_bytes(infile, buf_pair, BLOCK);
        cur = 0;
    }

    // read code from buffer into code
    *code = 0;
    for (int i = 0; i < bitlen; i++) {
        uint8_t LSB = buf_pair[cur / 8] & (1 << (cur % 8));
        *code |= (LSB >> (cur % 8)) << i;
        cur++;
        total_bits++;
        if (cur >= buf_len * 8) {
            buf_len = read_bytes(infile, buf_pair, BLOCK);
            cur = 0;
        }
    }

    // read symbol from buffer into sym
    *sym = 0;
    for (int i = 0; i < 8; i++) {
        uint8_t LSB = buf_pair[cur / 8] & (1 << (cur % 8));
        *sym |= (LSB >> (cur % 8)) << i;
        cur++;
        total_bits++;
        if (cur >= buf_len * 8) {
            buf_len = read_bytes(infile, buf_pair, BLOCK);
            cur = 0;
        }
    }
    //total_syms++;

    if (*code != STOP_CODE) {
        return true;
    } else {
        return false;
    }
}

void write_word(int outfile, Word *w) {
    for (uint32_t i = 0; i < w->len; i++) {
        buf_char[cur_char] = w->syms[i];
        cur_char++;
        total_syms++;
        //if buffer is full write it
        if (cur_char >= BLOCK) {
            write_bytes(outfile, buf_char, BLOCK);
            cur_char = 0;
        }
    }

    return;
}

void flush_words(int outfile) {
    // write out rest of buffer
    write_bytes(outfile, buf_char, cur_char);
    return;
}
