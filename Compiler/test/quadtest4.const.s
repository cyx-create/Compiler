.global __$main__^main
__$main__^main:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #10
mov t10001, t10000
L102:
L100105:
movw t108, #0
cmp t10001, t108
bne L103
L104:
mov t10002, t10001
mov r0, t10002
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
mov t10002, t10001
mov r0, t10002
bl putint
movw t107, #10
mov r0, t107
bl putch
sub t10003, t10002, #1
mov t10001, t10003
b L100105

