<ScheduleProgram program_last_label_num="105" program_last_temp_num="10400" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="105" last_temp_num="122" instruction_count="40">
        <Instruction index="0" kind="I_LABEL" assem="L105:">
            <Label num="105" name="L105"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="1" kind="I_OPER" assem="push {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="2" kind="I_OPER" assem="sub sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="3" kind="I_OPER" assem="add fp, sp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="4" kind="I_OPER" assem="bl getint">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="10300" name="t10300"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="mov `d0, #0">
            <Dst>
                <Temp index="0" num="10200" name="t10200"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="bl getint">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="mov `d0, r0">
            <Dst>
                <Temp index="0" num="10400" name="t10400"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="sub `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10206" name="t10206"/>
            </Dst>
            <Src>
                <Temp index="0" num="10300" name="t10300"/>
                <Temp index="1" num="10400" name="t10400"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="movw `d0, #8">
            <Dst>
                <Temp index="0" num="119" name="t119"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10207" name="t10207"/>
            </Dst>
            <Src>
                <Temp index="0" num="10206" name="t10206"/>
                <Temp index="1" num="119" name="t119"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_OPER" assem="add `d0, `s0, #7">
            <Dst>
                <Temp index="0" num="10204" name="t10204"/>
            </Dst>
            <Src>
                <Temp index="0" num="10207" name="t10207"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="movw `d0, #8">
            <Dst>
                <Temp index="0" num="120" name="t120"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10208" name="t10208"/>
            </Dst>
            <Src>
                <Temp index="0" num="10400" name="t10400"/>
                <Temp index="1" num="120" name="t120"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10201" name="t10201"/>
            </Dst>
            <Src>
                <Temp index="0" num="10200" name="t10200"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10203" name="t10203"/>
            </Dst>
            <Src>
                <Temp index="0" num="10204" name="t10204"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_LABEL" assem="L102:">
            <Label num="102" name="L102"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="movw `d0, #7">
            <Dst>
                <Temp index="0" num="123" name="t123"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="10203" name="t10203"/>
                <Temp index="1" num="123" name="t123"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_OPER" assem="bgt `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="103" name="L103"/>
            </Jumps>
        </Instruction>
        <Instruction index="21" kind="I_LABEL" assem="L104:">
            <Label num="104" name="L104"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_OPER" assem="movw `d0, #10">
            <Dst>
                <Temp index="0" num="122" name="t122"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="122" name="t122"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="24" kind="I_OPER" assem="bl putch">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="25" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10201" name="t10201"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="26" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="27" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="28" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="29" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="30" kind="I_LABEL" assem="L103:">
            <Label num="103" name="L103"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="31" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10203" name="t10203"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="32" kind="I_OPER" assem="bl putint">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="33" kind="I_OPER" assem="movw `d0, #32">
            <Dst>
                <Temp index="0" num="121" name="t121"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="34" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="121" name="t121"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="35" kind="I_OPER" assem="bl putch">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="36" kind="I_OPER" assem="sub `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10205" name="t10205"/>
            </Dst>
            <Src>
                <Temp index="0" num="10203" name="t10203"/>
                <Temp index="1" num="10208" name="t10208"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="37" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10201" name="t10201"/>
            </Dst>
            <Src>
                <Temp index="0" num="10203" name="t10203"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="38" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10203" name="t10203"/>
            </Dst>
            <Src>
                <Temp index="0" num="10205" name="t10205"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="39" kind="I_OPER" assem="b `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="102" name="L102"/>
            </Jumps>
        </Instruction>
    </Function>
</ScheduleProgram>
