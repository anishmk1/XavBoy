ROM ?= prog_OR.hex

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

all: clean link
	@./myprogram

compile:
	@g++ -Wall -Wextra -std=c++17 -c src/Memory.cpp -o bin/Memory.out
	@g++ -Wall -Wextra -std=c++17 -c src/CPU.cpp -o bin/CPU.out
	@g++ -Wall -Wextra -std=c++17 -c src/Peripherals.cpp -o bin/Peripherals.out
	@g++ -Wall -Wextra -std=c++17 -c src/Debug.cpp -o bin/Debug.out
	@g++ -Wall -Wextra -std=c++17 -c src/main.cpp -o bin/main.out


link: compile 
	@g++ bin/main.out bin/Memory.out bin/CPU.out bin/Peripherals.out bin/Debug.out -o myprogram

compile_rom:
	python3 xav_compiler.py test-roms/$(ROM)

debug: clean link 
	./myprogram --debug

quiet: clean link
	./myprogram --quiet

clean:
	@rm -f *.out myprogram
