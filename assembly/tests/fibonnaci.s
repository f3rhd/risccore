# a0 = number of Fibonacci terms
# sp = 0 (start of stack)
# Stack grows upward: sp += 4 on each push
# Fibonacci sequence stored starting at address 0, 4, 8, ...

    addi a0, zero, 10       # a0 = number of Fibonacci numbers
    addi sp, zero, 0        # stack pointer = 0 (base address)

    addi t0, zero, 0        # t0 = fib(0)
    addi t1, zero, 1        # t1 = fib(1)
    mv   t2, a0             # t2 = counter

# Store fib(0)
    sw   t0, 0(sp)
    addi sp, sp, 4
    addi t2, t2, -1
    beqz t2, done

# Store fib(1)
    sw   t1, 0(sp)
    addi sp, sp, 4
    addi t2, t2, -1
    beqz t2, done

# Loop
loop:
    add  t3, t0, t1         # t3 = fib(i)
    sw   t3, 0(sp)          # store at stack[sp]
    addi sp, sp, 4          # move stack pointer up
    mv   t0, t1             # shift window
    mv   t1, t3
    addi t2, t2, -1
    bnez t2,loop
done:
    j done                  # infinite loop to halt