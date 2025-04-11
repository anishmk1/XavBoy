SECTION "Main", ROM0

Main:
    inc a
    inc a
    inc a
    inc a
    ld bc, 0xff01   ; bc <= stores addr of serial output data
    ld [bc], a        ; store the value of a into 0xff01
    ld a, 0x81
    ld de, 0xff02   ; hl <= store addr of serial output valid
    ld [de], a
