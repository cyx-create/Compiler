.balign 4
.global B$f
.section .text
.arm
B$f:
B$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r0
	movw r0, #0
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
	mov r0, #0
	mov r0, #0
	movw r0, #12
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =A$f
	str r1, [r0, #8]
	mov r4, r0
	movw r0, #1
	str r0, [r4]
	movw r0, #12
	mov r0, r0
	bl malloc
	mov r0, r0
	ldr r1, =B$f
	str r1, [r0, #8]
	movw r1, #10
	str r1, [r0, #4]
	ldr r1, [r4]
	ldr r0, [r0, #4]
	add r0, r1, r0
	mov r0, r0
	bl putint
	movw r0, #10
	mov r0, r0
	bl putch
	movw r0, #0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global A$f
.section .text
.arm
A$f:
A$f$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, r0
	movw r0, #0
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
