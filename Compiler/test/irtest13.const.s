.global __$main__^main
__$main__^main:
L103:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t115, #8
mov r0, t115
bl malloc
mov t10100, t0
movw t116, #1
str t116, [t10100]
mov t10200, t10100
mov t10700, t10200
ldr t10300, [t10200]
L101:
movw t118, #0
cmp t118, t10300
bge L100
L102:
movw t117, #0
str t117, [t10700, #4]
movw t119, #0
mov r0, t119
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L100:
movw t120, #65535
movt t120, #65535
mov r0, t120
bl exit

