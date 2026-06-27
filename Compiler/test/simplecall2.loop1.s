.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
movw t116, #16
mov r0, t116
bl malloc
mov t10000, t0
movw t117, #3
str t117, [t10000]
movw t118, #1
str t118, [t10000, #4]
movw t119, #2
str t119, [t10000, #8]
movw t120, #3
str t120, [t10000, #12]
mov t10100, #0
movw t121, #4
mov r0, t121
bl malloc
mov t10300, t0
adr t122, C^m
str t122, [t10300]
mov t10101, t10300
ldr t10600, [t10101]
mov r12, t10600
mov r0, t10101
mov r1, t10000
blx r12
mov t10700, t0
mov r0, t10700
bl putint
movw t123, #10
mov r0, t123
bl putch
movw t124, #0
mov r0, t124
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

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
movw t112, #1
movw t113, #0
cmp t112, t113
bge L101
L100:
movw t114, #65535
movt t114, #65535
mov r0, t114
bl exit
L101:
movw t115, #1
cmp t115, t10300
bge L100
L102:
mov t10700, #2
movw t111, #4
mul t10800, t10700, t111
ldr t11000, [t10500, t10800]
mov r0, t11000
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

