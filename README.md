# BlackStar
BlackStar is a series of bypasses for all of Roblox's flags and checks, along with detailed and rigorous explanations to aid learning instead of spoonfeeding.

The planned bypasses are being made for memcheck, retcheck, hookcheck, FindWindowA, disabling logs, heartbeat, and VEH. BlackStar is a one-click injectable solution, or can be built into your own personal exploit.

BlackStar is auto-updating, only a very large and intentional Roblox update will break it. Obviously, any new checks will have to be added manually. 
This is achieved by scanning Roblox's memory for arrays of bytes to locate addresses dynamically. Checks will be periodically added as I make them for other projects.

The project MUST be built in x86 instead of x64 due to raw x86 assembly.

# Retcheck (return check)

Retcheck in Roblox is a very simple check. Each function with retcheck checks for the address it is returning a value to, then ensures it is in the memory space of Roblox before continuing. If the return address is not in the memory space of Roblox, the function then knows that the function is being called externally, most likely by an exploit, and therefore calls a function that shuts down the client.

![alt text](https://i.gyazo.com/87ebcc9753402722e2fd8c886c4b6a94.png)

To solve this problem, we need to be able to call these functions with retcheck attached, without triggering retcheck. My personal favourite method to solve this issue would be to simply replace the bytes in memory responsible to call the shutdown function after an exploit has been detected with NOP (0x70). This method requires you to find the location of the shutdown function call in memory, which is dynamically placed for each individual function, and my solution to this issue is a memory scanner.

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
    return 0;
}
```

After you locate the address in memory, you can simply write NOP (0x70) over x -> x+5. This wipes the function call from memory, and the function will simply bottom out and return once retcheck has been triggered. The if statement will simply pass as there is no further operations to complete.

```C++
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
        cout << "\ncannot set retcheck to invalid address" << endl;
    }
    else {
        DWORD a, b;
        VirtualProtect((LPVOID)end, 5, PAGE_EXECUTE_READWRITE, &a);
        *(char*)end = 0x90;
        *(char*)(end+1) = 0x90;
        *(char*)(end+2) = 0x90;
        *(char*)(end+3) = 0x90;
        *(char*)(end+4) = 0x90;
        VirtualProtect((LPVOID)end, 5, a, &a);
    }
}
```

This code works, but Roblox implements a memory checker that scans certain parts of it's memory space to find unauthorized patches. Since scanning an entire program's takes time, we can get away with patching the bytes in the function, running it, then patching back it's original bytes in order to appear unchanged for when the memory checker finally reaches it. This is a rough memcheck workaround.

```C++
void restoreRetcheck(int addr) {
    int retbytes[] = { 0x90, 0x31, 0xC0, 0x28, 0xFF };
    int end = scanForBytes(aslr(0x1434E30), retbytes, 1500);
    if (end == 0) {
        cout << "\ncannot set retcheck to invalid address" << endl;
    }
    else {
        DWORD a, b;
        VirtualProtect((LPVOID)end, 5, PAGE_EXECUTE_READWRITE, &a);
        *(char*)end = 0xE8;
        *(char*)(end + 1) = 0x31;
        *(char*)(end + 2) = 0xC0;
        *(char*)(end + 3) = 0x28;
        *(char*)(end + 4) = 0xFF;
        VirtualProtect((LPVOID)end, 5, a, &a);
    }
}
```

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
