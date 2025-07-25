.macro inc register,val
    addi register,zero,val
    nop
.endm
.macro dec register,val
    addi register,register,-val
.endm
main:
    inc ra,5
    dec ra,2
    nop
