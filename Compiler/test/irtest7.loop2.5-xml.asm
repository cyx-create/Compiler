<ScheduleProgram program_last_label_num="100111" program_last_temp_num="10700" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="108" last_temp_num="107" instruction_count="35">
        <Instruction index="0" kind="I_LABEL" assem="L108:">
            <Label num="108" name="L108"/>
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
        <Instruction index="4" kind="I_OPER" assem="mov `d0, #1">
            <Dst>
                <Temp index="0" num="10000" name="t10000"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="mov `d0, #2">
            <Dst>
                <Temp index="0" num="10100" name="t10100"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="bl getint">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10400" name="t10400"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10500" name="t10500"/>
            </Dst>
            <Src>
                <Temp index="0" num="10400" name="t10400"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="mov `d0, #0">
            <Dst>
                <Temp index="0" num="10300" name="t10300"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="108" name="t108"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="10000" name="t10000"/>
                <Temp index="1" num="108" name="t108"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_OPER" assem="bgt `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="100108" name="L100108"/>
            </Jumps>
        </Instruction>
        <Instruction index="13" kind="I_LABEL" assem="L104:">
            <Label num="104" name="L104"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10301" name="t10301"/>
            </Dst>
            <Src>
                <Temp index="0" num="10300" name="t10300"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_LABEL" assem="L100109:">
            <Label num="100109" name="L100109"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_OPER" assem="movw `d0, #2">
            <Dst>
                <Temp index="0" num="109" name="t109"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="cmp `s0, `s1">
            <Dst/>
            <Src>
                <Temp index="0" num="10100" name="t10100"/>
                <Temp index="1" num="109" name="t109"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="blt `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="105" name="L105"/>
            </Jumps>
        </Instruction>
        <Instruction index="19" kind="I_LABEL" assem="L106:">
            <Label num="106" name="L106"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_LABEL" assem="L100110:">
            <Label num="100110" name="L100110"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_OPER" assem="add `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10700" name="t10700"/>
            </Dst>
            <Src>
                <Temp index="0" num="10500" name="t10500"/>
                <Temp index="1" num="10301" name="t10301"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="22" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10700" name="t10700"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="23" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="24" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="25" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="26" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="27" kind="I_LABEL" assem="L105:">
            <Label num="105" name="L105"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="28" kind="I_LABEL" assem="L100111:">
            <Label num="100111" name="L100111"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="29" kind="I_OPER" assem="mov `d0, #1">
            <Dst>
                <Temp index="0" num="10302" name="t10302"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="30" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10301" name="t10301"/>
            </Dst>
            <Src>
                <Temp index="0" num="10302" name="t10302"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="31" kind="I_OPER" assem="b `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="100110" name="L100110"/>
            </Jumps>
        </Instruction>
        <Instruction index="32" kind="I_LABEL" assem="L100108:">
            <Label num="100108" name="L100108"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="33" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10301" name="t10301"/>
            </Dst>
            <Src>
                <Temp index="0" num="10300" name="t10300"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="34" kind="I_OPER" assem="b `j0">
            <Dst/>
            <Src/>
            <Jumps>
                <Label index="0" num="100111" name="L100111"/>
            </Jumps>
        </Instruction>
    </Function>
</ScheduleProgram>
