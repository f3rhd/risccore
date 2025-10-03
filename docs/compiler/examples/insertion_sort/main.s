.reset_vector:
	li sp, 0xFFFF0
	call .main
.insertion_sort:
	addi    sp,sp,-32
	sw      ra,28(sp)
	sw      s0,24(sp)
	addi    s0,sp,32
	sw      a0,-12(s0)
	sw      a1,-16(s0)
	li      t1,1
	sw      t1,-20(s0)
	li      t2,0
	sw      t2,-24(s0)
	li      t2,0
	sw      t2,-28(s0)
	j       .L0
.L0:
	lw      t1,-20(s0)
	slli    t0,t1,2
	lw      t3,-12(s0)
	add     t0,t3,t0
	lw      t0,0(t0)
	mv      t2,t0
	sw      t2,-28(s0)
	lw      t1,-20(s0)
	li      t6,1
	sub     t0,t1,t6
	mv      t2,t0
	sw      t2,-24(s0)
	j       .L2
.L3:
	lw      t2,-24(s0)
	addi    t0,t2,1
	slli    t0,t0,2
	lw      t3,-12(s0)
	add     t1,t3,t0
	lw      t2,-24(s0)
	slli    t0,t2,2
	lw      t3,-12(s0)
	add     t0,t3,t0
	lw      t0,0(t0)
	sw      t0,0(t1)
	lw      t2,-24(s0)
	li      t6,1
	sub     t0,t2,t6
	mv      t2,t0
	sw      t2,-24(s0)
.L2:
	lw      t2,-24(s0)
	blt     t2,zero,.L4
	lw      t2,-24(s0)
	slli    t0,t2,2
	lw      t3,-12(s0)
	add     t0,t3,t0
	lw      t0,0(t0)
	lw      t2,-28(s0)
	bgt     t0,t2,.L3
.L4:
	nop     
.L5:
	lw      t2,-24(s0)
	addi    t0,t2,1
	slli    t0,t0,2
	lw      t3,-12(s0)
	add     t0,t3,t0
	lw      t2,-28(s0)
	sw      t2,0(t0)
.L1:
	lw      t1,-20(s0)
	addi    t1,t1,1
	sw      t1,-20(s0)
	lw      t1,-20(s0)
	lw      t0,-16(s0)
	blt     t1,t0,.L0
.L6:
	nop     
	lw      ra,28(sp)
	lw      s0,24(sp)
	addi    sp,sp,32
	jr      ra
.main:
	addi    sp,sp,-32
	sw      ra,28(sp)
	sw      s0,24(sp)
	addi    s0,sp,32
	li      t0,5
	sw      t0,-28(s0)
	li      t0,4
	sw      t0,-24(s0)
	li      t0,3
	sw      t0,-20(s0)
	li      t0,2
	sw      t0,-16(s0)
	li      t0,1
	sw      t0,-12(s0)
	addi    t6,s0,-28
	mv      a0,t6
	li      a1,5
	call    .insertion_sort
	mv      t0,a0
	li      t6,0
	slli    t0,t6,2
	addi    t6,s0,-28
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,1
	bne     t0,t6,.L7
	li      t6,1
	slli    t0,t6,2
	addi    t6,s0,-28
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,2
	bne     t0,t6,.L7
	li      t6,2
	slli    t0,t6,2
	addi    t6,s0,-28
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,3
	bne     t0,t6,.L7
	li      t6,3
	slli    t0,t6,2
	addi    t6,s0,-28
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,4
	bne     t0,t6,.L7
	li      t6,4
	slli    t0,t6,2
	addi    t6,s0,-28
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,5
	bne     t0,t6,.L7
	j       .L9
.L10:
	nop     
.L9:
	li      t6,1
	bne     t6,zero,.L10
.L11:
	nop     
.L7:
	lw      ra,28(sp)
	lw      s0,24(sp)
	addi    sp,sp,32
	jr      ra
