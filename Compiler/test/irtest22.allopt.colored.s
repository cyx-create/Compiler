.balign 4
.global main
.section .text
.arm
main:
main$L113:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #4
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =fib$f
	str r1, [r0]
	mov r4, r0
	movw r0, #69
	mov r0, r0
	bl putch
	movw r0, #110
	mov r0, r0
	bl putch
	movw r0, #116
	mov r0, r0
	bl putch
	movw r0, #101
	mov r0, r0
	bl putch
	movw r0, #114
	mov r0, r0
	bl putch
	movw r0, #32
	mov r0, r0
	bl putch
	movw r0, #116
	mov r0, r0
	bl putch
	movw r0, #104
	mov r0, r0
	bl putch
	movw r0, #101
	mov r0, r0
	bl putch
	movw r0, #32
	mov r0, r0
	bl putch
	movw r0, #110
	mov r0, r0
	bl putch
	movw r0, #117
	mov r0, r0
	bl putch
	movw r0, #109
	mov r0, r0
	bl putch
	movw r0, #98
	mov r0, r0
	bl putch
	movw r0, #101
	mov r0, r0
	bl putch
	movw r0, #114
	mov r0, r0
	bl putch
	movw r0, #32
	mov r0, r0
	bl putch
	movw r0, #111
	mov r0, r0
	bl putch
	movw r0, #102
	mov r0, r0
	bl putch
	movw r0, #32
	mov r0, r0
	bl putch
	movw r0, #116
	mov r0, r0
	bl putch
	movw r0, #101
	mov r0, r0
	bl putch
	movw r0, #114
	mov r0, r0
	bl putch
	movw r0, #109
	mov r0, r0
	bl putch
	movw r0, #58
	mov r0, r0
	bl putch
	bl getint
	mov r7, r0
	movw r0, #0
	cmp r7, r0
	blt main$L100113
main$L104:
	movw r0, #47
	cmp r7, r0
	bgt main$L105
main$L106:
main$L107:
	add r8, r5, #1
	mov r5, r5
main$L110:
main$L100114:
	cmp r5, r7
	blt main$L111
main$L112:
	movw r0, #10
	mov r0, r0
	bl putch
	movw r0, #0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L111:
	ldr r0, [r6]
	mov r12, r0
	mov r0, r4
	mov r1, r5
	blx r12
	mov r0, r0
	mov r0, r0
	bl putint
	movw r0, #32
	mov r0, r0
	bl putch
	mov r5, r8
	b main$L100114
main$L105:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L100113:
	b main$L105

.balign 4
.global fib$f
.section .text
.arm
fib$f:
fib$f$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r6, r0
	mov r5, r1
	movw r0, #0
	cmp r5, r0
	beq fib$f$L100108
fib$f$L104:
	movw r0, #1
	cmp r5, r0
	beq fib$f$L105
fib$f$L106:
	ldr r0, [r6]
	mov r1, r0
	mov r0, r6
	sub r2, r5, #1
	mov r12, r1
	mov r0, r0
	mov r1, r2
	blx r12
	mov r0, r0
	mov r4, r0
	mov r0, r6
	ldr r0, [r0]
	mov r1, r6
	sub r2, r5, #2
	mov r12, r0
	mov r0, r1
	mov r1, r2
	blx r12
	mov r0, r0
	add r0, r4, r0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
fib$f$L105:
	mov r0, r5
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
fib$f$L100108:
	b fib$f$L105

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
