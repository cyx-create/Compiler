<ScheduleProgram program_last_label_num="100" program_last_temp_num="10000" function_count="1">
    <Function index="0" name="__$main__^main" last_label_num="100" last_temp_num="113" instruction_count="22">
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
        <Instruction index="4" kind="I_OPER" assem="movw `d0, #16">
            <Dst>
                <Temp index="0" num="109" name="t109"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="5" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="109" name="t109"/>
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
                <Temp index="0" num="10000" name="t10000"/>
            </Dst>
            <Src>
                <Temp index="0" num="0" name="t0"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="8" kind="I_OPER" assem="movw `d0, #3">
            <Dst>
                <Temp index="0" num="110" name="t110"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="9" kind="I_OPER" assem="str `d0, [`s0]">
            <Dst>
                <Temp index="0" num="110" name="t110"/>
            </Dst>
            <Src>
                <Temp index="0" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="10" kind="I_OPER" assem="movw `d0, #1">
            <Dst>
                <Temp index="0" num="111" name="t111"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="11" kind="I_OPER" assem="str `d0, [`s0, #4]">
            <Dst>
                <Temp index="0" num="111" name="t111"/>
            </Dst>
            <Src>
                <Temp index="0" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="12" kind="I_OPER" assem="movw `d0, #2">
            <Dst>
                <Temp index="0" num="112" name="t112"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="13" kind="I_OPER" assem="str `d0, [`s0, #8]">
            <Dst>
                <Temp index="0" num="112" name="t112"/>
            </Dst>
            <Src>
                <Temp index="0" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="14" kind="I_OPER" assem="movw `d0, #3">
            <Dst>
                <Temp index="0" num="113" name="t113"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="15" kind="I_OPER" assem="str `d0, [`s0, #12]">
            <Dst>
                <Temp index="0" num="113" name="t113"/>
            </Dst>
            <Src>
                <Temp index="0" num="10000" name="t10000"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="16" kind="I_OPER" assem="movw `d0, #0">
            <Dst>
                <Temp index="0" num="114" name="t114"/>
            </Dst>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="17" kind="I_OPER" assem="mov r0, `s0">
            <Dst/>
            <Src>
                <Temp index="0" num="114" name="t114"/>
            </Src>
            <Jumps/>
        </Instruction>
        <Instruction index="18" kind="I_OPER" assem="sub sp, fp, #36">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="19" kind="I_OPER" assem="add sp, sp, #4">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="20" kind="I_OPER" assem="pop {r4-r10, fp, lr}">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
        <Instruction index="21" kind="I_OPER" assem="bx lr">
            <Dst/>
            <Src/>
            <Jumps/>
        </Instruction>
    </Function>
</ScheduleProgram>
