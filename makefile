CC = gcc
CCFLAGS = -O0 -Wall -Wextra -Werror -pedantic -g -pthread
CCAFTER = -lm -Iinc -lpthread
MEMCHECK = valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -q

incs = $(wildcard src/*.h)
objs = $(patsubst src/%.c, build/%.o, $(wildcard src/*.c))

all: PideShop HungryVeryMuch

build/%.o: src/%.c $(incs)
	mkdir -p build
	$(CC) $(CCFLAGS) -c $< -o $@ $(CCAFTER) 

%.o: %.c $(incs)
	$(CC) $(CCFLAGS) -c $< -o $@ $(CCAFTER) 

PideShop: $(objs) server.o
	$(CC) $(CCFLAGS) $^ -o $@ $(CCAFTER) 

HungryVeryMuch: $(objs) client.o
	$(CC) $(CCFLAGS) $^ -o $@ $(CCAFTER) 

clean:
	rm -rf PideShop HungryVeryMuch build *.o 

fclean: clean
	rm -rf *.out *.txt

client: HungryVeryMuch
	clear
	valgrind -q --leak-check=full ./HungryVeryMuch 8080 10 10 10

server: PideShop
	clear
	valgrind -q --leak-check=full ./PideShop 8080 10 10 10

re: clean all test.out

test.out: $(objs) test.o
	$(CC) $(CCFLAGS) $^ -o test.out $(CCAFTER) 

test: test.out
	$(MEMCHECK) ./test.out

.PHONY: all clean re client server test fclean