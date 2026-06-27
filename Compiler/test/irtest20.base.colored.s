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
	mov r4, #0
	ldr r1, [r0]
	ldr r0, [r0, #4]
	add r1, r1, r0
	ldr r0, [r4]
	add r1, r1, r0
	add r0, r4, #8
	ldr r0, [r0]
	add r1, r1, r0
	add r0, r4, #12
	ldr r0, [r0]
	add r0, r1, r0
	mov r0, r0
	bl putint
	mov r0, r4
	ldr r1, [r0]
	ldr r0, [r0, #4]
	add r1, r1, r0
	ldr r0, [r4]
	add r1, r1, r0
	add r0, r4, #8
	ldr r0, [r0]
	add r0, r1, r0
	add r1, r4, #12
	ldr r1, [r1]
	add r0, r0, r1
	mov r0, r0
	bl putint
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

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
