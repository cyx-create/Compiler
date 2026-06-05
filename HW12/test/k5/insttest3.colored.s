.balign 4
.global C$m
.section .text
.arm
C$m:
C$m$L100:
	push {r4-r10, fp, lr}
	sub sp, sp, #4
	add fp, sp, #36
	ldr r0, [r0]
	sub sp, fp, #36
	add sp, sp, #4
	pop {r4-r10, fp, lr}
	bx lr

.balign 4
.global main
.section .text
.arm
main:
main$L108:
	push {r4-r10, fp, lr}
	sub sp, sp, #16
	add fp, sp, #48
	movw r0, #8
	bl malloc
	mov r4, r0
	mov r5, #3
	movw r0, #20
	bl malloc
	mov r3, r0
	movw r0, #4
	str r0, [r3]
	mov r0, r4
	add r2, r4, #4
	mov r1, r4
	movw r4, #1
	str r4, [r3, #4]
	movw r4, #2
	str r4, [r3, #8]
	movw r4, #3
	str r4, [r3, #12]
	movw r4, #4
	str r4, [r3, #16]
	str r3, [r0]
	ldr r0, =C$m
	str r0, [r2]
	ldr r2, [r1, #4]
	mov r0, r1
	blx r2
	mov r10, r0
	str r10, [fp, #-40]
	mov r10, r5
	str r10, [fp, #-44]
main$L102:
	movw r0, #0
	ldr r9, [fp, #-44]
	cmp r9, r0
	bge main$L103
main$L104:
	movw r0, #10
	bl putch
	movw r0, #2
	sub sp, fp, #48
	add sp, sp, #16
	pop {r4-r10, fp, lr}
	bx lr
main$L103:
	ldr r9, [fp, #-44]
	sub r10, r9, #1
	str r10, [fp, #-48]
	ldr r9, [fp, #-40]
	mov r4, r9
	ldr r9, [fp, #-40]
	ldr r5, [r9]
	movw r0, #0
	ldr r9, [fp, #-48]
	cmp r9, r0
	bge main$L106
main$L105:
	movw r0, #65535
	movt r0, #65535
	bl exit
main$L106:
	ldr r9, [fp, #-48]
	cmp r9, r5
	bge main$L105
main$L107:
	ldr r9, [fp, #-48]
	add r0, r9, #1
	movw r1, #4
	mul r0, r0, r1
	ldr r0, [r4, r0]
	bl putint
	movw r0, #32
	bl putch
	ldr r9, [fp, #-48]
	str r9, [fp, #-44]
	b main$L102

.global malloc
.global getint
.global getch
.global getarray
.global putint
.global putch
.global putarray
.global starttime
.global stoptime
