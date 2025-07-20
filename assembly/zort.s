    # Testbench with real register names, no pseudo instructions or directives

label_equal:
    addi t0, zero, 10        # t0 = 10
    addi t1, zero, 20        # t1 = 20
    lw   t1, 1000(t1)

    add t2, t0, t1           # t2 = t0 + t1 = 30
    sub s0, t1, t0           # s0 = t1 - t0 = 10

    addi sp, zero, 2048      # sp = 2048 (some address)
    sw t2, 0(sp)             # store 30 at memory[2048]

    lw s1, 0(sp)             # s1 = memory[2048] = 30

    beq t2, s1, label_equal  # if t2 == s1 jump to label_equal
    addi a0, zero, 0         # a0 = 0 (fail)
    j end

label_equal:
    addi a0, zero, 1         # a0 = 1 (success)

end:
    j end                    # infinite loop to end testbench
