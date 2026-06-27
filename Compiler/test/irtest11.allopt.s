.global __$main__^main
__$main__^main:
L103:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
L100:
movw t112, #65535
movt t112, #65535
mov r0, t112
bl exit

