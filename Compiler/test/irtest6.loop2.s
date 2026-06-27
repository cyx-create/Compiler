.global __$main__^main
__$main__^main:
L110:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #1
mov t10100, #2
movw t108, #0
cmp t10000, t108
bne L107
L108:
mov r0, t10000
bl putint
mov t10300, t103
L109:
L100110:
movw t109, #0
mov r0, t109
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L107:
mov t10300, #0
cmp t10000, t10100
blt L104
L105:
mov t10301, t10300
L100111:
add t10400, t10301, t10000
mov r0, t10400
bl putint
mov t10300, t10301
b L100110
L104:
mov t10301, t10300
L100112:
mov t10302, #1
mov t10301, t10302
b L100111

