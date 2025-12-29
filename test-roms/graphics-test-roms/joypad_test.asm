; ------------------------------------------------
; ------------------------------------------------
; Compile with RGBASM (v0.9.1)
; cd ./test-roms/graphics-test-roms
; rgbasm -o joypad_test.obj joypad_test.asm
; rgblink -o joypad_test.gb joypad_test.obj --sym symbol_file --map map_file
; rgbfix -v -p 0 joypad_test.gb
; ------------------------------------------------
; ------------------------------------------------
;   joypad_test.asm ---
;       Minimal Game Boy movement test 
; ------------------------------------------------
; ------------------------------------------------

INCLUDE "hardware.inc"

; -------------------------------
; Header
; -------------------------------
SECTION "Header", ROM0[$100]
    nop
    jp Start

    ds $150 - @, 0

; -------------------------------
; WRAM
; -------------------------------
SECTION "WRAM", WRAM0
sprite_x: db
sprite_y: db

; -------------------------------
; Tile data (8x8 solid square)
; -------------------------------
SECTION "TileData", ROM0
TileSquare:
    ; Each row = 2 bytes (bitplanes)
    ; 0xFF,0xFF = solid color 3
    REPT 8
        db $FF, $FF
    ENDR

; -------------------------------
; Main code
; -------------------------------
SECTION "Main", ROM0

Start:
    di

    ; Disable LCD
    ld a, [rLCDC]
    res 7, a
    ld [rLCDC], a

    ; ---------------------------
    ; Init VRAM
    ; ---------------------------
    ld hl, $8000
    ld bc, $2000
    xor a
.clear_vram:
    ld [hl+], a
    dec bc
    ld a, b
    or c
    jr nz, .clear_vram

    ; Copy tile into VRAM
    ld hl, TileSquare
    ld de, $8000
    ld bc, 16
.copy_tile:
    ld a, [hl+]
    ld [de], a
    inc de
    dec bc
    ld a, b
    or c
    jr nz, .copy_tile

    ; ---------------------------
    ; Init sprite position
    ; ---------------------------
    ld a, 72        ; screen Y + 16
    ld [sprite_y], a
    ld a, 80        ; screen X + 8
    ld [sprite_x], a

    ; ---------------------------
    ; Init OAM (sprite 0)
    ; ---------------------------
    ld hl, $FE00
    ld a, [sprite_y]
    ld [hl+], a     ; Y
    ld a, [sprite_x]
    ld [hl+], a     ; X
    ld a, 0
    ld [hl+], a     ; tile index
    ld a, 0
    ld [hl], a      ; attributes

    ; ---------------------------
    ; Palettes
    ; ---------------------------
    ld a, %11100100
    ld [rOBP0], a

    ; ---------------------------
    ; Enable LCD + sprites
    ; ---------------------------
    ld a, %10010010
    ld [rLCDC], a

MainLoop:
    call WaitVBlank
    call ReadInput
    call UpdateSprite
    jr MainLoop

; -------------------------------
; Wait for VBlank
; -------------------------------
WaitVBlank:
.wait_not_vblank:
    ld a, [rLY]
    cp 144
    jr nc, .wait_not_vblank   ; wait until LY < 144

.wait_vblank:
    ld a, [rLY]
    cp 144
    jr c, .wait_vblank        ; wait until LY >= 144
    ret

; -------------------------------
; Read JOYP and move sprite vars
; -------------------------------
ReadInput:
    ; Select D-Pad (P14 low)
    ld a, %00100000
    ld [rJOYP], a
    nop
    nop
    ld a, [rJOYP]
    cpl
    and %00001111

    bit 2, a        ; Up
    jr z, .no_up
    ld hl, sprite_y
    dec [hl]
.no_up:

    bit 3, a        ; Down
    jr z, .no_down
    ld hl, sprite_y
    inc [hl]
.no_down:

    bit 1, a        ; Left
    jr z, .no_left
    ld hl, sprite_x
    dec [hl]
.no_left:

    bit 0, a        ; Right
    jr z, .no_right
    ld hl, sprite_x
    inc [hl]
.no_right:

    ret

; -------------------------------
; Update OAM sprite position
; -------------------------------
UpdateSprite:
    ld hl, $FE00
    ld a, [sprite_y]
    ld [hl+], a
    ld a, [sprite_x]
    ld [hl], a
    ret