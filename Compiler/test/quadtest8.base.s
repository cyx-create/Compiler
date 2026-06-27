.global B^f
B^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t103, #200
mov r0, t103
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global A^f
A^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t103, #100
mov r0, t103
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
movw t103, #300
mov r0, t103
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
mov t10000, #0
mov t10100, #0
movw t121, #16
mov r0, t121
bl malloc
mov t10300, t0
adr t122, A^f
str t122, [t10300, #8]
mov t10001, t10300
movw t123, #4
str t123, [t10001]
movw t124, #16
mov r0, t124
bl malloc
mov t10400, t0
adr t125, B^f
str t125, [t10400, #8]
adr t126, B^g
str t126, [t10400, #12]
mov t10101, t10400
movw t127, #2
str t127, [t10101, #4]
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
movw t128, #10
mov r0, t128
bl putch
movw t129, #0
mov r0, t129
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

