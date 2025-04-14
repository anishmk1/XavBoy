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
    nop
    ld a, 0x01
    ld b, 0xff
    add a, b
    ld hl, 0x1234
    ld [hl], 0xab
    add a, [hl]
    adc a, [hl]
    ld b, 0x4f
    sbc a, b
    ld b, 0xff
    and a, b
    