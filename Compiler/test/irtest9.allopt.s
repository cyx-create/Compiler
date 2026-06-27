.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t109, #16
mov r0, t109
bl malloc
mov t10200, t0
movw t110, #3
str t110, [t10200]
movw t111, #0
mov r0, t111
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

