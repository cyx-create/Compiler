.global __$main__^main
__$main__^main:
L110:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
L107:
mov t10300, #0
L104:
mov t10301, t10300
L105:
mov t10301, #1
add t10400, t10301, #1
mov r0, t10400
bl putint
L109:
mov t10300, t10301
movw t110, #0
mov r0, t110
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

