CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
SRC = main.cpp

all: clean compile_rom main.out 
	./main.out

compile_rom:
	python3 xav_compiler.py test-roms/prog_OR.hex

main.out: $(SRC)
	$(CXX) $(CXXFLAGS) -o main.out $(SRC)

run: main.out
	./main.out

debug: main.out
	./main.out --debug

clean:
	rm -f main.out
