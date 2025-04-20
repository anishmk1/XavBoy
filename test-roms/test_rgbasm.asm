SECTION "Main", ROM0

Main:
    inc a
    inc a
    inc a
    ld hl, 0x1234
    ld [hl], 0xab
    add a, [hl]
    adc a, [hl]
    ld b, 0x4f
    sbc a, b
    ld b, 0xff
    and a, b
    ld [hl], 0xf4
    dec [hl] 
    dec [hl]
    dec [hl]
    dec [hl]
    dec [hl]
    dec [hl]
    jr 10

; MyJump:
;     inc [hl]
;     inc [hl]
;     inc [hl]
;     inc [hl]
;     inc [hl]
    