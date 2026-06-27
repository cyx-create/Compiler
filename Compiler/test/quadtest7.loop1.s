.global B^f
B^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t103, #0
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
movw t119, #12
mov r0, t119
bl malloc
mov t10300, t0
adr t120, A^f
str t120, [t10300, #8]
mov t10001, t10300
movw t121, #1
str t121, [t10001]
movw t122, #12
mov r0, t122
bl malloc
mov t10400, t0
adr t123, B^f
str t123, [t10400, #8]
mov t10101, t10400
movw t124, #10
str t124, [t10101, #4]
ldr t11600, [t10001]
ldr t11800, [t10101, #4]
add t10700, t11600, t11800
mov r0, t10700
bl putint
movw t125, #10
mov r0, t125
bl putch
movw t126, #0
mov r0, t126
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
movw t103, #0
mov r0, t103
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

