.global C^max
C^max:
L105:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
mov t101, t1
mov t102, t2
cmp t101, t102
bgt L102
L103:
mov r0, t102
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L102:
mov r0, t101
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t114, #4
mov r0, t114
bl malloc
mov t10100, t0
adr t115, C^max
str t115, [t10100]
mov t10200, t10100
ldr t10400, [t10200]
mov r12, t10400
movw t116, #100
movw t117, #200
mov r0, t10200
mov r1, t116
mov r2, t117
blx r12
mov t10500, t0
mov r0, t10500
bl putint
movw t118, #1
mov r0, t118
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

