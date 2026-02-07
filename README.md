# XavBoy

A Game Boy (DMG) emulator written in C++. Plays Tetris!

## Features

- CPU emulation (all instructions)
- PPU with background, window, and sprite rendering
- 8x8 and 8x16 sprite modes with X/Y flip support
- OAM DMA transfers
- Joypad input
- Timer and interrupt handling
- No Audio

## Building

### Requirements

- C++17 compiler (g++ or clang++)
- SDL2

### Build Commands

```bash
# Release build (optimized)
make release

# Debug build
make debug

# Build only (no run)
make build_release
```

## Usage

```bash
./myprogram <rom_file> [options]
```

### Options

| Option | Description |
|--------|-------------|
| `--quiet` | Suppress debug output |
| `--skip-boot-rom` | Skip the boot ROM sequence |
| `--no-dbg` | Disable debug features |
| `--step` | Step-by-step execution mode |

### Examples

```bash
# Run Tetris
./myprogram tetris.gb --quiet

# Run with boot ROM skipped
./myprogram game.gb --skip-boot-rom --quiet
```

## Controls

| Game Boy | Keyboard |
|----------|----------|
| D-Pad Up | W |
| D-Pad Down | S |
| D-Pad Left | A |
| D-Pad Right | D |
| A Button | J |
| B Button | K |
| Start | Enter |
| Select | Backspace |

## Tested Games

- Tetris

## Limitations

- No MBC (Memory Bank Controller) support - only 32KB ROMs
- No audio emulation
- No Game Boy Color support

## License

MIT
