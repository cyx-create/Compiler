.global __$main__^main
__$main__^main:
L103:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t110, #0
movw t111, #123
sub t10200, t110, t111
mov t10500, t100
ldr t10300, [t100]
movw t114, #0
cmp t10200, t114
bge L101
L100:
movw t115, #65535
movt t115, #65535
mov r0, t115
bl exit
L101:
cmp t10200, t10300
bge L100
L102:
add t10700, t10200, #1
movw t112, #4
mul t10800, t10700, t112
movw t113, #4
str t113, [t10500, t10800]
movw t116, #0
mov r0, t116
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

