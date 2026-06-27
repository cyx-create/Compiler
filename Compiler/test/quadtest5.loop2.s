.global __$main__^main
__$main__^main:
L110:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #1
mov t10100, #2
L102:
movw t108, #0
cmp t10000, t108
bne L103
L104:
movw t106, #100
mov r0, t106
bl putint
movw t107, #10
mov r0, t107
bl putch
movw t109, #1
mov r0, t109
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
movw t110, #0
cmp t10100, t110
bne L107
L108:
b L102
L107:
b L104

