ROM ?= prog_OR.hex

# OS detection for SDL2 flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)	## Darwin == Apple CoreOS (detects MacOS/iPadOS/iOS)
# $(info Detected MacOS)
	SDL_CFLAGS = -I/opt/homebrew/opt/sdl2/include
	SDL_LDFLAGS = -L/opt/homebrew/opt/sdl2/lib -lSDL2
	CXX = clang++
else
# $(info Detected non-MacOS system)
	SDL_CFLAGS = $(shell sdl2-config --cflags)
	SDL_LDFLAGS = $(shell sdl2-config --libs)
	CXX = g++
endif

# CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 $(SDL_CFLAGS)
CXXDBGFLAGS = -g -O0 -Wall -Wextra -std=c++17 -fsanitize=address $(SDL_CFLAGS)
LDDBGFLAGS = -g -O0 -fsanitize=address -rdynamic

all: clean link
	@./myprogram

build: clean link

compile:
	@$(CXX) $(CXXFLAGS) -c src/Memory.cpp -o bin/Memory.o
	@$(CXX) $(CXXFLAGS) -c src/CPU.cpp -o bin/CPU.o
	@$(CXX) $(CXXFLAGS) -c src/PPU.cpp -o bin/PPU.o
	@$(CXX) $(CXXFLAGS) -c src/Peripherals.cpp -o bin/Peripherals.o
	@$(CXX) $(CXXFLAGS) -c src/Debug.cpp -o bin/Debug.o
	@$(CXX) $(CXXFLAGS) -c src/LCD.cpp -o bin/LCD.o
	@$(CXX) $(CXXFLAGS) -c src/main.cpp -o bin/main.o

gdb:
	@$(CXX) $(CXXDBGFLAGS) -c src/Memory.cpp -o bin/Memory.o
	@$(CXX) $(CXXDBGFLAGS) -c src/CPU.cpp -o bin/CPU.o
	@$(CXX) $(CXXDBGFLAGS) -c src/PPU.cpp -o bin/PPU.o
	@$(CXX) $(CXXDBGFLAGS) -c src/Peripherals.cpp -o bin/Peripherals.o
	@$(CXX) $(CXXDBGFLAGS) -c src/Debug.cpp -o bin/Debug.o
	@$(CXX) $(CXXDBGFLAGS) -c src/LCD.cpp -o bin/LCD.o
	@$(CXX) $(CXXDBGFLAGS) -c src/main.cpp -o bin/main.o
	@$(CXX) $(LDDBGFLAGS) bin/main.o bin/Memory.o bin/CPU.o bin/PPU.o bin/Peripherals.o bin/Debug.o bin/LCD.o -o myprogram $(SDL_LDFLAGS)


link: compile 
	@$(CXX) bin/main.o bin/Memory.o bin/CPU.o bin/PPU.o bin/Peripherals.o bin/Debug.o bin/LCD.o -o myprogram $(SDL_LDFLAGS)

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
	@rm -f *.o myprogram