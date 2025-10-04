.reset_vector:
	li sp, 0xFFFF0
	call .main
.fac:
	addi    sp,sp,-16
	sw      ra,12(sp)
	sw      s0,8(sp)
	addi    s0,sp,16
	sw      a0,-12(s0)
	lw      t0,-12(s0)
	li      t5,1
	bgt     t0,t5,.L0
	lw      t0,-12(s0)
	mv      a0,t0
	lw      ra,12(sp)
	lw      s0,8(sp)
	addi    sp,sp,16
	jr      ra
.L0:
	lw      t0,-12(s0)
	li      t6,1
	sub     t1,t0,t6
	mv      a0,t1
	call    .fac
	mv      t2,a0
	lw      t0,-12(s0)
	mul     t3,t0,t2
	mv      a0,t3
	lw      ra,12(sp)
	lw      s0,8(sp)
	addi    sp,sp,16
	jr      ra
.main:
	addi    sp,sp,-16
	sw      ra,12(sp)
	sw      s0,8(sp)
	addi    s0,sp,16
	li      a0,5
	call    .fac
	mv      t0,a0
	mv      t1,t0
	sw      t1,-12(s0)
	j       .L2
.L3:
	nop     
.L2:
	li      t6,1
	bne     t6,zero,.L3
.L4:
	lw      ra,12(sp)
	lw      s0,8(sp)
	addi    sp,sp,16
	jr      ra
