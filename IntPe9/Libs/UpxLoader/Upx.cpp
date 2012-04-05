#include "Upx.h"

//This pointer
Upx* Upx::_this = 0;
unsigned long returnStart, returnJump, returnEnd;
unsigned long iatStart, iatEnd;

/**
 * Constructor
 *
 */
Upx::Upx()
{
	_this = this;
	_isUnpacked = false;
}

/* Initialize the UpxLoader
 *
 * Set up hooks and other stuff
 */
bool Upx::initialize()
{
	MODULEINFO info;
	unsigned long endloader = 0, popad = 0;

	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(0), &info, sizeof(MODULEINFO));

	if(*(unsigned char*)info.EntryPoint == 0x60)
	{
		endloader = (unsigned long)info.EntryPoint;
		while(true)
		{
			if(*(unsigned long*)endloader == 0x0)
				break;
			endloader++;
		}

		for(unsigned long j = (unsigned long)info.EntryPoint; j < endloader; j++)
		{
			if(*(unsigned long*)j == 0x8350F301) //IAT begin dump
			{
				*(unsigned char*)j = 0xE9;
				*(unsigned long*)(j+1) = JMP(j, storeIat);
				*(unsigned char*)(j+5) = 0x90; //fixup
				returnStart = j+6;
			}

			if(*(unsigned short*)j == 0x6158) //UNPACKED check hook
			{
				popad = j+1;
				returnJump = popad+5;

				*(unsigned char*)popad = 0xE9;
				*(unsigned long*)(popad+1) = JMP(popad, unpackedHook);
			}
		}
		return true;
	}else
		return false;
}

/**
 * Waits for the process to unpack
 *
 * Waits timeOut seconds for the process to unpack
 *
 * @param int		Amount of seconds to wait, (0 for indefinitely)
 */
unsigned long Upx::waitForUnpacked(int timeOut)
{
	clock_t timeout = clock() + timeOut * CLOCKS_PER_SEC;
	while(!_isUnpacked)
	{
		if(clock() > timeout && timeOut != 0) 
        	return WAIT_TIMEOUT;
		Sleep(10);
	}
	return 0;
}

/**
 * Hooks a function using the UPX IAT
 *
 * This will fail if the application does not has a UPX IAT
 *
 * @param const char*		Library name where the function resides
 * @param const char*		Function name
 * @param unsigned long		Address of new function
 * @return unsigned long	Address of old function
 */
unsigned long Upx::hookFunction(const char* libraryName, const char* functionName, unsigned long newFunction)
{
	if(iatStart == NULL || iatEnd == NULL)
		return hookIatFunction(NULL, functionName, newFunction); //Try to use default IAT Hook

	unsigned long address = (unsigned long)GetProcAddress(GetModuleHandleA(libraryName), functionName);

	//Address size of 4 so safe assumption to skip 4 every time
	for(unsigned long i = iatStart; i < iatEnd; i += 4)
	{
		if(*(unsigned long*)i == address)
		{
			*(unsigned long*)i = newFunction;
			return address;
		}
	}
	return 0; //fail
}

/**
 * Hooks a function using the real IAT
 *
 * This will fail if the application does not has a normal IAT
 *
 * @param const char*		Module name
 * @param const char*		Function name
 * @param unsigned long		Address of new function
 * @return unsigned long	Address of old function
 */
unsigned long Upx::hookIatFunction(const char* moduleName, const char* functionName, unsigned long newFunction)
{
	HMODULE hMod = GetModuleHandleA(moduleName);
	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)hMod;
	IMAGE_OPTIONAL_HEADER * pOptHeader = (IMAGE_OPTIONAL_HEADER*)((BYTE*)hMod + pDosHeader->e_lfanew +24);
	IMAGE_IMPORT_DESCRIPTOR* pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*) ((BYTE*)hMod + pOptHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress); //what is the rule of adding them?
	while ( pImportDesc->FirstThunk )
	{
		char * pszDllName = (char*) ((BYTE*)hMod + pImportDesc->Name);
		IMAGE_THUNK_DATA* pThunk = (IMAGE_THUNK_DATA*)((BYTE*)hMod + pImportDesc->OriginalFirstThunk);
		int n = 0;
		while ( pThunk->u1.Function )
		{
			char* pszFunName = (char*)((BYTE*)hMod + (DWORD)pThunk->u1.AddressOfData + 2);
			if(!IsBadReadPtr(pszFunName, 4))
			{
				//OutputDebugStringA(pszFunName);

				PDWORD lpAddr = (DWORD*)((BYTE*)hMod + pImportDesc->FirstThunk) + n;
				if(strcmp(pszFunName, functionName) == 0)
				{
					DWORD oldProtection;
					DWORD originalAddress = *lpAddr;
					//OutputDebugStringA("Found a match!");
					VirtualProtect(lpAddr, 4, PAGE_READWRITE, &oldProtection);
					*(DWORD*)lpAddr = newFunction;
					VirtualProtect(lpAddr, 4, oldProtection, &oldProtection);


					return originalAddress;
				}
			}
			n++;
			pThunk++;
		}
		pImportDesc++;
	}
	return 0;
}

/**
 * Search for aob and return the address
 *
 * @param const unsigned char[]		The AoB array
 * @param const char[]				The mask array ("xxx??xx?") 
 * @param unsigned long				Start address to search from
 * @param unsigned long				End address to search to
 * @return unsigned long			Address of AoB finding place
 */
unsigned long Upx::aob( const unsigned char AoB[], const char mask[], unsigned long aStart, unsigned long aMax)
{
	bool find;
	int size = strlen(mask);

	for(unsigned long address = aStart; address < aMax; address++)
	{
		if(*(unsigned char*)address == (unsigned char)AoB[0])
		{
			find = true;
			for(int i = 1; i < size; i++)
			{
				if(mask[i] == '?')
					continue;
				if(*(unsigned char*)(address+i) != (unsigned char)AoB[i])
				{
					find = false;
					break;
				}
			}
			if(find)
				return address;
		}
	}
	return 0;
}

/**
 * Static hook function to see if it unpacked
 *
 */
__declspec(naked) void Upx::unpackedHook()
{
	__asm
	{
		popad
		lea eax,[esp-80]
		pushad
	}

	_this->_isUnpacked = true;

	__asm
	{
		popad
		jmp returnJump
	}
}

/**
 * Static hook function to store the IAT address range
 *
 */
__declspec(naked) void Upx::storeIat()
{
	__asm
	{
		add ebx, esi
		push eax
		add edi, 8

		//IAT END ADDRESS
		cmp iatEnd, 0
		jne skipendzero
		mov iatEnd, ebx

		skipendzero:
		cmp iatEnd, ebx
		jnb skipend
		mov iatEnd, ebx
		skipend:

		//IAT START ADDRESS
		cmp iatStart, 0
		jne skipstartzero
		mov iatStart, ebx

		skipstartzero:
		cmp iatStart, ebx
		jb skipstart
		mov iatStart, ebx
		skipstart:

		jmp returnStart
    }
}
