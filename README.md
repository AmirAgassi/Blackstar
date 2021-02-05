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

This method requires you to find the location of the condition checking the return address, and changing the condition to jump away from the shutdown function no matter what. 

![](https://i.gyazo.com/34d94757f147c0c1fd0e6582d4d50c67.png)

We can see that the condition uses a JB instruction, and after looking it up in the [Coder64 Opcode and Instruction Reference](http://ref.x86asm.net/coder64.html#x0F02) website, it is described as "Jump near if below/not above or equal/carry". 

![](https://i.gyazo.com/43820dbed96e12f08434d9f55f6e0189.png)

Since we want it to jump regardless of the condition, we can utilize the JMP instruction, which simply jumps no matter what.

![](https://i.gyazo.com/200d23f612162bfb3f1ad34a90677f62.png)

Although this works, there is still a problem. Roblox's memory checker scans through all of the important segments to make sure none of the memory was altered or patched irregularly. There is one flaw in this system though, and due to the fact that the memory checker has to scan almost the entire program, it will take a good while for it to get to your segment, and we can exploit this. Our new patched JMP instruction in the Lua C function will soon be scanned over by the memory checker and detected, so to avoid this, we will patch back the original byte, restoring the check back and complying with the memory checker all while bypassing the return check.


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
