<ScheduleProgram program_last_label_num="100" program_last_temp_num="10200" function_count="2">
    <Function index="0" name="A^f" last_label_num="100" last_temp_num="102" instruction_count="11">
        <Instruction index="0" kind="I_LABEL" assem="L100:">
            <Label num="100" name="L100"/>
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
        <Instruction index="4" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="100" name="t100"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="103" name="t103"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="103" name="t103"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
    <Function index="1" name="__$main__^main" last_label_num="100" last_temp_num="109" instruction_count="19">
        <Instruction index="0" kind="I_LABEL" assem="L100:">
            <Label num="100" name="L100"/>
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
        <Instruction index="4" kind="I_OPER" assem="movw `d0, #8">
            <Dst>
                <Temp index="0" num="107" name="t107"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="107" name="t107"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="bl malloc">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10200" name="t10200"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="adr `d0, A^f">
            <Dst>
                <Temp index="0" num="108" name="t108"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="str `d0, [`s0, #4]">
            <Dst>
                <Temp index="0" num="108" name="t108"/>
            </Dst>
            <Src>
                <Temp index="0" num="10200" name="t10200"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10001" name="t10001"/>
            </Dst>
            <Src>
                <Temp index="0" num="10200" name="t10200"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="109" name="t109"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_OPER" assem="str `d0, [`s0]">
            <Dst>
                <Temp index="0" num="109" name="t109"/>
            </Dst>
            <Src>
                <Temp index="0" num="10001" name="t10001"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="110" name="t110"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="110" name="t110"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
