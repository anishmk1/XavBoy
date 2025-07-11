ROM ?= prog_OR.hex

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
SRC = main.cpp

# all: clean main.out 
# 	@./main.out
all: clean link
	@./myprogram

compile:
	@g++ -Wall -Wextra -std=c++17 -c Memory.cpp -o Memory.out
	@g++ -Wall -Wextra -std=c++17 -c CPU.cpp -o CPU.out
	@g++ -Wall -Wextra -std=c++17 -c Peripherals.cpp -o Peripherals.out
	@g++ -Wall -Wextra -std=c++17 -c Debug.cpp -o Debug.out
	@g++ -Wall -Wextra -std=c++17 -c main.cpp -o main.out


link: compile 
	@g++ main.out Memory.out CPU.out Peripherals.out Debug.out -o myprogram

compile_rom:
	python3 xav_compiler.py test-roms/$(ROM)

debug: clean link 
	./myprogram --debug

quiet: clean link
	./myprogram --quiet

clean:
	@rm -f *.out myprogram
