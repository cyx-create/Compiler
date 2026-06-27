.global __$main__^main
__$main__^main:
L108:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #1
mov t10003, t10002
sub t10004, t10003, #1
mov t10001, t10000
L105:
L100108:
movw t109, #0
cmp t10001, t109
bgt L104
L107:
mov t10002, t10001
L100109:
mov r0, t10002
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L104:
mov t10002, t10001
movw t110, #65535
movt t110, #65535
cmp t10002, t110
bgt L106
L106:
mov r0, t10003
bl putint
movw t108, #10
mov r0, t108
bl putch
mov t10001, t10004
b L100108

