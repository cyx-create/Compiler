.global __$main__^main
__$main__^main:
L103:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10900, #2
movw t113, #4
mul t10400, t10900, t113
mov r0, t10400
bl malloc
mov t10100, t0
movw t114, #1
str t114, [t10100]
mov t10200, t10100
mov t10700, t10200
ldr t10300, [t10200]
movw t117, #0
movw t118, #0
cmp t117, t118
bge L101
L100:
movw t119, #65535
movt t119, #65535
mov r0, t119
bl exit
L101:
movw t120, #0
cmp t120, t10300
bge L100
L102:
mov t11000, #1
movw t115, #4
mul t11100, t11000, t115
movw t116, #0
str t116, [t10700, t11100]
movw t121, #0
mov r0, t121
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

