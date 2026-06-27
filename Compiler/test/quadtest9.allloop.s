.global B^f
B^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t114, #4
str t114, [t100]
movw t115, #30
str t115, [t100, #8]
movw t116, #100
str t116, [t100, #12]
ldr t10700, [t100]
add t10800, t100, #8
ldr t10900, [t10800]
add t11000, t10700, t10900
add t11100, t100, #12
ldr t11200, [t11100]
add t11300, t11000, t11200
mov r0, t11300
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
movw t145, #20
mov r0, t145
bl malloc
mov t10300, t0
adr t146, A^f
str t146, [t10300, #16]
mov t10001, t10300
movw t147, #20
mov r0, t147
bl malloc
mov t10400, t0
adr t148, B^f
str t148, [t10400, #16]
mov t10101, t10400
add t12000, t10001, #16
ldr t10700, [t12000]
mov r12, t10700
mov r0, t10001
blx r12
add t12100, t10101, #16
ldr t10900, [t12100]
mov r12, t10900
mov r0, t10101
blx r12
mov t12200, t10001
ldr t12300, [t12200]
ldr t12500, [t10001, #4]
add t12600, t12300, t12500
mov t12700, t10101
ldr t12800, [t12700]
add t12900, t12600, t12800
add t13000, t10101, #8
ldr t13100, [t13000]
add t13200, t12900, t13100
add t13300, t10101, #12
ldr t13400, [t13300]
add t11100, t13200, t13400
mov r0, t11100
bl putint
movw t149, #10
mov r0, t149
bl putch
mov t10002, t10101
mov t13500, t10002
ldr t13600, [t13500]
mov t13700, t10101
ldr t13800, [t13700]
add t13900, t13600, t13800
add t14000, t10101, #8
ldr t14100, [t14000]
add t14200, t13900, t14100
add t14300, t10101, #12
ldr t14400, [t14300]
add t11400, t14200, t14400
mov r0, t11400
bl putint
movw t150, #10
mov r0, t150
bl putch
movw t151, #1
mov r0, t151
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
movw t105, #1
str t105, [t100]
movw t106, #2
str t106, [t100, #4]
movw t107, #0
mov r0, t107
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

