#include <Windows.h>
#include "Injector.h"
#include <stdlib.h>
#include <stdio.h>

Injector::Injector()
{
	_isAlive = true;

	_thread = new QThread;
	moveToThread(_thread);
	_thread->start();

	QMetaObject::invokeMethod(this, "listenerLoop", Qt::QueuedConnection);
}

Injector::~Injector()
{
	_isAlive = false;
	_thread->exit();
	delete _thread;
}

void Injector::listenerLoop()
{
	bool injected = false;
	while(_isAlive)
	{
		if(!injected)
			injected = inject();
		Sleep(20);
	}
}

bool Injector::inject()
{
	LPSTR dllname = "League of Legends.dll";
	LPSTR executablename = "League of Legends (TM) CLient";

	HWND win = NULL;	

	if(!(win = FindWindowA(NULL, executablename))) 
		return false;

	DWORD id;
	if(!GetWindowThreadProcessId(win,&id))
		return false;
	printf("Process ID: %d\n",id);

	HANDLE han = NULL;
	if(!(han = OpenProcess(PROCESS_ALL_ACCESS,FALSE,id)))
		return false;

	char currentDirectory[MAX_PATH + 1 + sizeof(dllname)] = {0}; 
	GetCurrentDirectoryA(MAX_PATH,currentDirectory);

	strcat_s(currentDirectory,"\\");
	strcat_s(currentDirectory,dllname);
	printf("Dll path: %s\n",currentDirectory);

	void* allocatedMemory = VirtualAllocEx(han,NULL,strlen(currentDirectory) + 1,MEM_COMMIT | MEM_RESERVE,PAGE_READWRITE);
	if(allocatedMemory == NULL)
		return false;

	printf("Dll Name allocated at: %p\n",allocatedMemory);

	SIZE_T writedBytes;
	WriteProcessMemory(han,allocatedMemory,currentDirectory,strlen(currentDirectory) + 1,&writedBytes);

	if(writedBytes < strlen(dllname) + 1)
		return false;


	HANDLE thread = CreateRemoteThread(han,NULL,0,(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("kernel32.dll"),"LoadLibraryA"),allocatedMemory,0,NULL);
	if(thread == NULL)
		return false;

	printf("Dll successfully injected\n");
	return true;
}