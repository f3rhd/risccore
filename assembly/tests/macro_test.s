.macro inc register, value
    addi register, register, value
.endm
.macro dec register, value
    addi register, register, -value
.endm
main:
    inc ra,5
    dec ra,3 
    nop
