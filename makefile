CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -ansi -g

incs = $(wildcard src/*.h)
objs = $(patsubst src/%.c, build/%.o, $(wildcard src/*.c))

all: PideShop HungryVeryMuch

build/%.o: src/%.c $(incs)
	mkdir -p build
	$(CC) $(CCFLAGS) -c $< -o $@ -Iinc

%.o: %.c $(incs)
	$(CC) $(CCFLAGS) -c $< -o $@ -Iinc

PideShop: $(objs) server.o
	$(CC) $(CCFLAGS) $^ -o $@ 

HungryVeryMuch: $(objs) client.o
	$(CC) $(CCFLAGS) $^ -o $@ 

clean:
	rm -rf PideShop HungryVeryMuch

client: HungryVeryMuch
	clear
	valgrind -q --leak-check=full ./HungryVeryMuch 8080 10 10 10

server: PideShop
	clear
	valgrind -q --leak-check=full ./PideShop 8080 10 10 10

re: clean all

test: $(objs)
	$(CC) $(CCFLAGS) test.c build/utils.o -o test.out
	./test.out

.PHONY: all clean re client server test