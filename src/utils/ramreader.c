#include "ramreader.h"

uintptr_t GetModuleBaseAddress(DWORD dwProcID, char* szModuleName)
{
    uintptr_t ModuleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcID);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 ModuleEntry32;
        ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hSnapshot, &ModuleEntry32))
        {
            do
            {
                if (strcmp(ModuleEntry32.szModule, szModuleName) == 0)
                {
                    ModuleBaseAddress = (uintptr_t)ModuleEntry32.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnapshot, &ModuleEntry32));
        }
        CloseHandle(hSnapshot);
    }
    return ModuleBaseAddress;
}

uintptr_t ResolvePointersChain(HANDLE pHandle, uintptr_t baseAddress, uintptr_t pointersChain[], int length){
    uintptr_t finalAddress = baseAddress;

    for(int i=0;i<length;++i){
        ReadProcessMemory(pHandle, (LPVOID)finalAddress, &finalAddress, sizeof(float), NULL);
        finalAddress += pointersChain[i];
    }

    return finalAddress;
}

int OttieniStato(){
    DWORD PID;
    uintptr_t pointersChain[4] = {0x5C, 0x0, 0xA8, 0xC};
    int gameState;

    GetWindowThreadProcessId(FindWindow(NULL, "Among Us"), &PID);
    HANDLE pHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
    uintptr_t BaseAddress = GetModuleBaseAddress(PID, "GameAssembly.dll");

    uintptr_t playerObjectAddress = BaseAddress+0x01C9CB30; //0x01C9EF04 Player

    uintptr_t playerSpeedAddress = ResolvePointersChain(pHandle, playerObjectAddress, pointersChain, 4);

    ReadProcessMemory(pHandle, (LPVOID)playerSpeedAddress, &gameState, sizeof(gameState), NULL);

    return gameState;
}