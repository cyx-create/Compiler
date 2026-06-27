.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	mov r6, #1
	mov r5, #2
	movw r0, #0
	cmp r6, r0
	bne main$L107
main$L108:
	mov r0, r6
	bl putint
	mov r0, r4
main$L109:
main$L100110:
	movw r0, #0
	mov r0, r0
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr
main$L107:
	mov r0, #0
	cmp r6, r5
	blt main$L104
main$L105:
	mov r4, r0
main$L100111:
	add r0, r4, r6
	mov r0, r0
	bl putint
	mov r0, r4
	b main$L100110
main$L104:
	mov r4, r0
main$L100112:
	mov r0, #1
	mov r4, r0
	b main$L100111

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
