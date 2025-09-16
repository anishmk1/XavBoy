; ------------------------------------------------
; ------------------------------------------------
; Compile with RGBASM
; cd ./test-roms/graphics-test-roms
; rgbasm -o color_bands_scroll.obj color_bands_scroll.asm
; rgblink -o color_bands_scroll.gb color_bands_scroll.obj
; rgbfix -v -p 0 color_bands_scroll.gb
; ------------------------------------------------
; ------------------------------------------------
;   color_bands_scroll.asm ---
;     Splits the screen into 4 horizontal bands:
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
    cp 144              ; if row == 144, then ExitLoop
    jr z, ExitLoop
    

    ; --- decide tile index based on row ---
    cp 32               ; check row < 32?
    jr c, RowTile0
    cp 64               ; check row < 64 ?
    jr c, RowTile1
    cp 96               ; check row < 96?
    jr c, RowTile2
    cp 128              ; check row < 128?
    jr c, RowTile3
    cp 160              ; check row < 160?
    jr c, RowTile2
    cp 192              ; check row < 192?
    jr c, RowTile1
    cp 224              ; check row < 224?
    jr c, RowTile0
    jr RowTile1 ; else (224 >= row < 256)


RowTile0:
    ld a, 0             ; Tile0 is @ 0x8000 - so write index = 0 
    jr RowTileChosen
RowTile1:
    ld a, 1             ; Tile1 is @ 0x8010 - so write index = 1
    jr RowTileChosen
RowTile2:
    ld a, 2             ; Tile2 is @ 0x8020 - so write index = 2
    jr RowTileChosen
RowTile3:
    ld a, 3             ; Tile3 is @ 0x8030 - so write index = 3

; a contains the tile index value for this row (selected above)
RowTileChosen:
    ld c, 32            ; column counter (32 tiles per row)

ColLoop:
    ld [hl+], a         ; write tile index and advance HL
    dec c
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
    ; increment SCY - continuously scroll downwards
    ; How to wait for each VBLANK period before incrementing SCY?

PollVBLANK1:
    ld a, [$fffa]
    cp 1
    jr z, incrSCY
    jr PollVBLANK1

PollVBLANK0:
    ld a, [$fffa]
    cp 0
    jr z, PollVBLANK1
    jr PollVBLANK0


incrSCY:
    ld a, [$ff42]
    inc a
    ld [$ff42], a
    jr PollVBLANK0




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
