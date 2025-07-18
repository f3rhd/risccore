    # Initialize n = 5
    addi a0, zero, 5       # a0 = 5

    jal ra, fact           # call fact(5)

    # result in a0
    jal zero, done         # halt (infinite loop)

# -------- factorial(a0) --------
fact:
    addi t0, zero, 1       # t0 = 1
    beq a0, t0, base_case  # if a0 == 1 -> return 1

    # Save ra and a0 on stack
    addi 10, sp, 0
    sw ra, 4(sp)
    sw a0, 0(sp)

    addi a0, a0, -1        # a0 = a0 - 1
    jal ra, fact           # recursive call: fact(n - 1)

    # After return: a0 = fact(n - 1)
    lw t1, 0(sp)           # t1 = original n
    lw ra, 4(sp)
    addi sp, sp, 8         # pop

    # Call multiply(t1, a0)
    add a1, zero,a0              # move fact(n-1) to a1
    add a0, t1,a0              # move original n to a0
    jal ra, multiply       # a0 = multiply(n, fact(n-1))

    jalr zero, 0(ra)       # return

base_case:
    addi a0, zero, 1
    jalr zero, 0(ra)

# -------- multiply(a0 * a1) --------
multiply:
    # a0 = x, a1 = y
    addi t0, zero, 0       # t0 = result = 0
    addi t1, zero, 0       # t1 = counter

multiply_loop:
    beq t1, a1, multiply_done
    add t0, t0, a0         # result += a0
    addi t1, t1, 1
    jal zero, multiply_loop

multiply_done:
    add a0, zero,t0              # result in a0
    jalr zero, 0(ra)

done:
    jal zero, done         # halt
