.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10600, #4
movw t107, #4
mul t10300, t10600, t107
mov r0, t10300
bl malloc
mov t10200, t0
movw t108, #3
str t108, [t10200]
mov t10000, t10200
movw t109, #0
mov r0, t109
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

