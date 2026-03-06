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
	li      t3,1
	sw      t3,-20(s0)
	li      t2,0
	sw      t2,-24(s0)
	li      t4,0
	sw      t4,-28(s0)
	j       .L0
.L0:
	lw      t3,-20(s0)
	slli    t0,t3,2
	lw      t1,-12(s0)
	add     t0,t1,t0
	lw      t0,0(t0)
	mv      t4,t0
	sw      t4,-28(s0)
	lw      t3,-20(s0)
	li      t6,1
	sub     t0,t3,t6
	mv      t2,t0
	sw      t2,-24(s0)
	j       .L2
.L3:
	lw      t2,-24(s0)
	addi    t0,t2,1
	slli    t0,t0,2
	lw      t1,-12(s0)
	add     t3,t1,t0
	lw      t2,-24(s0)
	slli    t0,t2,2
	lw      t1,-12(s0)
	add     t0,t1,t0
	lw      t0,0(t0)
	sw      t0,0(t3)
	lw      t2,-24(s0)
	li      t6,1
	sub     t0,t2,t6
	mv      t2,t0
	sw      t2,-24(s0)
.L2:
	lw      t2,-24(s0)
	blt     t2,zero,.L4
.L5:
	lw      t2,-24(s0)
	slli    t0,t2,2
	lw      t1,-12(s0)
	add     t0,t1,t0
	lw      t0,0(t0)
	lw      t4,-28(s0)
	bgt     t0,t4,.L3
.L6:
	nop     
.L4:
	nop     
.L7:
	lw      t2,-24(s0)
	addi    t0,t2,1
	slli    t0,t0,2
	lw      t1,-12(s0)
	add     t0,t1,t0
	lw      t4,-28(s0)
	sw      t4,0(t0)
.L1:
	lw      t3,-20(s0)
	addi    t3,t3,1
	sw      t3,-20(s0)
	lw      t3,-20(s0)
	lw      t0,-16(s0)
	blt     t3,t0,.L0
.L8:
	nop     
.L9:
	nop     
	lw      ra,28(sp)
	lw      s0,24(sp)
	addi    sp,sp,32
	jr      ra
.main:
	addi    sp,sp,-64
	sw      ra,60(sp)
	sw      s0,56(sp)
	addi    s0,sp,64
	li      t0,5
	sw      t0,-32(s0)
	li      t0,4
	sw      t0,-28(s0)
	li      t0,3
	sw      t0,-24(s0)
	li      t0,2
	sw      t0,-20(s0)
	li      t0,1
	sw      t0,-16(s0)
	addi    t6,s0,-32
	mv      a0,t6
	li      a1,5
	call    .insertion_sort
	mv      t0,a0
	li      t6,0
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,1
	bne     t0,t6,.L10
.L12:
	li      t6,1
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,2
	bne     t0,t6,.L10
.L13:
	li      t6,2
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,3
	bne     t0,t6,.L10
.L14:
	li      t6,3
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,4
	bne     t0,t6,.L10
.L15:
	li      t6,4
	slli    t0,t6,2
	addi    t6,s0,-32
	add     t0,t0,t6
	lw      t0,0(t0)
	li      t6,5
	bne     t0,t6,.L10
.L16:
	j       .L17
.L18:
	nop     
.L17:
	li      t6,1
	bne     t6,zero,.L18
.L19:
	nop     
.L20:
	nop     
.L10:
	lw      ra,60(sp)
	lw      s0,56(sp)
	addi    sp,sp,64
	jr      ra
