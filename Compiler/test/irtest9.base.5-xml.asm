<ScheduleProgram program_last_label_num="100" program_last_temp_num="10600" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="100" last_temp_num="108" instruction_count="19">
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
        <Instruction index="4" kind="I_OPER" assem="mov `d0, #4">
            <Dst>
                <Temp index="0" num="10600" name="t10600"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="movw `d0, #4">
            <Dst>
                <Temp index="0" num="107" name="t107"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_OPER" assem="mul `d0, `s0, `s1">
            <Dst>
                <Temp index="0" num="10300" name="t10300"/>
            </Dst>
            <Src>
                <Temp index="0" num="10600" name="t10600"/>
                <Temp index="1" num="107" name="t107"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="10300" name="t10300"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="bl malloc">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10200" name="t10200"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="movw `d0, #3">
            <Dst>
                <Temp index="0" num="108" name="t108"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="str `d0, [`s0]">
            <Dst>
                <Temp index="0" num="108" name="t108"/>
            </Dst>
            <Src>
                <Temp index="0" num="10200" name="t10200"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10000" name="t10000"/>
            </Dst>
            <Src>
                <Temp index="0" num="10200" name="t10200"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="109" name="t109"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="109" name="t109"/>
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
