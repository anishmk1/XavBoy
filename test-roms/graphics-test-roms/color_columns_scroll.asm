; ------------------------------------------------
; ------------------------------------------------
; Compile with RGBASM
; cd ./test-roms/graphics-test-roms
; rgbasm -o color_columns_scroll.obj color_columns_scroll.asm
; rgblink -o color_columns_scroll.gb color_columns_scroll.obj
; rgbfix -v -p 0 color_columns_scroll.gb
; ------------------------------------------------
; ------------------------------------------------
;   color_columns_scroll.asm ---
;     Splits the screen into 4 vertical bands:
;     White, Light Grey, Dark Grey, and Black
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


;-------------------------------;
;    Write Tile Map Indexes     ;
;-------------------------------;
; --- setup constants ---
    ld b, 0             ; b = pixel row = 0
    ld hl, $9800        ; Init HL to start of BG Tile Map
 RowLoop:
    ld a, b
    cp 144              ; if row == 144 (full screen height), then ExitLoop
    jr z, ExitLoop

    ld c, 0             ; column counter (32 tiles per row)

ColLoop:
    ; --- decide tile index based on column ---
    ld a, c
    cp 8                ; check col < 8?
    jr c, ColTile0
    cp 16               ; check col < 16 ?
    jr c, ColTile1
    cp 24               ; check col < 24?
    jr c, ColTile2
    jr ColTile3         ; else (col >= 24)

ColTile0:
    ld a, 0             ; Tile0 is @ 0x8000 - so write index = 0
    jr ColTileChosen
ColTile1:
    ld a, 1             ; Tile1 is @ 0x8010 - so write index = 1
    jr ColTileChosen
ColTile2:
    ld a, 2             ; Tile2 is @ 0x8020 - so write index = 2
    jr ColTileChosen
ColTile3:
    ld a, 3             ; Tile3 is @ 0x8030 - so write index = 3

; a contains the tile index value for this column (selected above)
ColTileChosen:
    ld [hl+], a         ; write tile index and advance HL
    inc c
    ld a, c
    cp 32               ; check if we've done all 32 columns
    jr nz, ColLoop

    inc b               ; move to next row
    jr RowLoop

ExitLoop:
    ; Enable LCD
    ld a, %10010001     ; LCD on; BG Tile Data Mode = unsigned; BG on
    ldh [$ff40], a      ; rLCDC <= a (upper byte is FF implicit in ldh)


    ;;;;;;;; BREAKPOINT 2 ;;;;;;;;;;;;;
    ld hl, $FFFE   ; HL = target address
    ld e, $AC
    ld [hl], e      ; keep an eye on the value of b (ROW). Make sure it goes to 32
    ld [hl], 0
    ;;;;;;;; BREAKPOINT 2 ;;;;;;;;;;;;;


Forever:
    ; Wait for VBlank before updating SCX
    call WaitVBlank

    ; Now safe to update scroll register
    ld a, [$ff43]       ; Load SCX (horizontal scroll)
    inc a
    ld [$ff43], a       ; Store back to SCX
    jr Forever

; Subroutine to wait for VBlank period
WaitVBlank:
    ld a, [$ff44]       ; Read LY register (current scanline)
    cp 144              ; VBlank starts at scanline 144
    jr c, WaitVBlank    ; If LY < 144, keep waiting
    ret                 ; VBlank detected, safe to return




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