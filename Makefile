ROM ?= prog_OR.hex

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

all: clean link
	@./myprogram

build: clean link

compile:
	@g++ -Wall -Wextra -std=c++17 -c src/Memory.cpp -o bin/Memory.out
	@g++ -Wall -Wextra -std=c++17 -c src/CPU.cpp -o bin/CPU.out
	@g++ -Wall -Wextra -std=c++17 -c src/PPU.cpp -o bin/PPU.out
	@g++ -Wall -Wextra -std=c++17 -c src/Peripherals.cpp -o bin/Peripherals.out
	@g++ -Wall -Wextra -std=c++17 -c src/Debug.cpp -o bin/Debug.out
	@g++ -Wall -Wextra -std=c++17 -c src/main.cpp -o bin/main.out

link: compile 
	@g++ bin/main.out bin/Memory.out bin/CPU.out bin/PPU.out bin/Peripherals.out bin/Debug.out -o myprogram

debug: clean link 
	./myprogram --debug

quiet: clean link
	./myprogram --quiet

test:
	python3 cpu_instrs_sanity_test.py




# Run SDL Window
app:
	g++ src/App.cpp -o bin/App.out -L /opt/homebrew/opt/sdl2/lib -I /opt/homebrew/opt/sdl2/include -lSDL2
	./bin/App.out

# Test PPU - probably need to merge with app rule
ppu: clean link
	./myprogram --test_ppu





 # PROBABLY NOT NEEDED ANYMORE...
compile_rom:
	python3 xav_compiler.py test-roms/$(ROM)

clean:
	@rm -f *.out myprogram