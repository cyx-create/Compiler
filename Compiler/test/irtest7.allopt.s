.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
bl getint
mov t10400, t0
mov t10500, t10400
L104:
mov t10301, #0
L106:
mov t10301, t10301
add t10700, t10500, t10301
mov r0, t10700
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

