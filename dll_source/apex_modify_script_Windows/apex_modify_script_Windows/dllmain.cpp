// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。

//=================================================
//Apex version v3.0.26.26
//=================================================

#include "pch.h"

#include <Windows.h>
#include <stdio.h>
#include <intrin.h>
#include <stdint.h>

void memcopy(BYTE* dst, BYTE* src, size_t size) {
    DWORD oldprotect;
    VirtualProtect(dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    memcpy(dst, src, size);
    VirtualProtect(dst, size, oldprotect, &oldprotect);
}

bool __fastcall MyLoadScript(void* sqvm, const char* szScriptPath, const char* szScriptName, int nFlag) {
    bool(__fastcall * OriginalLoadScript)(void* sqvm, const char* szScriptPath, const char* szScriptName, int nFlag);
    OriginalLoadScript = (bool(__fastcall*)(void*, const char*, const char*, int))((uintptr_t)GetModuleHandle(NULL) + (0x7FF726432180 - 0x7FF725850000));
    bool result;

    if (strcmp(szScriptName, "sh_character_select_new.gnut") == 0) {//<---addable
        char new_script_path[1024];
        memset(new_script_path, 0, sizeof(char) * 1024);
        sprintf_s(new_script_path, "modscript/%s", szScriptPath);

        result = OriginalLoadScript(sqvm, new_script_path, szScriptName, nFlag);

        //char str[1024];
        //sprintf_s(str, "szScriptName = %s\nszScriptPath = %s\n", szScriptName, szScriptPath);
        //MessageBox(NULL, str, "MyLoadScript", MB_OK);

    }
    else {
        result = OriginalLoadScript(sqvm, szScriptPath, szScriptName, nFlag);
    }

    return result;
}

void rewrite_relative_call_address(uintptr_t call_op_address, uintptr_t new_func_addr) {
    uintptr_t new_relative_addr = new_func_addr - call_op_address - 5;
    uint32_t new_relative_addr_32bit = (uint32_t)new_relative_addr;

    memcopy((BYTE*)(call_op_address + 1), (BYTE*)(&new_relative_addr_32bit), sizeof(uint32_t));

    //char str[1024 * 4];
    //sprintf_s(str, "call_op_address : %p\nnew_relative_addr : %p\nnew_relative_addr_32bit : %08x\nnew_func_addr : %p\n",
    //    call_op_address, new_relative_addr, new_relative_addr_32bit, new_func_addr);
    //MessageBox(NULL, str, "rewrite_relative_call_address", MB_OK);
}

void main()
{
    /*
        48 b8 [ef cd ab 89 67 45 23 01]    movabs rax,0x123456789abcdef
        ff e0                              jmp    rax
    */
    uintptr_t modify_addr = (uintptr_t)GetModuleHandle(NULL) + (0x14EBF7000 - 0x140000000);//(Read/Executable)address
    BYTE override_op_1[] = { 0x48, 0xB8, /**/0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01,/**/ 0xFF, 0xE0 };
    uintptr_t* func_addr = (uintptr_t*)(&(override_op_1[2]));
    *func_addr = (uintptr_t)MyLoadScript;
    memcopy((BYTE*)modify_addr, override_op_1, sizeof(BYTE) * 12);


    rewrite_relative_call_address((uintptr_t)GetModuleHandle(NULL) + (0x7FF726060FF1 - 0x7FF725850000), modify_addr/*(uintptr_t)MyLoadScript*/);//call LoadScript
    rewrite_relative_call_address((uintptr_t)GetModuleHandle(NULL) + (0x7FF72606106F - 0x7FF725850000), modify_addr/*(uintptr_t)MyLoadScript*/);//call LoadScript
    rewrite_relative_call_address((uintptr_t)GetModuleHandle(NULL) + (0x7FF726431BDA - 0x7FF725850000), modify_addr/*(uintptr_t)MyLoadScript*/);//call LoadScript
    rewrite_relative_call_address((uintptr_t)GetModuleHandle(NULL) + (0x7FF726431BFA - 0x7FF725850000), modify_addr/*(uintptr_t)MyLoadScript*/);//call LoadScript
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        main();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
