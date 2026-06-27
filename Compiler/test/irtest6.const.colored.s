.balign 4
.global main
.section .text
.arm
main:
main$L110:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
main$L107:
	mov r0, #0
main$L104:
	mov r4, r0
main$L105:
	mov r4, #1
	add r0, r4, #1
	mov r0, r0
	bl putint
main$L109:
	mov r0, r4
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
