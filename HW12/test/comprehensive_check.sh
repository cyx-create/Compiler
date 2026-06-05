#!/bin/bash
set -u
cd "$(dirname "$0")"
LIB="../vendor/libsysy/libsysy32.s"
GCC="arm-linux-gnueabihf-gcc -mcpu=cortex-a72 -Wall -static -Wl,-z,noexecstack -lm"

PASS=0
FAIL=0
CRASH=0
TIMEOUT=0
COMPILE_FAIL=0

run_one() {
    local name=$1 kin=$2 input=$3 expected=$4 label=$5
    local src="k${kin}/${name}.colored.s"
    local bin="/tmp/hw12_${name}_k${kin}_$$"

    if [ ! -f "$src" ]; then
        echo "MISSING $label"
        FAIL=$((FAIL + 1))
        return
    fi

    if ! $GCC -o "$bin" "$src" "$LIB" 2>/dev/null; then
        echo "COMPILE_FAIL $label"
        COMPILE_FAIL=$((COMPILE_FAIL + 1))
        return
    fi

    local out ec
    if [ -n "$input" ]; then
        out=$(printf '%s' "$input" | timeout 4 qemu-arm "$bin" 2>/dev/null | tr -d '\r')
    else
        out=$(timeout 4 qemu-arm "$bin" 2>/dev/null | tr -d '\r')
    fi
    ec=$?
    rm -f "$bin"

    if [ $ec -eq 124 ]; then
        echo "TIMEOUT $label"
        TIMEOUT=$((TIMEOUT + 1))
        return
    fi
    if [ $ec -eq 139 ] || [ $ec -eq 134 ]; then
        echo "CRASH $label"
        CRASH=$((CRASH + 1))
        return
    fi

    if [ "$out" = "$expected" ]; then
        echo "OK $label"
        PASS=$((PASS + 1))
    else
        echo "FAIL $label"
        echo "  expected=[$expected]"
        echo "  got=[$out]"
        FAIL=$((FAIL + 1))
    fi
}

run_one_expect_timeout() {
    local name=$1 kin=$2 input=$3 label=$4
    local src="k${kin}/${name}.colored.s"
    local bin="/tmp/hw12_to_${name}_k${kin}_$$"

    if [ ! -f "$src" ]; then
        echo "MISSING $label"
        FAIL=$((FAIL + 1))
        return
    fi

    if ! $GCC -o "$bin" "$src" "$LIB" 2>/dev/null; then
        echo "COMPILE_FAIL $label"
        COMPILE_FAIL=$((COMPILE_FAIL + 1))
        return
    fi

    local ec
    if [ -n "$input" ]; then
        printf '%s' "$input" | timeout 2 qemu-arm "$bin" >/dev/null 2>&1
    else
        timeout 2 qemu-arm "$bin" >/dev/null 2>&1
    fi
    ec=$?
    rm -f "$bin"

    if [ $ec -eq 124 ]; then
        echo "OK $label (timeout as expected)"
        PASS=$((PASS + 1))
    elif [ $ec -eq 139 ] || [ $ec -eq 134 ]; then
        echo "CRASH $label"
        CRASH=$((CRASH + 1))
    else
        echo "FAIL $label (expected timeout, ec=$ec)"
        FAIL=$((FAIL + 1))
    fi
}

