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
        "   Decompresses files using the LZ78 compression algorithm.\n"
        "   Used with files compressed with the corresponding encoder.\n"
        "\n"
        "USAGE\n"
        "   %s [-vh] [-i input] [-o output]\n"
        "\n"
        "OPTIONS\n"
        "   -v          Display decompression statistics\n"
        "   -i input    Specify input to decompress (stdin by default)\n"
        "   -o output   Specify output of decompressed input (stdout by default)\n"
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

    // open standard input and output if not specified
    if (infile == -1) {
        infile = STDIN_FILENO;
    }

    // read header
    FileHeader header = { 0, 0 };
    read_header(infile, &header);

    if (out != NULL) {
        outfile = open(out, O_WRONLY | O_CREAT | O_TRUNC, header.protection);
        if (outfile == -1) {
            printf("Failed to open %s.\n", out);
            return 1;
        }
    } else {
        outfile = STDOUT_FILENO;
    }

    // start decompressing
    WordTable *table = wt_create();
    uint8_t curr_sym = 0;
    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;

    while (read_pair(infile, &curr_code, &curr_sym, bit_length(next_code))) {
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        write_word(outfile, table[next_code]);
        next_code = next_code + 1;
        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }
    flush_words(outfile);
    wt_delete(table);

    // if verbose is enabled print statistics
    if (verbose) {
        uint64_t comp = total_bits / 8;
        if (total_bits % 8 != 0) {
            comp++;
        }
        uint64_t uncomp = total_syms;
        float save = 100 * (1 - ((float) comp / (float) uncomp));
        printf("Compressed file size: %" PRIu64 " bytes\n", comp);
        printf("Uncompressed file size: %" PRIu64 " bytes\n", uncomp);
        printf("Compression ratio: %0.2f %%\n", save);
    }

    close(infile);
    close(outfile);

    return 0;
}
