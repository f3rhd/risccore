# recursive factorial of 5 -> result in a0
start:
    addi    sp, zero, 0xFFF0   # set stack pointer (assuming top of RAM)
    addi    a0, zero, 5        # n = 5
    jal     ra, fact           # call fact(5)

    jal     zero, hang         # infinite loop with result in a0

# int fact(int n)
# input:  a0 = n
# output: a0 = result
fact:
    addi    t0, zero, 1        # t0 = 1
    beq     a0, zero, base     # if n == 0 -> return 1

    # save ra and n on stack
    addi    sp, sp, -8
    sw      ra, 4(sp)
    sw      a0, 0(sp)

    addi    a0, a0, -1         # a0 = n-1
    jal     ra, fact           # call fact(n-1)

    # restore n
    lw      t1, 0(sp)          # t1 = n
    mul     a0, a0, t1         # a0 = fact(n-1) * n

    # restore ra
    lw      ra, 4(sp)
    addi    sp, sp, 8
    ret
base:
    add     a0, zero, t0       # return 1
    ret
hang:
    jal     zero, hang         # infinite loop
