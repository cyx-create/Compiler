.global A^f
A^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t105, #0
mov r0, t105
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
movw t123, #20
mov r0, t123
bl malloc
mov t10300, t0
adr t124, A^f
str t124, [t10300, #12]
mov t10001, t10300
movw t125, #1
str t125, [t10001]
movw t126, #20
mov r0, t126
bl malloc
mov t10400, t0
adr t127, B^f
str t127, [t10400, #12]
adr t128, B^g
str t128, [t10400, #16]
mov t10101, t10400
movw t129, #1
str t129, [t10101, #8]
add t11900, t10001, #12
ldr t10700, [t11900]
mov r12, t10700
mov r0, t10001
blx r12
mov t10800, t0
mov r0, t10800
bl putint
mov t10002, t10101
add t12000, t10002, #12
ldr t11000, [t12000]
mov r12, t11000
mov r0, t10002
blx r12
mov t11100, t0
mov r0, t11100
bl putint
movw t130, #0
mov r0, t130
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global B^g
B^g:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
add t10500, t100, #12
ldr t10200, [t10500]
mov r12, t10200
mov r0, t100
blx r12
mov t10300, t0
mov r0, t10300
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global B^f
B^f:
L103:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t101, t0
ldr t11300, [t-1]
add t11400, t101, #8
ldr t11500, [t11400]
add t10700, t11300, t11500
ldr t10300, [t101, #4]
mov t10600, t10300
ldr t10400, [t10300]
L101:
movw t125, #0
cmp t125, t10400
bge L100
L102:
ldr t12000, [t10600, #4]
add t11000, t10700, t12000
add t12100, t101, #16
ldr t10800, [t12100]
mov r12, t10800
mov r0, t101
blx r12
mov t10900, t0
add t12200, t11000, t10900
mov r0, t12200
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L100:
movw t126, #65535
movt t126, #65535
mov r0, t126
bl exit

