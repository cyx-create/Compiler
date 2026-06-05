.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #12
	add fp, sp, #44
	bl getint
	mov r10, #10
	str r10, [fp, #-44]
	mov r10, #0
	str r10, [fp, #-40]
	movw r1, #0
	cmp r0, r1
	bgt main$L107
main$L108:
	mov r0, #4
main$L109:
	bl putint
	movw r0, #10
	bl putch
	sub sp, fp, #44
	add sp, sp, #12
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	ldr r9, [fp, #-44]
	mov r0, r9
	ldr r9, [fp, #-40]
	mov r1, r9
main$L104:
	movw r2, #0
	cmp r0, r2
	bgt main$L105
main$L106:
	mov r0, r1
	b main$L109
main$L105:
	movw r1, #3
	mul r1, r1, r0
	sub r4, r0, #1
	add r5, r1, #2
	mov r0, r5
	bl putint
	movw r0, #32
	bl putch
	mov r0, r4
	mov r1, r5
	b main$L104

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
