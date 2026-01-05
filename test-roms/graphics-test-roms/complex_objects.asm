; ------------------------------------------------
; ------------------------------------------------
; Compile with RGBASM
; cd ./test-roms/graphics-test-roms
; rgbasm -o complex_objects.obj complex_objects.asm
; rgblink -o complex_objects.gb complex_objects.obj
; rgbfix -v -p 0 complex_objects.gb
; ------------------------------------------------
; ------------------------------------------------
;   complex_objects.asm ---
;       Demonstrates 8x16 sprite mode (LCDC bit 2 = 1)
;       Draws a color band background and 8x16 objects on screen
;       Each object is 8 pixels wide × 16 pixels tall, using 2 tiles stacked vertically

; ------------------------------------------------
; ------------------------------------------------


SECTION "Header", ROM0[$100]
    nop
    jp Start

; --- Header filler so rgbfix can patch correctly ---
    ds $150 - @, 0


SECTION "Main", ROM0
Start:
    ; Disable LCD at start
    ldh a, [0xff40]     ; load current LCDC value
    and %01111111       ; clear bit 7 (LCD enable)
    ldh [0xff40], a     ; write back to LCDC

    ;-------------------------------;
    ;    Write Tile Data to VRAM    ;
    ;-------------------------------;

    ; Initialize Background Palette (all 4 shades)
    ld a, %11100100   ; 00=white, 01=light gray, 10=dark gray, 11=black
    ldh [$ff47], a    ; rBGP <= a (Background Palette)

    ; Initialize Object Palette 0 (for sprites)
    ; Objects/sprites use separate palette registers (OBP0 and OBP1)
    ; The OAM attribute byte (byte 3) bit 4 selects which palette:
    ;   Bit 4 = 0 → Use OBP0 ($FF48)
    ;   Bit 4 = 1 → Use OBP1 ($FF49)
    ld a, %11100100   ; Same palette mapping for objects
    ldh [$ff48], a    ; rOBP0 <= a (Object Palette 0)


    ; First Define 4 tiles - all white to all black 8x8 grid of pixels
    ; each tile takes 16 bytes

    ; TILE 0 ($8000) -- ALL WHITE
    ld hl, Tile0White   ; HL = source in ROM
    ld de, $8000        ; DE = destination in VRAM
    ld b, 16            ; B = number of bytes to copy
    call CopyToVRAM

    ; TILE 1 ($8010) -- ALL LIGHT GREY
    ld hl, Tile1LightGrey   ; HL = source in ROM
    ld de, $8010            ; DE = destination in VRAM
    ld b, 16                ; B = number of bytes to copy
    call CopyToVRAM

    ; TILE 2 ($8020) -- ALL DARK GREY
    ld hl, Tile2DarkGrey    ; HL = source in ROM
    ld de, $8020            ; DE = destination in VRAM
    ld b, 16                ; B = number of bytes to copy
    call CopyToVRAM

    ; TILE 3 ($8030) -- ALL BLACK
    ld hl, Tile3Black       ; HL = source in ROM
    ld de, $8030            ; DE = destination in VRAM
    ld b, 16                ; B = number of bytes to copy
    call CopyToVRAM

    ; ----------------------------------------
    ; TILE 4 ($8040) -- TOP HALF OF 8x16 SPRITE
    ; ----------------------------------------
    ; In 8x16 mode, each sprite uses TWO consecutive tiles stacked vertically.
    ; This is the TOP tile (upper 8 pixels of the 16-pixel tall sprite)
    ; Pattern: Checkerboard pattern for top half
    ld hl, TileObjectTop    ; HL = source in ROM
    ld de, $8040            ; DE = destination in VRAM
    ld b, 16                ; B = number of bytes to copy
    call CopyToVRAM

    ; ----------------------------------------
    ; TILE 5 ($8050) -- BOTTOM HALF OF 8x16 SPRITE
    ; ----------------------------------------
    ; This is the BOTTOM tile (lower 8 pixels of the 16-pixel tall sprite)
    ; Pattern: Inverted checkerboard pattern for bottom half
    ;
    ; IMPORTANT: In 8x16 mode, the tile index's bit 0 is IGNORED:
    ;   - Top tile    = tile_index & 0xFE  (force even)
    ;   - Bottom tile = (tile_index & 0xFE) + 1
    ;
    ; Examples:
    ;   - Tile index 4 → uses tiles 4 (top) and 5 (bottom)
    ;   - Tile index 5 → ALSO uses tiles 4 (top) and 5 (bottom)
    ;   - Tile index 6 → uses tiles 6 (top) and 7 (bottom)
    ld hl, TileObjectBottom ; HL = source in ROM
    ld de, $8050            ; DE = destination in VRAM
    ld b, 16                ; B = number of bytes to copy
    call CopyToVRAM



