all: PideShop HungryVeryMuch

build/%.o: %.cpp utils.h
	mkdir -p build
	g++ -c $< -o $@

PideShop: build/server.o build/utils.o
	g++ $^ -o $@ 

HungryVeryMuch: build/client.o build/utils.o
	g++ $^ -o $@ 

clean:
	rm -rf build PideShop HungryVeryMuch

client: HungryVeryMuch
	clear
	valgrind -q ./HungryVeryMuch 8080 10 10 10

server: PideShop
	clear
	valgrind -q ./PideShop 8080 10 10 10

re: clean all

.PHONY: all clean re client server