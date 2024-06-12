CC = g++
CFLAGS = -std=c++20

all: PideShop HungryVeryMuch

build/%.o: %.cpp utils.h
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
	$(CC) $(CCFLAGS) test.cpp build/utils.o -o test.out
	./test.out

.PHONY: all clean re client server test