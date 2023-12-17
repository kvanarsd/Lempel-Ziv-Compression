CC = clang
CFLAGS = -Wall -Wextra -Werror -Wpedantic
EXEC = encode decode
OBJECTS = io.o trie.o word.o

all: $(EXEC)

encode: encode.o $(OBJECTS)
	$(CC) -o $@ $^

decode: decode.o $(OBJECTS)
	$(CC) -o $@ $^

io.o: io.c
	$(CC) $(CFLAGS) -c io.c
	
trie.o: trie.c
	$(CC) $(CFLAGS) -c trie.c

word.o: word.c
	$(CC) $(CFLAGS) -c word.c

clean:
	rm -f $(EXEC) $(OBJECTS) encode.o decode.o
format:
	clang-format -i -style=file *.[ch]

scan-build: clean
	scan-build --use-cc=$(CC) make
