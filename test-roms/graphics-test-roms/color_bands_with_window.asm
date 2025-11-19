; ------------------------------------------------
; ------------------------------------------------
; Compile with RGBASM
; cd ./test-roms/graphics-test-roms
; rgbasm -o color_bands_with_window.obj color_bands_with_window.asm
; rgblink -o color_bands_with_window.gb color_bands_with_window.obj
; rgbfix -v -p 0 color_bands_with_window.gb
; ------------------------------------------------
; ------------------------------------------------
;   color_bands_with_window.asm ---
;     Splits the screen into 4 horizontal bands:
;     White, Light Grey, Dark Grey, and Black
;
;     Also draws a blank white Window covering only the 
;     bottom right corner of the screen
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
    
    ; Initialize palette (all 4 shades)
    ld a, %11100100   ; 00=white, 01=light gray, 10=dark gray, 11=black
    ldh [$ff47], a       ; rBGP <= a


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

    cp 144              ; if row == 144, then DrawWindow
    jr z, DrawWindow
    

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
;     Write Window Tile Map Indexes   ;
;-------------------------------------;
DrawWindow:
    ; First set WX and WY to center of the screen
    ld a, 72
    ldh [$ff4a], a      ; WY = 72
    ld a, 87
    ldh [$ff4b], a      ; WX = 87 (appears at X=80)

; Then setup .rowLoop and colLoop again - it can be the full window size
; --- setup constants ---
    ld a, 0             ; tile_id is always 0 (White) for window
    ld b, 144           ; b = pixel row
    ld hl, $9C00        ; Init HL to start of Window Tile Map

.rowLoop
    ld c, 32            ; column counter (32 tiles per row)
.colLoop
    ld [hl+], a         ; write White tile index and advance HL
    dec c
    jr nz, .colLoop

    dec b
    jr nz, .rowLoop      ; [CHANGED condition]


.enableLCD
    ld a, %11110001     ; LCDC configuration:   (BG and Window ON)
    ;      ||||||||
    ;      |||||||+-- Bit 0: BG Display Enable        = 1 (show background)
    ;      ||||||+--- Bit 1: OBJ (Sprite) Enable      = 0 (no sprites)
    ;      |||||+---- Bit 2: OBJ Size                 = 0 (8x8)
    ;      ||||+----- Bit 3: BG Tile Map Select       = 0 ($9800–$9BFF)
    ;      |||+------ Bit 4: BG & Window Tile Data    = 1 ($8000–$8FFF, unsigned)
    ;      ||+------- Bit 5: Window Display Enable    = 1 (show window)
    ;      |+-------- Bit 6: Window Tile Map Select   = 1 ($9C00–$9FFF)
    ;      +--------- Bit 7: LCD Control Operation    = 1 (LCD on)

    ldh [$ff40], a      ; rLCDC <= a (upper byte is FF implicit in ldh)
    

Forever:
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
