.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #1
mov t10100, #2
bl getint
mov t10400, t0
mov t10500, t10400
mov t10300, #0
movw t108, #1
cmp t10000, t108
bgt L100108
L104:
mov t10301, t10300
L100109:
movw t109, #2
cmp t10100, t109
blt L105
L106:
L100110:
add t10700, t10500, t10301
mov r0, t10700
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L105:
L100111:
mov t10302, #1
mov t10301, t10302
b L100110
L100108:
mov t10301, t10300
b L100111