;------------------------------------------;
;     Write Background Tile Map Indexes    ;
;------------------------------------------;
DrawBackground:
; --- setup constants ---
    ld b, 0             ; b = tile row = 0
    ld hl, $9800        ; Init HL to start of BG Tile Map
.rowLoop
    ld a, b
    sla a               ; [NEW] multiply tile row by 8 -> convert to pixel y
    sla a               ; [NEW]
    sla a               ; [NEW]
    ; now A = pixel_y (0, 8, 16, ...)

    cp 144              ; if row == 144, then DrawObjects
    jr z, DrawObjects


    ; --- decide tile index based on row ---
    cp 36               ; check row < 36?
    jr c, .rowTile0
    cp 72               ; check row < 72 ?
    jr c, .rowTile1
    cp 108              ; check row < 108?
    jr c, .rowTile2
    jr .rowTile3 ; else (row >= 108)


.rowTile0:
    ld a, 0             ; Tile0 is @ 0x8000 - so write index = 0
    jr .rowTileChosen
.rowTile1:
    ld a, 1             ; Tile1 is @ 0x8010 - so write index = 1
    jr .rowTileChosen
.rowTile2:
    ld a, 2             ; Tile2 is @ 0x8020 - so write index = 2
    jr .rowTileChosen
.rowTile3:
    ld a, 3             ; Tile3 is @ 0x8030 - so write index = 3

; a contains the tile index value for this row (selected above)
.rowTileChosen
    ld c, 32            ; column counter (32 tiles per row)

.colLoop
    ld [hl+], a         ; write tile index and advance HL
    dec c
    jr nz, .colLoop

    inc b               ; move to next tile row
    cp 32               ; [NEW] stop after 32 tile rows
    jr nz, .rowLoop      ; [CHANGED condition]

;-------------------------------------;
;           Draw Object Tiles         ;
;-------------------------------------;
; Construct objects using the object data format
; Write to OAM (Object Attribute Memory)
;
; Each sprite in 8x16 mode is 8 pixels wide × 16 pixels tall
; OAM format (4 bytes per sprite):
;   Byte 0: Y position (screen_y + 16)
;   Byte 1: X position (screen_x + 8)
;   Byte 2: Tile index (bit 0 ignored in 8x16 mode)
;   Byte 3: Attributes/Flags

DrawObjects:
    ld hl, $FE00        ; Init to start of OAM ($FE00-FE9F)

    ; ========================================
    ; SPRITE 1: Top-left corner (screen position 0, 0)
    ; ========================================
    ld a, 16            ; Byte 0 = Y position = 16 → ScreenY = 0
    ld [hl+], a
    ld a, 8             ; Byte 1 = X position = 8 → ScreenX = 0
    ld [hl+], a
    ld a, 4             ; Byte 2 = Tile Index = 4 (uses tiles 4 & 5)
    ld [hl+], a
    ld a, %10000000     ; Byte 3 - Attributes/Flags
    ld [hl+], a

    ; ========================================
    ; SPRITE 2: Bottom-left corner (screen position 0, 100)
    ; ========================================
    ; Note: 8x16 sprite, so Y position needs to account for 16-pixel height
    ld a, 116           ; Byte 0 = Y position = 116 → ScreenY = 100
    ld [hl+], a
    ld a, 8             ; Byte 1 = X position = 8 → ScreenX = 0
    ld [hl+], a
    ld a, 4             ; Byte 2 = Tile Index = 4 (uses tiles 4 & 5)
    ld [hl+], a
    ld a, %10000000     ; Byte 3 - Attributes/Flags
    ld [hl+], a

    ; ========================================
    ; SPRITE 3: Top-right corner (screen position 100, 0)
    ; ========================================
    ld a, 16            ; Byte 0 = Y position = 16 → ScreenY = 0
    ld [hl+], a
    ld a, 108           ; Byte 1 = X position = 108 → ScreenX = 100
    ld [hl+], a
    ld a, 4             ; Byte 2 = Tile Index = 4 (uses tiles 4 & 5)
    ld [hl+], a
    ld a, %10000000     ; Byte 3 - Attributes/Flags
    ld [hl+], a

    ; ========================================
    ; SPRITE 4: Bottom-right corner (screen position 100, 100)
    ; ========================================
    ld a, 116           ; Byte 0 = Y position = 116 → ScreenY = 100
    ld [hl+], a
    ld a, 108           ; Byte 1 = X position = 108 → ScreenX = 100
    ld [hl+], a
    ld a, 4             ; Byte 2 = Tile Index = 4 (uses tiles 4 & 5)
    ld [hl+], a
    ld a, %10000000     ; Byte 3 - Attributes/Flags
    ld [hl+], a




