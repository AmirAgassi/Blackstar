# BlackStar
BlackStar is a series of bypasses for all of Roblox's flags and checks. 

The planned bypasses are being made for memcheck, retcheck, hookcheck, FindWindowA, disabling logs, heartbeat, and VEH. BlackStar is a one-click injectable solution, or can be built into your own personal exploit.

BlackStar is auto-updating, only a very large and intentional Roblox update will break it. Obviously, any new checks will have to be added manually. 
This is achieved by scanning Roblox's memory for arrays of bytes to locate addresses dynamically. Checks will be periodically added as I make them for other projects.

The project MUST be built in x86 instead of x64 due to raw x86 assembly.

# Retcheck (return check)

Retcheck in Roblox is a very simple check. Each function with retcheck checks for the address it is returning a value to, then ensures it is in the memory space of Roblox before continuing. If the return address is not in the memory space of Roblox, the function then knows that the function is being called externally, most likely by an exploit, and therefore calls a function that shuts down the client.
![alt text]https://i.gyazo.com/f9e7000f0f95a771582403d5ee969143.png


# Anti-log upload crashes

Roblox uses a WinAPI MessageBox to display a crash message before uploading logs. To circumvent this, you can overwrite the WinAPI MessageBox function to watch when it is called for a roblox crash, then wipes the logs before it can be uploaded.

```C++
int hookMessageBox(const char *errTitle, const char* errMsg) {
    if (errTitle == "Roblox has crashed.") {
        DWORD logInfo = 0xB316A;
        for (int i = 0; i < 150; i++) {
            VirtualProtect((LPVOID)(logInfo + i), 1, PAGE_EXECUTE_READWRITE, (PDWORD)0x90); //delete logs
        }
    }
}
void overwriteMessageBox() {
    DWORD bkup;
    VirtualProtect((LPVOID)&MessageBoxA, 1, PAGE_EXECUTE_READWRITE, &bkup);
    *(char*)(&MessageBoxA) = 0xE9;
    *(DWORD*)((DWORD)&MessageBoxA+1)=((DWORD)&hookMessageBox-(DWORD)&MessageBoxA)-5;
    VirtualProtect((LPVOID)&MessageBoxA, 1, bkup, &bkup);
}
 ```
 
 # FindWindowA check
 
 Roblox has added a simple check to FindWindowA, so all you need to do is write a NOP to a JNP that runs the check.
 
 ```C++
 void bypassFindWindowA() {
	 DWORD* bkup;
	 VirtualProtect((LPVOID)&FindWindowA, 1, PAGE_EXECUTE_READWRITE, &bkup);
	 *(char*)&FindWindowA = 0x90;
	 VirtualProtect((LPVOID)&FindWindowA, 1, bkup, &bkup);
}
```
