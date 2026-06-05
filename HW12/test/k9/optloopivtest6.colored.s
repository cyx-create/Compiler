.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #8
	add fp, sp, #40
	bl getint
	mov r10, r0
	str r10, [fp, #-40]
	mov r0, #10
	mov r1, #0
main$L102:
	movw r2, #0
	cmp r0, r2
	bgt main$L103
main$L104:
	mov r0, r1
	bl putint
	movw r2, #10
	mov r0, r2
	bl putch
	mov r0, r1
	sub sp, fp, #40
	add sp, sp, #8
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	movw r1, #0
	ldr r9, [fp, #-40]
	cmp r9, r1
	bgt main$L107
main$L108:
	mov r1, #4
	mov r4, r1
main$L109:
	sub r0, r0, #1
	mov r1, r4
	b main$L102
main$L107:
	movw r1, #3
	mul r1, r1, r0
	add r4, r1, #2
	mov r0, r4
	bl putint
	movw r1, #32
	mov r0, r1
	bl putch
	b main$L109

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
