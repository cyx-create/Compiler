.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t107, #1
mov r0, t107
bl putint
movw t108, #10
mov r0, t108
bl putch
movw t109, #1
mov r0, t109
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

