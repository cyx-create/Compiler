.global __$main__^main
__$main__^main:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
bl getint
mov t10000, t0
mov t10001, t10000
L102:
L100105:
movw t119, #0
cmp t10001, t119
bgt L103
L104:
mov r0, t10001
bl putint
movw t118, #10
mov r0, t118
bl putch
movw t120, #2
mov r0, t120
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
movw t116, #4
mul t10900, t116, t10001
add t10300, t10900, #2
mov r0, t10300
bl putint
movw t117, #32
mov r0, t117
bl putch
sub t10002, t10001, #2
mov t10001, t10002
b L100105

