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





   
namespace memory
{
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

    DWORD scan(char* content)
    {
        SYSTEM_INFO SystemInfo;
        GetSystemInfo(&SystemInfo);
        DWORD PageSize = SystemInfo.dwPageSize;
        MEMORY_BASIC_INFORMATION meminfo;
        DWORD Start = (DWORD)SystemInfo.lpMinimumApplicationAddress;
        DWORD End = (DWORD)SystemInfo.lpMaximumApplicationAddress;
        for (DWORD lpAddress = Start; lpAddress <= End; lpAddress += PageSize) //tepigs scanner
        {
            VirtualQuery((void*)lpAddress, &meminfo, PageSize);
            if (meminfo.Type == MEM_MAPPED) continue;
            if (meminfo.Protect == PAGE_READWRITE) {
                DWORD Address = FindPattern(lpAddress, PageSize, (PBYTE)content, (char*)"xxxx");
                if (Address != NULL)
                    return Address;
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
clua_gettop gettop = (clua_gettop)aslr(0x017258F0); //outdated

typedef int(__cdecl* clua_settop)(int a1);
clua_settop settop = (clua_settop)aslr(0x11B4E30); //works

typedef int(__cdecl* lua_print)(int, const char*);
lua_print printx = (lua_print)aslr(0x65D8E0); //works

typedef int(__cdecl* RPrint)(int, const char*, ...);
RPrint r_Print = (RPrint)aslr(0x65D8E0);

typedef int(__stdcall* clua_getfield)(int, int, const char*);
clua_getfield getfield = (clua_getfield)aslr(0x10A4E30); //works



int lua_gettop(unsigned int rL)
{
    return (*(DWORD*)(rL + 24) - *(DWORD*)(rL + 12)) >> 4;
}




int calculate_size(int func_start, int end, int second) {
    bool keepgoing = true;  int max = 1000;  int amt = 0;
    if (end == 0) end = 0xC2;
    while (keepgoing) {

        amt = amt + 1;
        int i;

        if (((BYTE*)func_start)[0] == end) {

            printf("%d ", ((BYTE*)func_start)[0]);

            if (second != 0) {
                if (((BYTE*)func_start + 0x1)[0] == second) {

                    printf("%d\r\n\r\n", ((BYTE*)func_start)[1]);

                    printf("Found EOF (%d %d). (%d down from function start.) (0x%d)\r\n", end, second, amt, (func_start));
                    return func_start;
                    break;
                }
            }
            else {
                printf("\r\n\r\nFound EOF (%d). (%d down from function start.) (0x%d)\r\n", end, amt, (func_start));
                return func_start;
                break;
            }
        }

        else {
            printf("%d ", ((BYTE*)func_start)[0]);
        }
        if (amt > max) {
            printf("Size calculation failed - function too large.");
            break;
        }
        func_start = func_start + 0x1;
    }
}





void dump_hex(int func_start, int end) {
    int amt = 0;
    while (true) {
        amt += 1;
        int i;
        printf("%d", (func_start));
        cout << reinterpret_cast<int*>(*(DWORD*)func_start) << endl;
        if (amt >= end) {
            printf("Size calculation failed - function too large.");
            break;
        }
        func_start = func_start + 0x1;
    }
}




void pushstringcalled(int a, int b, int c) {
    printf(" Function called.");
}

void setcall(DWORD x, int d, char* opcode) {
    DWORD a, b;
    VirtualProtect((LPVOID)x, d, PAGE_EXECUTE_READWRITE, &a);

    cout << "asd";
    *(char*)x = *opcode;
    *(DWORD*)(x + 1) = (DWORD)((char*)pushstringcalled - (char*)(x + 5));
    VirtualProtect((LPVOID)x, d, a, &a);
}

char* copyopcode(int d) {
    return (char*)d;
}


string int2hex(int x) {
    std::stringstream stream;
    stream << std::hex << std::uppercase << x;
    std::string result(stream.str());
    return result;
}


int scanForBytes(int addy, int bytes[], int max) {
    //printf("%d", addy);
    int origaddy = addy;
    for (int a = 1; a < max; a++) {
        BYTE* add = (BYTE*)(((BYTE*)addy)[0]);
        int addx = (int)add;
     //   std::cout << "0x" << addy << ": " << "0x" << std::hex << std::uppercase << addx << '\n';
        
        if (addx == 0xC2) {
           // printf("FOUND END, WTF??? BROKEN GAME SOMETHING WENT WRONG");
            return 0;
            break;
        }
        if (addx == bytes[0]) {
            //std::cout << "found first byte: " << "0x" << std::hex << std::uppercase << addx << '\n';
            
            int tempaddy = addy;
            int startaddy = tempaddy;
            for (int b = 1; b <= sizeof(bytes); b++) {
                int comp = (int)(BYTE*)(((BYTE*)tempaddy+b)[0]);
               // cout << "addy is " << comp << endl;
                //cout << "comparing " << bytes[b] << " to " << comp << endl;
                if (comp == bytes[b]) {
                    //cout << "match found, passing on\n";
                }
                else
                {
                    //printf("broken silicon, moving on");
                    break;
                }
                if (b == sizeof(bytes)) {
                    std::stringstream stream;
                    int retaddy = tempaddy;
                    stream << std::hex << std::uppercase << (int)retaddy;
                    std::string result(stream.str());
                    //cout << "acc found at " << result << endl;
                    //cout << "diff is " << (retaddy - origaddy);
                    //cout << endl;
                    return retaddy;
                }
            }
        
        }

        addy += 1;
    }
    return 1;
}



bool confirmRetcheckExists(int addr) {
    int retbytes[] = { 0xE8, 0x31, 0xC0, 0x28, 0xFF };
    int end = scanForBytes(aslr(0x1434E30), retbytes, 1500);
    if (end > 1) {
        return true;
    }
    int antiretbytes[] = { 0x90, 0x31, 0xC0, 0x28, 0xFF };
    end = scanForBytes(aslr(0x1434E30), antiretbytes, 1500);
    if (end > 1) {
        return false;
    }
    cout << "CANNOT FIND EITHER, FUNCTION IS MISSING ENABLED/DISABLED RETCHECK." << endl;
    return false;

}

void setRetcheck(int addr) {
    int retbytes[] = { 0xE8, 0x31, 0xC0, 0x28, 0xFF };
    int end = scanForBytes(aslr(0x1434E30), retbytes, 1500);
    if (end == 0) {
        cout << "\ncannot set retcheck to unfound address (cant find call)" << endl;
    }
    else {
        cout << "\nsetting retcheck at " << int2hex(end) << endl;
        DWORD a, b;
        VirtualProtect((LPVOID)end, 1, PAGE_EXECUTE_READWRITE, &a);

        *(char*)end = 0x90;
        *(char*)(end+1) = 0x90;
        *(char*)(end+2) = 0x90;
        *(char*)(end+3) = 0x90;
        *(char*)(end+4) = 0x90; //work on this

        VirtualProtect((LPVOID)end, 1, a, &a);

        cout << "retcheck is: " << confirmRetcheckExists(addr) << endl;
    }
}

void restoreRetcheck(int addr) {
    int retbytes[] = { 0x90, 0x31, 0xC0, 0x28, 0xFF };
    int end = scanForBytes(aslr(0x1434E30), retbytes, 1500);
    if (end == 0) {
        cout << "\ncannot restore retcheck to unfound address (cant find call)" << endl;
    }
    else {
        cout << "\nrestoring retcheck at " << int2hex(end) << endl;
        DWORD a, b;
        VirtualProtect((LPVOID)end, 1, PAGE_EXECUTE_READWRITE, &a);
        *(char*)end = 0xE8;
        VirtualProtect((LPVOID)end, 1, a, &a);
        cout << "retcheck is: " << confirmRetcheckExists(addr) << endl;
    }
}

int main() {

    Console("BlackStar");


    int scriptc = aslr(0x01AC94C4);
    int scriptContext = memory::scan((char*)&scriptc);

    int state = scriptContext + 56 * 0 + 164 ^ *(DWORD*)(scriptContext + 56 * 0 + 164);
    setRetcheck(aslr(0x1434E30));
    getfield(state, -10002, "game");

    restoreRetcheck(aslr(0x1434E30));
 

    //setcall(retn, 5, (char*)0xE8);


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

