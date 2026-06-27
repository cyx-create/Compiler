.global __$main__^main
__$main__^main:
L110:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #1
mov t10100, #2
L102:
movw t104, #0
cmp t10000, t104
bne L103
L104:
movw t105, #0
mov r0, t105
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L103:
movw t106, #0
cmp t10100, t106
bne L107
L108:
b L102
L107:
b L104

