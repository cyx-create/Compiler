.global A^f
A^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t105, #100
mov r0, t105
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t123, #16
mov r0, t123
bl malloc
mov t10300, t0
adr t124, A^f
str t124, [t10300, #8]
mov t10001, t10300
movw t125, #4
str t125, [t10001]
movw t126, #16
mov r0, t126
bl malloc
mov t10400, t0
adr t127, B^f
str t127, [t10400, #8]
adr t128, B^g
str t128, [t10400, #12]
mov t10101, t10400
movw t129, #2
str t129, [t10101, #4]
mov t10002, t10101
add t11800, t10002, #8
ldr t10700, [t11800]
mov r12, t10700
mov r0, t10002
blx r12
mov t10800, t0
ldr t12000, [t10101, #4]
add t10900, t10800, t12000
mov r0, t10900
bl putint
movw t130, #10
mov r0, t130
bl putch
movw t131, #0
mov r0, t131
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global B^g
B^g:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t105, #300
mov r0, t105
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global B^f
B^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t105, #200
mov r0, t105
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

