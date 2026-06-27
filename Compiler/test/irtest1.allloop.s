.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #1
mov r0, t10000
bl putint
movw t105, #10
mov r0, t105
bl putch
mov r0, t10000
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

