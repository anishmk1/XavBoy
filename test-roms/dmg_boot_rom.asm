SECTION "BootROM", ROM0[$0000]

	ld sp,$fffe		; $0000  Setup Stack

	xor a			; $0003  Zero the memory from $8000-$9FFF (VRAM)
	ld hl,$9fff		; $0004
Addr_0007:
	ld [hl-],a		; $0007
	bit 7,h			; $0008
	jr nz, Addr_0007	; $000a

	ld hl,$ff26		; $000c  Setup Audio
	ld c,$11		; $000f
	ld a,$80		; $0011
	ld [hl-],a		; $0013
	ldh [$ff00+c],a		; $0014
	inc c			; $0015
	ld a,$f3		; $0016
	ldh [$ff00+c],a		; $0018
	ld [hl-],a		; $0019
	ld a,$77		; $001a
	ld [hl],a		; $001c

	ld a,$fc		; $001d  Setup BG palette
	ldh [$ff47],a		; $001f

	ld de,$0104		; $0021  Convert and load logo data from cart into Video RAM
	ld hl,$8010		; $0024
Addr_0027:
	ld a,[de]		; $0027
	call $0095		; $0028
	call $0096		; $002b
	inc de			; $002e
	ld a,e			; $002f
	cp $34			; $0030
	jr nz, Addr_0027	; $0032

	ld de,$00d8		; $0034  Load 8 additional bytes into Video RAM
	ld b,$08		; $0037
Addr_0039:
	ld a,[de]		; $0039
	inc de			; $003a
	ld [hl+],a		; $003b
	inc hl			; $003c
	dec b			; $003d
	jr nz, Addr_0039	; $003e

	ld a,$19		; $0040  Setup background tilemap
	ld [$9910],a		; $0042
	ld hl,$992f		; $0045
Addr_0048:
	ld c,$0c		; $0048
Addr_004A:
	dec a			; $004a
	jr z, Addr_0055		; $004b
	ld [hl-],a		; $004d
	dec c			; $004e
	jr nz, Addr_004A	; $004f
	ld l,$0f		; $0051
	jr Addr_0048		; $0053

	; === Scroll logo on screen, and play logo sound===

Addr_0055:
	ld h,a			; $0055  Initialize scroll count, H=0
	ld a,$64		; $0056
	ld d,a			; $0058  set loop count, D=$64
	ldh [$ff42],a		; $0059  Set vertical scroll register
	ld a,$91		; $005b
	ldh [$ff40],a		; $005d  Turn on LCD, showing Background
	inc b			; $005f  Set B=1
Addr_0060:
	ld e,$02		; $0060
Addr_0062:
	ld c,$0c		; $0062
Addr_0064:
	ldh a,[$ff44]		; $0064  wait for screen frame
	cp $90			; $0066
	jr nz, Addr_0064	; $0068
	dec c			; $006a
	jr nz, Addr_0064	; $006b
	dec e			; $006d
	jr nz, Addr_0062	; $006e

	ld c,$13		; $0070
	inc h			; $0072  increment scroll count
	ld a,h			; $0073
	ld e,$83		; $0074
	cp $62			; $0076  $62 counts in, play sound #1
	jr z, Addr_0080		; $0078
	ld e,$c1		; $007a
	cp $64			; $007c
	jr nz, Addr_0086	; $007e  $64 counts in, play sound #2
Addr_0080:
	ld a,e			; $0080  play sound
	ldh [$ff00+c],a		; $0081
	inc c			; $0082
	ld a,$87		; $0083
	ldh [$ff00+c],a		; $0085
Addr_0086:
	ldh a,[$ff42]		; $0086
	sub b			; $0088
	ldh [$ff42],a		; $0089  scroll logo up if B=1
	dec d			; $008b
	jr nz, Addr_0060	; $008c

	dec b			; $008e  set B=0 first time
	jr nz, Addr_00E0	; $008f    ... next time, cause jump to "Nintendo Logo check"

	ld d,$20		; $0091  use scrolling loop to pause
	jr Addr_0060		; $0093

	; ==== Graphic routine ====

	ld c,a			; $0095  "Double up" all the bits of the graphics data
	ld b,$04		; $0096     and store in Video RAM
Addr_0098:
	push bc			; $0098
	rl c			; $0099
	rla			; $009b
	pop bc			; $009c
	rl c			; $009d
	rla			; $009f
	dec b			; $00a0
	jr nz, Addr_0098	; $00a1
	ld [hl+],a		; $00a3
	inc hl			; $00a4
	ld [hl+],a		; $00a5
	inc hl			; $00a6
	ret			; $00a7

Addr_00A8:
	;Nintendo Logo
	db $CE,$ED,$66,$66,$CC,$0D,$00,$0B,$03,$73,$00,$83,$00,$0C,$00,$0D
	db $00,$08,$11,$1F,$88,$89,$00,$0E,$DC,$CC,$6E,$E6,$DD,$DD,$D9,$99
	db $BB,$BB,$67,$63,$6E,$0E,$EC,$CC,$DD,$DC,$99,$9F,$BB,$B9,$33,$3E

Addr_00D8:
	;More video data
	db $3C,$42,$B9,$A5,$B9,$A5,$42,$3C

	; ===== Nintendo logo comparison routine =====

Addr_00E0:
	ld hl,$0104		; $00e0	; point HL to Nintendo logo in cart
	ld de,$00a8		; $00e3	; point DE to Nintendo logo in DMG rom

Addr_00E6:
	ld a,[de]		; $00e6
	inc de			; $00e7
	cp [hl]			; $00e8	;compare logo data in cart to DMG rom
	jr nz,$fe		; $00e9	;if not a match, lock up here
	inc hl			; $00eb
	ld a,l			; $00ec
	cp $34			; $00ed	;do this for $30 bytes
	jr nz, Addr_00E6	; $00ef

	ld b,$19		; $00f1
	ld a,b			; $00f3
Addr_00F4:
	add a,[hl]		; $00f4
	inc hl			; $00f5
	dec b			; $00f6
	jr nz, Addr_00F4	; $00f7
	add a,[hl]		; $00f9
	jr nz,$fe		; $00fa	; if $19 + bytes from $0134-$014D  don't add to $00
					;  ... lock up

	ld a,$01		; $00fc
	ldh [$ff50],a		; $00fe	;turn off DMG rom
