#include "io.h"
#include "word.h"
#include "trie.h"
#include "code.h"

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define OPTIONS "i:o:vh"

void synopsis(char *exec) {
    fprintf(stderr,
        "SYNOPSIS\n"
        "   Compresses files using the LZ78 compression algorithm.\n"
        "   Compressed files are decompressed with the corresponding decoder.\n"
        "\n"
        "USAGE\n"
        "   %s [-vh] [-i input] [-o output]\n"
        "\n"
        "OPTIONS\n"
        "   -v          Display compression statistics\n"
        "   -i input    Specify input to compress (stdin by default)\n"
        "   -o output   Specify output of compressed input (stdout by default)\n"
        "   -h          Display program help and usage\n",
        exec);
}

int bit_length(uint16_t n) {
    // counting bit length of next code
    int bitlen = 0;
    while (n) {
        bitlen++;
        n >>= 1;
    }
    return bitlen;
}

int main(int argc, char **argv) {
    int verbose = 0;
    int infile = -1;
    int outfile = -1;
    char *out = NULL;

    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'v': verbose = 1; break;
        case 'i':
            infile = open(optarg, O_RDONLY | O_EXCL);
            if (infile == -1) {
                printf("Failed to open %s.\n", optarg);
                return 1;
            }
            break;
        case 'o': out = optarg; break;
        case 'h': synopsis(argv[0]); return 0;
        default: synopsis(argv[0]); return 1;
        }
    }
    // open standard input not specified
    if (infile == -1) {
        infile = STDIN_FILENO;
    }

    // get header information from infile
    struct stat file_info;
    if (fstat(infile, &file_info) == -1) {
        printf("Failed to read infile information\n");
        return 1;
    }

    FileHeader header = { MAGIC, 0 };
    header.magic = MAGIC;
    header.protection = file_info.st_mode;

    // open output file
    if (out != NULL) {
        outfile = open(out, O_WRONLY | O_CREAT | O_TRUNC, header.protection);
        if (outfile == -1) {
            printf("Failed to open %s.\n", out);
            return 1;
        }
    } else {
        outfile = STDOUT_FILENO;
    }

    // write header to output
    write_header(outfile, &header);

    // start compression
    // defining variables
    // referenced from asgn6.pdf
    TrieNode *root = trie_create();
    TrieNode *curr_node = root;
    TrieNode *prev_node = NULL;
    TrieNode *next_node = NULL;
    uint8_t curr_sym = 0;
    uint8_t prev_sym = 0;
    uint16_t next_code = START_CODE;

    // read and compress file
    while (read_sym(infile, &curr_sym)) {
        next_node = trie_step(curr_node, curr_sym);
        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            write_pair(outfile, curr_node->code, curr_sym, bit_length(next_code));
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code = next_code + 1;
        }

        if (next_code == MAX_CODE) {
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }
        prev_sym = curr_sym;
    }
    if (curr_node != root) {
        write_pair(outfile, prev_node->code, prev_sym, bit_length(next_code));
        next_code = (next_code + 1) % MAX_CODE;
    }
    write_pair(outfile, STOP_CODE, 0, bit_length(next_code));
    flush_pairs(outfile);

    // if verbose is enabled print statistics
    if (verbose) {
        uint64_t comp = total_bits / 8;
        if (total_bits % 8 != 0) {
            comp++;
        }
        uint64_t uncomp = total_syms;
        float save = 100.0 * (1.0 - ((float) comp / (float) uncomp));
        printf("Compressed file size: %" PRIu64 " bytes\n", comp);
        printf("Uncompressed file size: %" PRIu64 " bytes\n", uncomp);
        printf("Compression ratio: %0.2f%%\n", save);
    }

    close(infile);
    close(outfile);
    trie_delete(root);

    return 0;
}
