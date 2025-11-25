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

# User options
SKIP_BOOT_ROM ?= 0

ifeq ($(SKIP_BOOT_ROM),1)
    EXTRA_RUN_ARGS += --skip-boot-rom
endif

# CXX = g++
CXXFLAGS = -DDEBUG_MODE -Wall -Wextra -std=c++17 $(SDL_CFLAGS)
CXXRELFLAGS = -DREL_MODE -O3 -Wall -Wextra -std=c++17 -DNDEBUG $(SDL_CFLAGS)
# CXXDBGFLAGS = -g -O2 -Wall -Wextra -std=c++17 -fsanitize=address,undefined $(SDL_CFLAGS)
# LDDBGFLAGS = -g -O2 -fsanitize=address,undefined -rdynamic
CXXDBGFLAGS = -DDEBUG_MODE -g -O0 -Wall -Wextra -std=c++17 -fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-inline $(SDL_CFLAGS)
LDDBGFLAGS = -g -O0 -fsanitize=address,undefined -fno-omit-frame-pointer -fno-optimize-sibling-calls -fno-inline -rdynamic

all: clean compile link
	@./myprogram

build: clean compile link

build_release: clean compile_release link

release: build_release
	@./myprogram --quiet

step: clean compile link
	@./myprogram --step $(EXTRA_RUN_ARGS)

debug: clean gdb
	./myprogram --quiet $(EXTRA_RUN_ARGS)

quiet: clean compile link
	@./myprogram --quiet $(EXTRA_RUN_ARGS)

compile:
	@$(CXX) $(CXXFLAGS) -c src/Memory.cpp -o bin/Memory.o
	@$(CXX) $(CXXFLAGS) -c src/CPU.cpp -o bin/CPU.o
	@$(CXX) $(CXXFLAGS) -c src/PPU.cpp -o bin/PPU.o
	@$(CXX) $(CXXFLAGS) -c src/Peripherals.cpp -o bin/Peripherals.o
	@$(CXX) $(CXXFLAGS) -c src/Debug.cpp -o bin/Debug.o
	@$(CXX) $(CXXFLAGS) -c src/LCD.cpp -o bin/LCD.o
	@$(CXX) $(CXXFLAGS) -c src/main.cpp -o bin/main.o

compile_release:
	@$(CXX) $(CXXRELFLAGS) -c src/Memory.cpp -o bin/Memory.o
	@$(CXX) $(CXXRELFLAGS) -c src/CPU.cpp -o bin/CPU.o
	@$(CXX) $(CXXRELFLAGS) -c src/PPU.cpp -o bin/PPU.o
	@$(CXX) $(CXXRELFLAGS) -c src/Peripherals.cpp -o bin/Peripherals.o
	@$(CXX) $(CXXRELFLAGS) -c src/Debug.cpp -o bin/Debug.o
	@$(CXX) $(CXXRELFLAGS) -c src/LCD.cpp -o bin/LCD.o
	@$(CXX) $(CXXRELFLAGS) -c src/main.cpp -o bin/main.o

gdb:
	@$(CXX) $(CXXDBGFLAGS) -c src/Memory.cpp -o bin/Memory.o
	@$(CXX) $(CXXDBGFLAGS) -c src/CPU.cpp -o bin/CPU.o
	@$(CXX) $(CXXDBGFLAGS) -c src/PPU.cpp -o bin/PPU.o
	@$(CXX) $(CXXDBGFLAGS) -c src/Peripherals.cpp -o bin/Peripherals.o
	@$(CXX) $(CXXDBGFLAGS) -c src/Debug.cpp -o bin/Debug.o
	@$(CXX) $(CXXDBGFLAGS) -c src/LCD.cpp -o bin/LCD.o
	@$(CXX) $(CXXDBGFLAGS) -c src/main.cpp -o bin/main.o
	@$(CXX) $(LDDBGFLAGS) bin/main.o bin/Memory.o bin/CPU.o bin/PPU.o bin/Peripherals.o bin/Debug.o bin/LCD.o -o myprogram $(SDL_LDFLAGS)

link:
	@$(CXX) bin/main.o bin/Memory.o bin/CPU.o bin/PPU.o bin/Peripherals.o bin/Debug.o bin/LCD.o -o myprogram $(SDL_LDFLAGS)

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