# BlackStar
BlackStar is a series of bypasses for all of Roblox's flags and checks. 

The planned bypasses are being made for memcheck, retcheck, hookcheck, FindWindowA, disabling logs, heartbeat, and VEH. BlackStar is a one-click injectable solution, or can be built into your own personal exploit.

BlackStar is auto-updating, only a very large and intentional Roblox update will break it. Obviously, any new checks will have to be added manually. 
This is achieved by scanning Roblox's memory for arrays of bytes to locate addresses dynamically. Checks will be periodically added as I make them for other projects.


Hookcheck

```C++
int hookMessageBox(const char *errTitle, const char* errMsg) {
    if (errTitle == "Roblox has Crashed") {
        DWORD logInfo = 0xB316A;
        for (int i = 0; i < 150; i++) {
            VirtualProtect((LPVOID)(logInfo + i), 1, PAGE_EXECUTE_READWRITE, (PDWORD)0x90); //delete logs
        }
    }
}


void overwriteMessageBox() {
    DWORD o;
    VirtualProtect((LPVOID)&MessageBoxA, 1, PAGE_EXECUTE_READWRITE, &o);
    *(char*)(&MessageBoxA) = 0xE9;
    *(DWORD*)((DWORD)&MessageBoxA + 1) = ((DWORD)&hookMessageBox - (DWORD)&MessageBoxA) - 5;
    VirtualProtect((LPVOID)&MessageBoxA, 1, o, &o);
}
 ```
 
