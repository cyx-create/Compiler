<ScheduleProgram program_last_label_num="103" program_last_temp_num="10500" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="103" last_temp_num="111" instruction_count="11">
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
        <Instruction index="4" kind="I_MOVE" assem="mov `d0, `s0">
            <Dst>
                <Temp index="0" num="10500" name="t10500"/>
            </Dst>
            <Src>
                <Temp index="0" num="100" name="t100"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="ldr `d0, [`s0]">
            <Dst>
                <Temp index="0" num="10300" name="t10300"/>
            </Dst>
            <Src>
                <Temp index="0" num="100" name="t100"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="6" kind="I_LABEL" assem="L100:">
            <Label num="100" name="L100"/>
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="7" kind="I_OPER" assem="movw `d0, #65535">
            <Dst>
                <Temp index="0" num="112" name="t112"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="movt `d0, #65535">
            <Dst>
                <Temp index="0" num="112" name="t112"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="112" name="t112"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="bl exit">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
