.macro inc register, value
    addi register, register, value
.endm
main:
    inc ra,5,6
    nop
