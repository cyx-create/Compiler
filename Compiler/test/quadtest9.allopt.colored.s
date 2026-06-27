.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r1, r0
	movw r0, #1
	str r0, [r1]
	movw r0, #2
	str r0, [r1, #4]
	movw r0, #0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r2, r0
	movw r0, #4
	str r0, [r2]
	movw r0, #30
	str r0, [r2, #8]
	movw r0, #100
	str r0, [r2, #12]
	ldr r1, [r2]
	add r0, r2, #8
	ldr r0, [r0]
	add r0, r1, r0
	add r1, r2, #12
	ldr r1, [r1]
	add r0, r0, r1
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #20
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =A$f
	str r1, [r0, #16]
	mov r5, r0
	movw r0, #20
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =B$f
	str r1, [r0, #16]
	mov r4, r0
	add r0, r5, #16
	ldr r0, [r0]
	mov r12, r0
	mov r0, r5
	blx r12
	add r0, r4, #16
	ldr r0, [r0]
	mov r12, r0
	mov r0, r4
	blx r12
	mov r0, r5
	ldr r0, [r0]
	ldr r1, [r5, #4]
	add r1, r0, r1
	mov r0, r4
	ldr r0, [r0]
	add r1, r1, r0
	add r0, r4, #8
	ldr r0, [r0]
	add r1, r1, r0
	add r0, r4, #12
	ldr r0, [r0]
	add r0, r1, r0
	mov r0, r0
	bl putint
	movw r0, #10
	mov r0, r0
	bl putch
	mov r0, r4
	ldr r1, [r0]
	mov r0, r4
	ldr r0, [r0]
	add r1, r1, r0
	add r0, r4, #8
	ldr r0, [r0]
	add r0, r1, r0
	add r1, r4, #12
	ldr r1, [r1]
	add r0, r0, r1
	mov r0, r0
	bl putint
	movw r0, #10
	mov r0, r0
	bl putch
	movw r0, #1
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
