#pragma once

#include "pch.h"
#include <iostream>
#include <string>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <iterator>
#include <string.h>
using namespace std;

#define _CRT_SECURE_NO_DEPRICATE
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>







namespace Memory {

    bool Compare(const BYTE* pData, const BYTE* bMask, const char* szMask)
    {
        for (; *szMask; ++szMask, ++pData, ++bMask)
            if (*szMask == 'x' && *pData != *bMask) return 0;
        return (*szMask) == NULL;
    }

    DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE* bMask, char* szMask)
    {
        for (int i = 0; i < (int)dwLen; i++)
            if (Compare((BYTE*)(dwAddress + (int)i), bMask, szMask))  return (int)(dwAddress + i);
        return 0;
    }

    int Scan(DWORD mode, char* content, char* mask)
    {
        DWORD PageSize;
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        PageSize = si.dwPageSize;
        MEMORY_BASIC_INFORMATION mi;
        for (DWORD lpAddr = 0; lpAddr < 0x7FFFFFFF; lpAddr += PageSize)
        {
            DWORD vq = VirtualQuery((void*)lpAddr, &mi, PageSize);
            if (vq == ERROR_INVALID_PARAMETER || vq == 0) break;
            if (mi.Type == MEM_MAPPED) continue;
            if (mi.Protect == mode)
            {
                int addr = FindPattern(lpAddr, PageSize, (PBYTE)content, mask);
                if (addr != 0)
                {
                    return addr;
                }
            }
        }
    }
}




DWORD rbx_L;

DWORD ScriptContext;

#define SetThreadIdentity(rbx_L, Context) *(char*)(rbx_L - 14) = Context 
#define aslr(x)(x - 0x400000 + (DWORD)GetModuleHandleA(0))

#define get_vm_key(rbx_L) decrypt_ptr((int)decrypt_ptr((int)(rbx_L + 8)) + 28) //ckey



int retflag1 = aslr(0x273A8B0);
int retflag2 = aslr(0x273A8A8);

int savedret1, savedret2;






#define set_arg(i,o,n,c)        ((i) = (((i) & MASK0(n, c) | \
                                (((Instruction)o << c) & MASK1(n, c)))))

#define bit(n, i) ((n & (1 << i)) ? 1 : 0)



int tzcnt(int num) {   //broken as of last roblox update
    int streak = 0;
    for (int i = 0; i < 32; i++) {
        if (!(num & (1 << i))) streak++;
        else return streak;
    }
    return streak;
}





#define SCRIPTCONTEXT 0x011AD1EC
#define VM_HACKFLAG 0x1566B98
#define DESERIALIZE_ADDRESS 0x404740
#define GETGLOBALSTATE_ADDRESS 0x64D550
#define SPAWN_ADDRESS 0x657EB0
#define NEWTHREAD_ADDRESS 0x492210




namespace func {
    int decrypt_ptr(int ptr) {
        return *(int*)(ptr)+(ptr);
    }

    int getParent(DWORD Instance) {
        __asm {
            mov ecx, Instance
            mov eax, [ecx + 0x34]
        }
    }

    const char* getClassName(DWORD Instance) {
        __asm {
            mov ecx, Instance
            mov eax, [ecx]
            call dword ptr[eax + 0x10]
        }
    }

    std::string getName(DWORD Instance) {
        std::string* Disgusting;
        __asm {
            mov ecx, Instance
            mov eax, [ecx + 0x28]
            mov Disgusting, eax
        }
        return Disgusting->c_str();
    }

    DWORD getGlobalState(DWORD* ScriptContext, DWORD Index) {
        DWORD* SC = ScriptContext;
        DWORD indx = Index;

        return SC[14 * indx + 41] ^ (DWORD)&SC[14 * indx + 41];
    }

    std::string getInstanceName(DWORD Instance) {
        std::string* Disgusting;
        __asm {
            mov ecx, Instance
            mov eax, [ecx + 0x28]
            mov Disgusting, eax
        }
        return Disgusting->c_str();
    }
}










void Console(const char* N) {
    AllocConsole();
    SetConsoleTitleA(N);
    freopen("CONOUT$", "w", stdout);
    freopen("CONIN$", "r", stdin);

}




int EncryptOpcode(int op, int key) {
    int a = key;
    int b = op;

    int x = (a >> tzcnt(a));
    int y = (x * x) + x - 1;
    int t = y * x;
    y *= 2 - t;
    t = y * x;
    y *= 2 - t;
    t = y * x;
    y *= 2 - t;
    return y * (b >> tzcnt(a));
}


namespace addresses {
    int getGlobalState = aslr(0x1);
    int lua_error = aslr(0x018E72E0);
}

namespace bypasses {
    void saveRetcheck() {
        savedret1 = *(DWORD*)&retflag1;
        savedret2 = *(DWORD*)&retflag2;
    }

    void overrideRetcheck() {
        *(DWORD*)retflag1 = *(DWORD*)savedret1;
        *(DWORD*)retflag2 = *(DWORD*)savedret2;
    }

}


typedef DWORD(__thiscall* clua_getglobalstate)(DWORD, DWORD);
clua_getglobalstate GetGlobalState = (clua_getglobalstate)addresses::getGlobalState; //removed in last update?? 


//use scriptcontext offset to find lua_state.

typedef int(__cdecl* clua_gettop)(int a1);
clua_gettop gettop = (clua_gettop)aslr(0x017258F0); //works


typedef int(__cdecl* lua_print)(int, const char*);
lua_print printx = (lua_print)aslr(0x65D8E0); //works

typedef int(__cdecl* RPrint)(int, const char*, ...);
RPrint r_Print = (RPrint)aslr(0x65D8E0);


int main() {

    Console("ScriptWare");

    printf("Saving ReturnCheck Flags... ");

    //savedret1 = *(DWORD*)&retflag1; //retcheck flags sanity check
    //savedret2 = *(DWORD*)&retflag2;
    //bypasses::saveRetcheck(); //save retcheck flag value (dynamic each game)
    r_Print(1, "Hello, world!");
    //cout << "Ok. Retflag 1: " << savedret1 << ", Retflag 2: " << savedret2 << endl;

    cout << "Testing Retcheck spoofing...";

    //bypasses::overrideRetcheck();

    cout << " Done." << endl << endl;

    cout << "Testing lua C functions...";

    //cout << gettop(0x014058F0) << endl;

    cout << "Done. Finding lua_state... ";
    //gettop(10000);




    //bypasses::overrideRetcheck();
    //cout << *(DWORD*)retflag1 << endl << *(DWORD*)retflag2 << endl;

    return 1;
}








BOOL APIENTRY DllMain(HMODULE hModule,
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

