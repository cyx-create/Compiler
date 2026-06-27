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
movw t112, #12
mov r0, t112
bl malloc
mov t10300, t0
adr t113, A^f
str t113, [t10300, #8]
mov t10001, t10300
movw t114, #1
str t114, [t10001]
movw t115, #12
mov r0, t115
bl malloc
mov t10400, t0
adr t116, B^f
str t116, [t10400, #8]
mov t10101, t10400
movw t117, #1
str t117, [t10101, #4]
movw t118, #0
mov r0, t118
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