run_stress_k2() {
    local name=$1 expected=$2 rounds=$3 label=$4
    local src="k2/${name}.colored.s"
    local bin="/tmp/hw12_stress_${name}_$$"

    if [ ! -f "$src" ]; then
        echo "MISSING $label"
        FAIL=$((FAIL + 1))
        return
    fi

    if ! $GCC -o "$bin" "$src" "$LIB" 2>/dev/null; then
        echo "COMPILE_FAIL $label"
        COMPILE_FAIL=$((COMPILE_FAIL + 1))
        return
    fi

    local i out ec
    for i in $(seq 1 "$rounds"); do
        out=$(timeout 4 qemu-arm "$bin" 2>/dev/null | tr -d '\r')
        ec=$?
        if [ $ec -eq 124 ]; then
            echo "TIMEOUT $label round=$i"
            TIMEOUT=$((TIMEOUT + 1))
            rm -f "$bin"
            return
        fi
        if [ $ec -eq 139 ] || [ $ec -eq 134 ]; then
            echo "CRASH $label round=$i"
            CRASH=$((CRASH + 1))
            rm -f "$bin"
            return
        fi
        if [ "$out" != "$expected" ]; then
            echo "FAIL $label round=$i"
            echo "  expected=[$expected]"
            echo "  got=[$out]"
            FAIL=$((FAIL + 1))
            rm -f "$bin"
            return
        fi
    done
    rm -f "$bin"
    echo "OK $label x${rounds}"
    PASS=$((PASS + 1))
}

check_k_consistency() {
    local name=$1 input=$2 label=$3
    local outs=()
    for kin in 2 5 9; do
        local bin="/tmp/hw12_consist_$$_${kin}"
        if ! $GCC -o "$bin" "k${kin}/${name}.colored.s" "$LIB" 2>/dev/null; then
            echo "CONSIST_COMPILE_FAIL $label k=$kin"
            FAIL=$((FAIL + 1))
            return
        fi
        local o
        if [ -n "$input" ]; then
            o=$(printf '%s' "$input" | timeout 4 qemu-arm "$bin" 2>/dev/null | tr -d '\r')
        else
            o=$(timeout 4 qemu-arm "$bin" 2>/dev/null | tr -d '\r')
        fi
        rm -f "$bin"
        outs+=("$o")
    done
    if [ "${outs[0]}" = "${outs[1]}" ] && [ "${outs[1]}" = "${outs[2]}" ]; then
        echo "OK CONSIST $label"
        PASS=$((PASS + 1))
    else
        echo "FAIL CONSIST $label"
        echo "  k2=[${outs[0]}]"
        echo "  k5=[${outs[1]}]"
        echo "  k9=[${outs[2]}]"
        FAIL=$((FAIL + 1))
    fi
}