.enableLCD

    ; ========================================
    ; LCDC CONFIGURATION FOR 8x16 SPRITE MODE
    ; ========================================
    ; KEY DIFFERENCE: Bit 2 = 1 enables 8x16 sprite mode
    ; (compare to simple_objects.asm which uses %11010011 for 8x8 mode)

    ld a, %11010111     ; LCDC configuration:   (BG, OBJ, and 8x16 mode ON)
    ;      ||||||||
    ;      |||||||+-- Bit 0: BG Display Enable        = 1 (show background)
    ;      ||||||+--- Bit 1: OBJ (Sprite) Enable      = 1 (show sprites)
    ;      |||||+---- Bit 2: OBJ Size                 = 1 (8x16) ← 8x16 MODE!
    ;      ||||+----- Bit 3: BG Tile Map Select       = 0 ($9800–$9BFF)
    ;      |||+------ Bit 4: BG & Window Tile Data    = 1 ($8000–$8FFF, unsigned)
    ;      ||+------- Bit 5: Window Display Enable    = 0 (hide window)
    ;      |+-------- Bit 6: Window Tile Map Select   = 1 ($9C00–$9FFF)
    ;      +--------- Bit 7: LCD Control Operation    = 1 (LCD on)

    ldh [$ff40], a      ; rLCDC <= a (upper byte is FF implicit in ldh)


Forever:
    halt                ; Wait for interrupt (VBlank)
    nop                 ; Required after halt
    jr Forever


; Subroutine to Copy Predefined tile data into VRAM
CopyToVRAM:
.loop:
    ld a, [hl+]         ; load each byte from ROM
    ld [de], a          ; store to VRAM
    inc de              ; point to next VRAM addr
    dec b               ; dec number of bytes
    jr nz, .loop        ; if number of bytes != 0, keep looping
    ret




; --- Tile Data in ROM ---
SECTION "TileData", ROM0
Tile0White:
    ds 16, 0   ; 16 bytes of 0x00 → all pixels index 0 (white)

; Encode Tile data such that all pixels are 01
; Bytes 0/2/4/6/8/10/12/14 should be FF
; Bytes 1/3/5/7/9/11/13/15 should be 00
Tile1LightGrey:
    ; 8 rows × 2 bytes per row
    db 0xFF,0x00
    db 0xFF,0x00
    db 0xFF,0x00
    db 0xFF,0x00
    db 0xFF,0x00
    db 0xFF,0x00
    db 0xFF,0x00
    db 0xFF,0x00

; Encode Tile data such that all pixels are 10
; Bytes 0/2/4/6/8/10/12/14 should be 00
; Bytes 1/3/5/7/9/11/13/15 should be FF
Tile2DarkGrey:
    db 0x00,0xFF
    db 0x00,0xFF
    db 0x00,0xFF
    db 0x00,0xFF
    db 0x00,0xFF
    db 0x00,0xFF
    db 0x00,0xFF
    db 0x00,0xFF

Tile3Black:
    ds 16, 0xFF      ; all pixels = 3 (black)

; ========================================
; TOP HALF OF 8x16 SPRITE (Tile 4)
; ========================================
; Checkerboard pattern with black borders
; Visual pattern makes it clear this is the TOP tile
TileObjectTop:
    db 0xFF, 0xFF    ; Row 0: Black border (top)
    db 0xAA, 0x55    ; Row 1: Checkerboard ■□■□■□■□
    db 0x55, 0xAA    ; Row 2: Checkerboard □■□■□■□■
    db 0xAA, 0x55    ; Row 3: Checkerboard ■□■□■□■□
    db 0x55, 0xAA    ; Row 4: Checkerboard □■□■□■□■
    db 0xAA, 0x55    ; Row 5: Checkerboard ■□■□■□■□
    db 0x55, 0xAA    ; Row 6: Checkerboard □■□■□■□■
    db 0xFF, 0x00    ; Row 7: Light gray separator

; ========================================
; BOTTOM HALF OF 8x16 SPRITE (Tile 5)
; ========================================
; Inverted checkerboard pattern with black borders
; Visual pattern makes it clear this is the BOTTOM tile
TileObjectBottom:
    db 0xFF, 0x00    ; Row 0: Light gray separator (continuation)
    db 0x55, 0xAA    ; Row 1: Inverted checkerboard □■□■□■□■
    db 0xAA, 0x55    ; Row 2: Inverted checkerboard ■□■□■□■□
    db 0x55, 0xAA    ; Row 3: Inverted checkerboard □■□■□■□■
    db 0xAA, 0x55    ; Row 4: Inverted checkerboard ■□■□■□■□
    db 0x55, 0xAA    ; Row 5: Inverted checkerboard □■□■□■□■
    db 0xAA, 0x55    ; Row 6: Inverted checkerboard ■□■□■□■□
    db 0xFF, 0xFF    ; Row 7: Black border (bottom)
