.global __$main__^main
__$main__^main:
L100:
push {r4-r10, fp, lr}
sub sp, sp, #4
add fp, sp, #36
mov t10000, #0
mov t10100, #0
ldr t10900, [t10000]
ldr t11100, [t10000, #4]
add t11200, t10900, t11100
ldr t11400, [t10100]
add t11500, t11200, t11400
add t11600, t10100, #8
ldr t11700, [t11600]
add t11800, t11500, t11700
add t11900, t10100, #12
ldr t12000, [t11900]
add t10300, t11800, t12000
mov r0, t10300
bl putint
mov t10001, t10100
ldr t12200, [t10001]
ldr t12400, [t10001, #4]
add t12500, t12200, t12400
ldr t12700, [t10100]
add t12800, t12500, t12700
add t12900, t10100, #8
ldr t13000, [t12900]
add t13100, t12800, t13000
add t13200, t10100, #12
ldr t13300, [t13200]
add t10500, t13100, t13300
mov r0, t10500
bl putint
movw t134, #0
mov r0, t134
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
ldr t10400, [t100]
add t10500, t100, #8
ldr t10600, [t10500]
add t10700, t10400, t10600
add t10800, t100, #12
ldr t10900, [t10800]
add t11000, t10700, t10900
mov r0, t11000
sub sp, fp, #36
add sp, sp, #4
pop {r4-r10, fp, lr}
bx lr

