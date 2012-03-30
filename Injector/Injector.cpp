#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>

void exit_error()
{
	system("PAUSE");
	exit(EXIT_FAILURE);
}

int main(char** args, int argc)
{
	LPSTR dllname = "LOLDllProxy.dll";
	LPSTR executablename = "League of Legends (TM) CLient";

	HWND win = NULL;	

	if(!(win = FindWindowA(NULL,executablename))) 
	{
		perror("Unable to find League of Legends windows, are you sure it's running?\n");
		exit_error();
	}

	DWORD id;
	if(!GetWindowThreadProcessId(win,&id))
	{
		perror("Unable to get process ID\n");
		exit_error();
	}
	printf("Process ID: %d\n",id);

	HANDLE han = NULL;
	if(!(han = OpenProcess(PROCESS_ALL_ACCESS,FALSE,id)))
	{
		perror("Unable to open process\n");
		exit_error();
	}
	
	char currentDirectory[MAX_PATH + 1 + sizeof(dllname)] = {0}; 
	GetCurrentDirectoryA(MAX_PATH,currentDirectory);
	
	strcat_s(currentDirectory,"\\");
	strcat_s(currentDirectory,dllname);
	printf("Dll path: %s\n",currentDirectory);

	void* allocatedMemory = VirtualAllocEx(han,NULL,strlen(currentDirectory) + 1,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);
	if(allocatedMemory == NULL)
	{
		perror("Error allocating memory for dll name\n");
		exit_error();
	}
	printf("Dll Name allocated at: %p\n",allocatedMemory);
		
	SIZE_T writedBytes;
	WriteProcessMemory(han,allocatedMemory,currentDirectory,strlen(currentDirectory) + 1,&writedBytes);

	if(writedBytes < strlen(dllname) + 1)
	{
		perror("Error writing Dll name in memory\n");
		exit_error();
	}
	

	HANDLE thread = CreateRemoteThreadEx(han,NULL,0,(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"),"LoadLibraryA"),allocatedMemory,0,NULL,NULL);
	if(thread == NULL)
	{
		perror("Error while creating dll thread\n");
		exit_error();
	}

	printf("Dll successfully injected\n");
	system("PAUSE");
	return EXIT_SUCCESS;
}