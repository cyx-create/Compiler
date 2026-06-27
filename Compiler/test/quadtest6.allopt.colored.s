.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	movw r0, #24
	mov r0, r0
	bl malloc
	mov r6, r0
	movw r0, #5
	str r0, [r6]
	movw r0, #1
	str r0, [r6, #4]
	movw r0, #2
	str r0, [r6, #8]
	movw r0, #3
	str r0, [r6, #12]
	movw r0, #4
	str r0, [r6, #16]
	movw r0, #5
	str r0, [r6, #20]
	mov r1, #0
	mov r0, r6
	mov r4, r5
	mov r0, r4
	add r2, r0, #1
	movw r3, #4
	mul r2, r2, r3
	add r8, r0, #1
	mov r0, r1
main$L102:
main$L100108:
	ldr r1, [r6]
	cmp r0, r1
	blt main$L103
main$L104:
	mov r5, r0
	movw r1, #1
	mov r0, r1
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	mov r5, r0
	ldr r9, [r6]
	movw r0, #0
	cmp r5, r0
	bge main$L106
main$L105:
	mov r4, r5
main$L100109:
	movw r0, #65535
	movt r0, #65535
	mov r0, r0
	bl exit
main$L106:
	cmp r4, r9
	bge main$L105
main$L107:
	ldr r0, [r7]
	mov r0, r0
	bl putint
	movw r0, #10
	mov r0, r0
	bl putch
	mov r0, r8
	b main$L100108

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
