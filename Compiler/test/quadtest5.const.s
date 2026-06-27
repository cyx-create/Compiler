.global __$main__^main
__$main__^main:
L110:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
L102:
L103:
L107:
L104:
movw t108, #100
mov r0, t108
bl putint
movw t109, #10
mov r0, t109
bl putch
movw t110, #1
mov r0, t110
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

