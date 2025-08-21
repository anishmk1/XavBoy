;; simple_infinite_loop.asm

;.section "Entry", ROM0[$100]
    ;nop             ; required first instruction after reset
    ;jp Start        ; jump to our code

;.section "Code", ROM0
;Start:
;Forever:
    ;jr Forever      ; jump relative back to itself (infinite loop)

SECTION "Header", ROM0[$0100]
    nop
    jp Start

SECTION "Code", ROM0[$0150]
Start:
Forever:
    jr Forever