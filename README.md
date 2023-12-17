#Asignment 6: Lempel-Ziv Compression

## Description:
This program provides an encode program and a decode program. The encode program preforms LZ78
compression on a given file while the decode program preforms LZ78 decompression.

## Build:
Make sure the supporting function files trie.c, word.c, io.c and header files trie.h, word.h, io.h, endian.h, and code.h are in the directory. Also make sure decode.c, encode.c, and Makefile are included. If you only want to make one executable you can call 'make encode' to make the excutable encode or call 'make decode' to make the excutable decode. Call 'make' or 'make all' to make both. 

## Cleaning:
Calling 'make clean' will remove all made excutables and .o files from directory.

## Running:
Calling either executable with -h will print the usage.

## Running Encode:
Encode's valid arguments are 'i:o:vh'. -i with a file name specifies the input to compress, stdin by default. -o with a file name specifies the output of compressed input, stdout by default. -v enables verbose and prints out the compression statistics. -h prints out the usage.

## Running Decode:
Decode's valid arguments are 'i:o:vh'. -i with a file name specifies the input to decompress, stdin by default. -o with a file name specifies the output of decompressed input, stdout by default. -v enables verbose and prints out the compression statistics. -h prints out the usage.

