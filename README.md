# BlackStar
BlackStar is a series of bypasses for all of Roblox's flags and checks. 

The planned bypasses are being made for memcheck, retcheck, hookcheck, FindWindowA, disabling logs, heartbeat, and VEH. BlackStar is a one-click injectable solution, or can be built into your own personal exploit.

BlackStar is auto-updating, only a very large and intentional Roblox update will break it. Obviously, any new checks will have to be added manually. 
This is achieved by scanning Roblox's memory for arrays of bytes to locate addresses dynamically. Checks will be periodically added as I make them for other projects.

The project MUST be built in x86 instead of x64 due to raw x86 assembly.

# Retcheck (return check)

Retcheck in Roblox is a very simple check. Each function with retcheck checks for the address it is returning a value to, then ensures it is in the memory space of Roblox before continuing. If the return address is not in the memory space of Roblox, the function then knows that the function is being called externally, most likely by an exploit, and therefore calls a function that shuts down the client.

![alt text](https://i.gyazo.com/87ebcc9753402722e2fd8c886c4b6a94.png)

To solve this problem, we need to be able to call these functions with retcheck attached, without triggering retcheck. My personal favourite method to solve this issue would be to simply replace the bytes in memory responsible to call the shutdown function after an exploit has been detected with NOP (0x70). This method requires you to find the location of the shutdown function call in memory, which is dynamically placed for each individual function. My solution to this issue is a memory scanner.

```C++
int scanForBytes(int addy, int bytes[], int max) {
    int origaddy = addy;
    for (int a = 1; a < max; a++) {
        BYTE* add = (BYTE*)(((BYTE*)addy)[0]);
        int addx = (int)add;
        if (addx == 0xC2) {
            return 0;
            break;
        }
        if (addx == bytes[0]) {
            
            int tempaddy = addy;
            int startaddy = tempaddy;
            for (int b = 1; b <= sizeof(bytes); b++) {
                int comp = (int)(BYTE*)(((BYTE*)tempaddy+b)[0]);
                if (comp == bytes[b]) {
                }
                else
                {
                    break;
                }
                if (b == sizeof(bytes)) {
                    return tempaddy;
                }
            }
        }
        addy += 1;
    }
    return 1;
}```


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
