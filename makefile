CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -g

all: PideShop HungryVeryMuch

build/%.o: %.c utils.h
	mkdir -p build
	$(CC) $(CCFLAGS) -c $< -o $@

PideShop: build/server.o build/utils.o
	$(CC) $(CCFLAGS) $^ -o $@ 

HungryVeryMuch: build/client.o build/utils.o
	$(CC) $(CCFLAGS) $^ -o $@ 

clean:
	rm -rf  PideShop HungryVeryMuch

client: HungryVeryMuch
	clear
	valgrind -q ./HungryVeryMuch 8080 10 10 10

server: PideShop
	clear
	valgrind -q ./PideShop 8080 10 10 10

re: clean all

test: build/utils.o
	$(CC) $(CCFLAGS) test.c build/utils.o -o test.out
	./test.out

.PHONY: all clean re client server test