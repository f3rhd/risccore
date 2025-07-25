.macro inc register
    addi register,zero,1
    nop
.endm
#   inc - arguments
#   |
#   definition
main:
    inc ra
    nop
