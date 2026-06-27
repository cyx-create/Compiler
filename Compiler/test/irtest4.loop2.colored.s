.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r0, #1
	mov r0, r0
main$L105:
main$L100108:
	movw r1, #0
	cmp r0, r1
	bgt main$L100109
main$L104:
	mov r4, r0
main$L100110:
	movw r0, #0
	cmp r4, r0
	beq main$L106
main$L107:
	mov r0, r4
main$L100111:
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L106:
main$L100112:
	mov r0, r4
	bl putint
	movw r0, #10
	mov r0, r0
	bl putch
	sub r0, r4, #1
	mov r0, r0
	b main$L100108
main$L100109:
	mov r4, r0
	b main$L100112

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
