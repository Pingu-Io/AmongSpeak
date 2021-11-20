#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <string.h>

uintptr_t GetModuleBaseAddress(DWORD dwProcID, char* szModuleName);
uintptr_t ResolvePointersChain(HANDLE pHandle, uintptr_t baseAddress, uintptr_t pointersChain[], int length);
int OttieniStato();