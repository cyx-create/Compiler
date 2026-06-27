.balign 4
.global main
.section .text
.arm
main:
main$L113:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, #0
	mov r5, #0
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
	mov r6, r0
	movw r0, #0
	cmp r6, r0
	blt main$L100113
main$L104:
	mov r0, r5
main$L100114:
	movw r1, #47
	cmp r6, r1
	bgt main$L105
main$L106:
	mov r0, r0
main$L100115:
main$L107:
	mov r8, r0
main$L110:
main$L100116:
	cmp r8, r6
	blt main$L111
main$L112:
	mov r7, r8
main$L100117:
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
	mov r7, r8
main$L100118:
	ldr r0, [r4]
	mov r12, r0
	mov r0, r4
	mov r1, r7
	blx r12
	mov r0, r0
	mov r0, r0
	bl putint
	movw r0, #32
	mov r0, r0
	bl putch
	add r0, r7, #1
	mov r8, r0
	b main$L100116
main$L105:
main$L100119:
	movw r1, #0
	movw r0, #1
	sub r0, r1, r0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L100113:
	mov r0, r5
	b main$L100119

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
