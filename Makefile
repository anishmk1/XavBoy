ROM ?= prog_OR.hex

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

# OS detection for SDL2 flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	SDL_CFLAGS = -I/opt/homebrew/opt/sdl2/include
	SDL_LDFLAGS = -L/opt/homebrew/opt/sdl2/lib -lSDL2
else
	SDL_CFLAGS = $(shell sdl2-config --cflags)
	SDL_LDFLAGS = $(shell sdl2-config --libs)
endif

all: clean link
	@./myprogram

build: clean link

compile:
	@g++ -Wall -Wextra -std=c++17 -c src/Memory.cpp -o bin/Memory.out
	@g++ -Wall -Wextra -std=c++17 -c src/CPU.cpp -o bin/CPU.out
	@g++ -Wall -Wextra -std=c++17 -c src/PPU.cpp -o bin/PPU.out
	@g++ -Wall -Wextra -std=c++17 -c src/Peripherals.cpp -o bin/Peripherals.out
	@g++ -Wall -Wextra -std=c++17 -c src/Debug.cpp -o bin/Debug.out
	@g++ -Wall -Wextra -std=c++17 $(SDL_CFLAGS) -c src/LCD.cpp -o bin/LCD.out
	@g++ -Wall -Wextra -std=c++17 -c src/main.cpp -o bin/main.out

link: compile 
	@g++ bin/main.out bin/Memory.out bin/CPU.out bin/PPU.out bin/Peripherals.out bin/Debug.out bin/LCD.out -o myprogram $(SDL_LDFLAGS)

debug: clean link 
	./myprogram --debug

quiet: clean link
	./myprogram --quiet

test:
	python3 cpu_instrs_sanity_test.py




# Test PPU - probably need to merge with app rule
ppu: clean link
	./myprogram --test_ppu





 # PROBABLY NOT NEEDED ANYMORE...
compile_rom:
	python3 xav_compiler.py test-roms/$(ROM)

clean:
	@rm -f *.out myprogram