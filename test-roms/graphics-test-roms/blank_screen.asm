; Minimal GB ROM: turns on LCD and shows a solid black screen

SECTION "Header", ROM0[$0100]
    nop
    jp Start

SECTION "Code", ROM0[$0150]
Start:
    ; Set background palette (BGP, 0xFF47)
    ; Bit pairs (from LSB): Color 0,1,2,3 → hardware shades
    ; $FF = 11 11 11 11b = all shades mapped to black
    ; $55 = 01 01 01 01b = all shades mapped to light grey
    ld   a, $55
    ld   [$FF47], a

    ; Turn on LCD (LCDC, 0xFF40)
    ; Bit 7 = LCD enable, Bit 0 = BG enable
    ld   a, $91       ; 10010001b → LCD on, BG on, tilemap 0
    ld   [$FF40], a

Forever:
    jr   Forever