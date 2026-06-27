.global C^m
C^m:
L103:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
mov t101, t1
mov t10500, t101
ldr t10300, [t101]
L101:
movw t113, #1
cmp t113, t10300
bge L100
L102:
add t10900, t10500, #8
ldr t11000, [t10900]
mov r0, t11000
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr
L100:
movw t114, #65535
movt t114, #65535
mov r0, t114
bl exit

.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t118, #16
mov r0, t118
bl malloc
mov t10000, t0
movw t119, #3
str t119, [t10000]
movw t120, #1
str t120, [t10000, #4]
movw t121, #2
str t121, [t10000, #8]
movw t122, #3
str t122, [t10000, #12]
movw t123, #4
mov r0, t123
bl malloc
mov t10300, t0
adr t124, C^m
str t124, [t10300]
mov t10101, t10300
ldr t10600, [t10101]
mov r12, t10600
mov r0, t10101
mov r1, t10000
blx r12
mov t10700, t0
mov r0, t10700
bl putint
movw t125, #10
mov r0, t125
bl putch
movw t126, #0
mov r0, t126
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

