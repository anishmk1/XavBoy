; ===============================
; Game Boy hardware registers
; RGBDS v0.6+ compatible
; ===============================

; -------------------------------
; I/O Registers ($FF00–$FF7F)
; -------------------------------

DEF rJOYP  = $FF00

DEF rSB    = $FF01
DEF rSC    = $FF02

DEF rDIV   = $FF04
DEF rTIMA  = $FF05
DEF rTMA   = $FF06
DEF rTAC   = $FF07

DEF rIF    = $FF0F
DEF rIE    = $FFFF

; Sound
DEF rNR10  = $FF10
DEF rNR11  = $FF11
DEF rNR12  = $FF12
DEF rNR13  = $FF13
DEF rNR14  = $FF14

DEF rNR21  = $FF16
DEF rNR22  = $FF17
DEF rNR23  = $FF18
DEF rNR24  = $FF19

DEF rNR30  = $FF1A
DEF rNR31  = $FF1B
DEF rNR32  = $FF1C
DEF rNR33  = $FF1D
DEF rNR34  = $FF1E

DEF rNR41  = $FF20
DEF rNR42  = $FF21
DEF rNR43  = $FF22
DEF rNR44  = $FF23

DEF rNR50  = $FF24
DEF rNR51  = $FF25
DEF rNR52  = $FF26

; LCD / PPU
DEF rLCDC  = $FF40
DEF rSTAT  = $FF41
DEF rSCY   = $FF42
DEF rSCX   = $FF43
DEF rLY    = $FF44
DEF rLYC   = $FF45
DEF rDMA   = $FF46
DEF rBGP   = $FF47
DEF rOBP0  = $FF48
DEF rOBP1  = $FF49
DEF rWY    = $FF4A
DEF rWX    = $FF4B

; -------------------------------
; Memory base addresses
; -------------------------------

DEF VRAM_BASE = $8000
DEF OAM_BASE  = $FE00
DEF HRAM_BASE = $FF80
