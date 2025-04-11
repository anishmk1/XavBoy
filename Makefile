ROM ?= prog_OR.hex

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
SRC = main.cpp

all: clean main.out 
	./main.out

compile_rom:
	python3 xav_compiler.py test-roms/$(ROM)

main.out: $(SRC)
	g++ -Wall -Wextra -std=c++17 -o main.out $(SRC)

run: main.out
	./main.out

debug: main.out
	./main.out --debug

clean:
	rm -f main.out
