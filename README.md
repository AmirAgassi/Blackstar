# BlackStar
BlackStar is a series of bypasses for all of Roblox's flags and checks, along with detailed and rigorous explanations to aid learning instead of spoonfeeding.

The planned bypasses are being made for memcheck, retcheck, hookcheck, FindWindowA, disabling logs, heartbeat, and VEH. BlackStar is not going to be downloadable or compiled to be injected, to further avoid use without understanding. Obviously, any new checks will have to be added manually. Checks will be periodically added as I make them for other projects.

~~BlackStar is auto-updating, only a very large and intentional Roblox update will break it.~~
~~This is achieved by scanning Roblox's memory for arrays of bytes to locate addresses dynamically.~~
I am too lazy to do that. Have a field day with the IDA Pro decompiler you obviously pirated.

The project MUST be built in x86 due to raw x86 assembly.

# Retcheck (return check)

Retcheck in Roblox is a very simple check. Each function with retcheck checks for the address it is returning a value to, then ensures it is in the memory space of Roblox before continuing. If the return address is not in the memory space of Roblox, the function then knows that the function is being called externally, by an exploit, and therefore calls a function that shuts down the client.

![alt text](https://i.gyazo.com/b22b14d1524b483128e7feddcd65ee4f.png)

To solve this problem, we need to be able to call these functions with retcheck attached, without triggering retcheck.  

![](https://i.gyazo.com/34d94757f147c0c1fd0e6582d4d50c67.png)

We can see that the condition uses a JB instruction, and after looking it up in the [Coder64 Opcode and Instruction Reference](http://ref.x86asm.net/coder64.html#x0F02) website, it is described as "Jump near if below/not above or equal/carry". 

![](https://i.gyazo.com/43820dbed96e12f08434d9f55f6e0189.png)

Since we want it to jump regardless of the condition, we can utilize the JMP instruction, which simply jumps no matter what.

![](https://i.gyazo.com/200d23f612162bfb3f1ad34a90677f62.png)

Now that we have a game plan down to bypass retcheck, we next need to be able to locate the condition in any Lua C function. After studying a few functions with retcheck, I found three bytes that seemed static throughout each implementation, and that was 0x72, 0xA1 2 bytes forward, and 0x8B another 7 bytes forward (relative to the first instruction).

![](https://i.gyazo.com/4067f1c0d5acaffd55eb8c6a564a5fad.png)

Now that we have an identifier found, we can start at the top of the function given, and search down until we find 0x72 at the first position, 0xA1 2 bytes forward, and 0x8B 7 bytes forward. Let's write it. 

HOIST

```C++
bool bypassRetcheck(DWORD addy) {
    int retcheckInstructions[] = { 0x72, 0xA1, 0x8B };
    BYTE* functionalAddr = (BYTE*)addy;
    while (!(functionalAddr[0] == retcheckInstructions[0] && functionalAddr[2] == retcheckInstructions[1] && functionalAddr[7] == retcheckInstructions[2])) {
        functionalAddr += 1;
    }
    patchRetcheck(functionalAddr);
    return true;
}
```
Next, we can simply use the address pointing to the JB instruction and, after ensuring the identifiers check out, write the replacement byte (our JMP instruction, 0xEB).

```C++
void patchRetcheck(BYTE* functionalAddr, DWORD addr) {
    int replacementByte = 0xEB;
    if (functionalAddr[0] == 0x72 && functionalAddr[2] == 0xA1 && functionalAddr[7] == 0x8B) {
        WriteProcessMemory(GetCurrentProcess(), *(LPVOID*)&functionalAddr, (LPVOID)&replacementByte, 1, NULL);
    }
}
```

Although this works, there is still a problem. Roblox's memory checker scans through all of the important segments to make sure none of the memory was altered or patched irregularly. There is one flaw in this system though, and due to the fact that the memory checker has to scan almost the entire program, it will take a good while for it to get to your segment, and we can exploit this. Our new patched JMP instruction in the Lua C function will soon be scanned over by the memory checker and detected, so to avoid this, we will patch back the original byte, restoring the check back and complying with the memory checker all while bypassing the return check.

```C++
void restoreRetcheck(BYTE* functionalAddr) {
    if (functionalAddr[0] == replacementByte && functionalAddr[2] == 0xA1 && functionalAddr[7] == 0x8B) {
        WriteProcessMemory(GetCurrentProcess(), *(LPVOID*)&functionalAddr, (LPVOID)&retcheckInstructions[0], 1, NULL);
    }
}
```

# Anti log-upload crashes

Roblox uses a WinAPI MessageBox to display a crash message before uploading logs. To circumvent this, you can overwrite the WinAPI MessageBox function to watch when it is called for a roblox crash, then wipes the logs before it can be uploaded.

```C++
int hookMessageBox(const char *errTitle, const char* errMsg) {
    if (errTitle == "Roblox has crashed.") {
        DWORD logInfo = aslr(0xB316A);
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
