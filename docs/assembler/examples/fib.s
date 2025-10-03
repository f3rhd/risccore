# compute fib(5) -> a0, then infinite loop
start:
    addi    t3, zero, 5      # n = 5

    addi    t0, zero, 0      # f0 = 0
    addi    t1, zero, 1      # f1 = 1

    beq     t3, zero, done_zero   # if n==0 -> a0 = 0
    addi    t4, zero, 1
    beq     t3, t4, done_one    # if n==1 -> a0 = 1

    addi    t2, zero, 1      # i = 1

loop:
    blt     t3, t2, after_loop  # if n < i, finished
    add     t5, t0, t1      # t5 = f0 + f1
    add     t0, t1, x0      # f0 = f1
    add     t1, t5, x0      # f1 = t5
    addi    t2, t2, 1       # i++
    blt     t2, t3, loop    # while i < n continue
    # one more check: if i == n we already computed f_n -> fall through

after_loop:
    add     a0, t1, x0      # result in a0

    jal     zero, hang        # jump to infinite loop

done_zero:
    add     a0, zero, zero
    jal     zero, hang

done_one:
    add     a0, zero, t1
    jal     zero, hang

hang:
    jal     zero, hang        # infinite loop


