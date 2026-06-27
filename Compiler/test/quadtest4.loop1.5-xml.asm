<ScheduleProgram program_last_label_num="100107" program_last_temp_num="10003" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="105" last_temp_num="105" instruction_count="30">
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
        <Instruction index="4" kind="I_OPER" assem="mov `d0, #10">
            <Dst>
                <Temp index="0" num="10000" name="t10000"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="sub `d0, `s0, #1">
            <Dst>
                <Temp index="0" num="10003" name="t10003"/>
            </Dst>
            <Src>
                <Temp index="0" num="10002" name="t10002"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10001" name="t10001"/>
            </Dst>
            <Src>
                <Temp index="0" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_LABEL" assem="L102:">
            <Label num="102" name="L102"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_LABEL" assem="L100105:">
            <Label num="100105" name="L100105"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="106" name="t106"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="10001" name="t10001"/>
                <Temp index="1" num="106" name="t106"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="bne `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="103" name="L103"/>
            </Jumps>
        </Instruction>
        <Instruction index="12" kind="I_LABEL" assem="L104:">
            <Label num="104" name="L104"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10002" name="t10002"/>
            </Dst>
            <Src>
                <Temp index="0" num="10001" name="t10001"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_LABEL" assem="L100106:">
            <Label num="100106" name="L100106"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10002" name="t10002"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_LABEL" assem="L103:">
            <Label num="103" name="L103"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10002" name="t10002"/>
            </Dst>
            <Src>
                <Temp index="0" num="10001" name="t10001"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_LABEL" assem="L100107:">
            <Label num="100107" name="L100107"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10002" name="t10002"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="24" kind="I_OPER" assem="bl putint">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="25" kind="I_OPER" assem="movw `d0, #10">
            <Dst>
                <Temp index="0" num="105" name="t105"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="26" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="105" name="t105"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="27" kind="I_OPER" assem="bl putch">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="28" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10001" name="t10001"/>
            </Dst>
            <Src>
                <Temp index="0" num="10003" name="t10003"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="29" kind="I_OPER" assem="b `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="100105" name="L100105"/>
            </Jumps>
        </Instruction>
    </Function>
</ScheduleProgram>
