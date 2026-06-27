.global A^f
A^f:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t100, t0
movw t107, #1
str t107, [t100]
movw t108, #2
str t108, [t100, #4]
movw t109, #0
mov r0, t109
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
movw t116, #4
str t116, [t100]
movw t117, #30
str t117, [t100, #8]
movw t118, #100
str t118, [t100, #12]
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
movw t147, #20
mov r0, t147
bl malloc
mov t10300, t0
adr t148, A^f
str t148, [t10300, #16]
mov t10001, t10300
movw t149, #20
mov r0, t149
bl malloc
mov t10400, t0
adr t150, B^f
str t150, [t10400, #16]
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
movw t151, #10
mov r0, t151
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
movw t152, #10
mov r0, t152
bl putch
movw t153, #1
mov r0, t153
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

