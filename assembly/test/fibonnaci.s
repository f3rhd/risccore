# Assume a0 = 10 (number of Fibonacci terms to generate)
# We hardcode that with addi
# s0 = 0x10000000 (memory base address)

    addi a0, zero, 10        # a0 = 10
    lui s0, 0x10000        # s0 = 0x10000000 (base address)

    addi t0, zero, 0         # t0 = fib(0)
    addi t1, zero, 1         # t1 = fib(1)
    mv   t2, a0            # t2 = counter (remaining numbers to compute)

# Store fib(0)
    sw t0, 0(s0)
    addi s0, s0, 4
    addi t2, t2, -1
    beqz t2, done

# Store fib(1)
    sw t1, 0(s0)
    addi s0, s0, 4
    addi t2, t2, -1
    beqz t2, done

# Loop
loop:
    add  t3, t0, t1        # t3 = t0 + t1
    sw   t3, 0(s0)         # store fib(i)
    addi s0, s0, 4         # advance memory ptr
    mv   t0, t1            # shift window
    mv   t1, t3
    addi t2, t2, -1
    bnez t2, loop

done:
    j done                 # infinite loop
