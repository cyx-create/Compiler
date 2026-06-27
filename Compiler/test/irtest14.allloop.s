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

.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t107, #8
mov r0, t107
bl malloc
mov t10200, t0
adr t108, A^f
str t108, [t10200, #4]
mov t10001, t10200
movw t109, #1
str t109, [t10001]
movw t110, #0
mov r0, t110
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

