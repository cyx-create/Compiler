<ScheduleProgram program_last_label_num="103" program_last_temp_num="10800" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="103" last_temp_num="113" instruction_count="32">
        <Instruction index="0" kind="I_LABEL" assem="L103:">
            <Label num="103" name="L103"/>
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
        <Instruction index="4" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="110" name="t110"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="movw `d0, #123">
            <Dst>
                <Temp index="0" num="111" name="t111"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="sub `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10200" name="t10200"/>
            </Dst>
            <Src>
                <Temp index="0" num="110" name="t110"/>
                <Temp index="1" num="111" name="t111"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10500" name="t10500"/>
            </Dst>
            <Src>
                <Temp index="0" num="100" name="t100"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="10300" name="t10300"/>
            </Dst>
            <Src>
                <Temp index="0" num="100" name="t100"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="114" name="t114"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="10200" name="t10200"/>
                <Temp index="1" num="114" name="t114"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="bge `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="101" name="L101"/>
            </Jumps>
        </Instruction>
        <Instruction index="12" kind="I_LABEL" assem="L100:">
            <Label num="100" name="L100"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="movw `d0, #65535">
            <Dst>
                <Temp index="0" num="115" name="t115"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="movt `d0, #65535">
            <Dst>
                <Temp index="0" num="115" name="t115"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="115" name="t115"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_OPER" assem="bl exit">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_LABEL" assem="L101:">
            <Label num="101" name="L101"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="10200" name="t10200"/>
                <Temp index="1" num="10300" name="t10300"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="bge `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="100" name="L100"/>
            </Jumps>
        </Instruction>
        <Instruction index="20" kind="I_LABEL" assem="L102:">
            <Label num="102" name="L102"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_OPER" assem="add `d0, `s0, #1">
            <Dst>
                <Temp index="0" num="10700" name="t10700"/>
            </Dst>
            <Src>
                <Temp index="0" num="10200" name="t10200"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_OPER" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="112" name="t112"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10800" name="t10800"/>
            </Dst>
            <Src>
                <Temp index="0" num="10700" name="t10700"/>
                <Temp index="1" num="112" name="t112"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="24" kind="I_OPER" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="113" name="t113"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="25" kind="I_OPER" assem="str `d0, [`s0, `s1]">
            <Dst>
                <Temp index="0" num="113" name="t113"/>
            </Dst>
            <Src>
                <Temp index="0" num="10500" name="t10500"/>
                <Temp index="1" num="10800" name="t10800"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="26" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="116" name="t116"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="27" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="116" name="t116"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="28" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="29" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="30" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="31" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
