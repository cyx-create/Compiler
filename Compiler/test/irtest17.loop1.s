.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #0
mov t10100, #0
movw t117, #16
mov r0, t117
bl malloc
mov t10300, t0
adr t118, A^f
str t118, [t10300, #8]
mov t10001, t10300
movw t119, #1
str t119, [t10001]
movw t120, #16
mov r0, t120
bl malloc
mov t10400, t0
adr t121, B^f
str t121, [t10400, #8]
adr t122, B^g
str t122, [t10400, #12]
mov t10101, t10400
movw t123, #1
str t123, [t10101, #4]
mov t10002, t10101
add t11600, t10002, #8
ldr t10700, [t11600]
mov r12, t10700
mov r0, t10002
blx r12
mov t10800, t0
mov r0, t10800
bl putint
movw t124, #0
mov r0, t124
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
movw t103, #0
mov r0, t103
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global B^f
B^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t103, #0
mov r0, t103
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

.global A^f
A^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t103, #0
mov r0, t103
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