echo "========== 1. 编译检查：39 个 colored.s =========="
for kin in 2 5 9; do
    for src in k${kin}/*.colored.s; do
        name=$(basename "$src" .colored.s)
        bin="/tmp/hw12_compile_$$"
        if $GCC -o "$bin" "$src" "$LIB" 2>/dev/null; then
            echo "COMPILE_OK ${name} k=$kin"
            PASS=$((PASS + 1))
            rm -f "$bin"
        else
            echo "COMPILE_FAIL ${name} k=$kin"
            COMPILE_FAIL=$((COMPILE_FAIL + 1))
        fi
    done
done

echo ""
echo "========== 2. 功能测试：无输入程序 =========="
for kin in 2 5 9; do
    run_one insttest0 $kin "" "200" "insttest0 k=$kin"
    run_one insttest1 $kin "" "4 3 2 1 " "insttest1 k=$kin"
    run_one insttest2 $kin "" "3 2 1 " "insttest2 k=$kin"
    run_one insttest3 $kin "" "3 2 1 " "insttest3 k=$kin"
    run_one bubblesort $kin "" "0 1 2 3 5 6 9 " "bubblesort k=$kin"
    run_one optloopivtest4 $kin "" "34 30 26 22 18 14 10 6 2 -2 0" "optloopivtest4 k=$kin"
done

echo ""
echo "========== 3. 功能测试：有输入程序 =========="
for kin in 2 5 9; do
    run_one insttest4 $kin $'1\n' "4 3 2 1 " "insttest4(+1) k=$kin"
    run_one insttest4 $kin $'0\n' "9 8 7 6 5 " "insttest4(0) k=$kin"
    run_one insttest4 $kin $'-1\n' "9 8 7 6 5 " "insttest4(-1) k=$kin"

    run_one fibonacci $kin $'0\n' "Enter the number of term:" "fibonacci(0) k=$kin"
    run_one fibonacci $kin $'1\n' "Enter the number of term:0 " "fibonacci(1) k=$kin"
    run_one fibonacci $kin $'2\n' "Enter the number of term:0 1 " "fibonacci(2) k=$kin"
    run_one fibonacci $kin $'5\n' "Enter the number of term:0 1 1 2 3 " "fibonacci(5) k=$kin"
    run_one fibonacci $kin $'9\n' "Enter the number of term:0 1 1 2 3 5 8 13 21 " "fibonacci(9) k=$kin"

    run_one optloopivtest1 $kin $'1\n' "2 " "optloopivtest1(1) k=$kin"
    run_one optloopivtest1 $kin $'3\n' "10 6 2 " "optloopivtest1(3) k=$kin"
    run_one optloopivtest1 $kin $'4\n' "14 10 6 2 " "optloopivtest1(4) k=$kin"
    run_one optloopivtest1 $kin $'5\n' "18 14 10 6 2 " "optloopivtest1(5) k=$kin"

    run_one optloopivtest2 $kin $'1\n' "6 -1" "optloopivtest2(1) k=$kin"
    run_one optloopivtest2 $kin $'4\n' "18 10 0" "optloopivtest2(4) k=$kin"
    run_one optloopivtest2 $kin $'5\n' "22 14 6 -1" "optloopivtest2(5) k=$kin"

    run_one optloopivtest3 $kin $'10\n3\n' "63 39 15 -9 " "optloopivtest3(10,3) k=$kin"
    run_one optloopivtest3 $kin $'9\n2\n' "63 47 31 15 -1 " "optloopivtest3(9,2) k=$kin"
    run_one optloopivtest3 $kin $'1\n2\n' "-1 " "optloopivtest3(1,2) k=$kin"

    run_one optloopivtest5 $kin $'1\n' "32 29 26 23 20 17 14 11 8 5 5" "optloopivtest5(+1) k=$kin"
    run_one optloopivtest5 $kin $'0\n' "4" "optloopivtest5(0) k=$kin"
    run_one optloopivtest5 $kin $'-1\n' "4" "optloopivtest5(-1) k=$kin"

    run_one optloopivtest6 $kin $'1\n' "32 29 26 23 20 17 14 11 8 5 5" "optloopivtest6(+1) k=$kin"
    run_one optloopivtest6 $kin $'0\n' "4" "optloopivtest6(0) k=$kin"
    run_one optloopivtest6 $kin $'-1\n' "4" "optloopivtest6(-1) k=$kin"
done

echo ""
echo "========== 4. k 一致性（同输入 k2/k5/k9 输出相同）=========="
check_k_consistency insttest0 "" "insttest0"
check_k_consistency bubblesort "" "bubblesort"
check_k_consistency optloopivtest4 "" "optloopivtest4"
check_k_consistency insttest4 $'1\n' "insttest4(+1)"
check_k_consistency fibonacci $'5\n' "fibonacci(5)"
check_k_consistency optloopivtest3 $'10\n3\n' "optloopivtest3(10,3)"
check_k_consistency optloopivtest5 $'1\n' "optloopivtest5(+1)"
check_k_consistency optloopivtest6 $'1\n' "optloopivtest6(+1)"
check_k_consistency optloopivtest1 $'4\n' "optloopivtest1(4)"

echo ""
echo "========== 5. 边界用例：fibonacci =========="
for kin in 2 5 9; do
    # n=0：循环 0 次，仅提示（已在第 3 节覆盖，此处强调边界）
    run_one fibonacci $kin $'0\n' "Enter the number of term:" "fibonacci(0) boundary k=$kin"
    # n<0：直接 return -1
    run_one fibonacci $kin $'-1\n' "Enter the number of term:" "fibonacci(-1) k=$kin"
    # n>47：直接 return -1
    run_one fibonacci $kin $'48\n' "Enter the number of term:" "fibonacci(48) k=$kin"
    # 小规模与中等规模
    run_one fibonacci $kin $'3\n' "Enter the number of term:0 1 1 " "fibonacci(3) k=$kin"
    run_one fibonacci $kin $'10\n' "Enter the number of term:0 1 1 2 3 5 8 13 21 34 " "fibonacci(10) k=$kin"
done

echo ""
echo "========== 6. 边界用例：optloopivtest1/2/3 =========="
for kin in 2 5 9; do
    # optloopivtest1：k=0 不进入循环
    run_one optloopivtest1 $kin $'0\n' "" "optloopivtest1(0) k=$kin"
    # optloopivtest2：k=0 直接 putint(0)
    run_one optloopivtest2 $kin $'0\n' "0" "optloopivtest2(0) k=$kin"

    # optloopivtest3：k=0 不进入循环
    run_one optloopivtest3 $kin $'0\n1\n' "" "optloopivtest3(0,1) k=$kin"
    # 单步递减
    run_one optloopivtest3 $kin $'1\n1\n' "7 " "optloopivtest3(1,1) k=$kin"
    # 大步长一次变负
    run_one optloopivtest3 $kin $'2\n3\n' "-1 " "optloopivtest3(2,3) k=$kin"
    run_one optloopivtest3 $kin $'1\n10\n' "-65 " "optloopivtest3(1,10) k=$kin"
    # 等步长归零
    run_one optloopivtest3 $kin $'5\n5\n' "7 " "optloopivtest3(5,5) k=$kin"
    # 较长循环（s=1）
    run_one optloopivtest3 $kin $'10\n1\n' "79 71 63 55 47 39 31 23 15 7 " "optloopivtest3(10,1) k=$kin"
    # 大数两步结束
    run_one optloopivtest3 $kin $'100\n50\n' "407 7 " "optloopivtest3(100,50) k=$kin"
done

echo ""
echo "========== 7. 死循环检测：optloopivtest3(10,0) =========="
for kin in 2 5 9; do
    run_one_expect_timeout optloopivtest3 $kin $'10\n0\n' "optloopivtest3(10,0) k=$kin"
done

echo ""
echo "========== 8. k=2 压力测试：insttest1～3 连跑 10 次 =========="
run_stress_k2 insttest1 "4 3 2 1 " 10 "insttest1 k=2 stress"
run_stress_k2 insttest2 "3 2 1 " 10 "insttest2 k=2 stress"
run_stress_k2 insttest3 "3 2 1 " 10 "insttest3 k=2 stress"

echo ""
echo "========== 9. 边界 k 一致性 =========="
check_k_consistency fibonacci $'0\n' "fibonacci(0)"
check_k_consistency fibonacci $'-1\n' "fibonacci(-1)"
check_k_consistency fibonacci $'10\n' "fibonacci(10)"
check_k_consistency optloopivtest3 $'0\n1\n' "optloopivtest3(0,1)"
check_k_consistency optloopivtest3 $'10\n1\n' "optloopivtest3(10,1)"
check_k_consistency optloopivtest3 $'100\n50\n' "optloopivtest3(100,50)"
check_k_consistency optloopivtest1 $'0\n' "optloopivtest1(0)"
check_k_consistency optloopivtest2 $'0\n' "optloopivtest2(0)"

echo ""
echo "========== SUMMARY =========="
TOTAL=$((PASS + FAIL + CRASH + TIMEOUT + COMPILE_FAIL))
echo "PASS=$PASS FAIL=$FAIL CRASH=$CRASH TIMEOUT=$TIMEOUT COMPILE_FAIL=$COMPILE_FAIL TOTAL=$TOTAL"
if [ $FAIL -eq 0 ] && [ $CRASH -eq 0 ] && [ $TIMEOUT -eq 0 ] && [ $COMPILE_FAIL -eq 0 ]; then
    echo "VERDICT: ALL_PASSED"
    exit 0
else
    echo "VERDICT: HAS_FAILURES"
    exit 1
fi
