.CODE

extern g_SampleMod_ReturnAddr1:QWORD
extern shouldSampleModWork:BYTE
SampleModDetour1 PROC
        cmp byte ptr [shouldSampleModWork], 1
        je ret1
        inc byte ptr [rcx+rax*4+0001612Ch]
    ret1:
        jmp g_SampleMod_ReturnAddr1
SampleModDetour1 ENDP

extern g_SampleMod_ReturnAddr2:QWORD
SampleModDetour2 PROC
    jmp g_SampleMod_ReturnAddr2
SampleModDetour2 ENDP

END