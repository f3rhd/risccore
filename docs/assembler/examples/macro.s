.macro incr reg
    addi reg, reg, 1
.endm

start:
    addi t0, zero, 5    # t0 = 5
    incr t0             # t0 = t0 + 1
    incr t0             # t0 = t0 + 1 again
    jal zero, hang      # infinite loop

hang:
    jal zero, hang
