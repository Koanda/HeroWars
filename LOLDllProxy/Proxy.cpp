#include <stdio.h>
#include <enet\enet.h>
#include <Windows.h>
#include "MemoryManager.h"

unsigned char enet_host_service_pattern[] = {0x8B,0xD8,0x83,0xC4,0x0C,0x83,0xFB,0xFE};// just after we call enet_host_service
unsigned char decrypt_packet_pattern[] = {0x8B,0x52,0x0C,0x52,0x51,0x50}; // just before we decrypt the packet

/*
006FC94A   . 8B52 0C        MOV EDX,DWORD PTR DS:[EDX+C]
006FC94D   . 52             PUSH EDX                                 ; /Arg3
006FC94E   . 51             PUSH ECX                                 ; |Arg2
006FC94F   . 50             PUSH EAX                                 ; |Arg1
006FC950   . E8 5BE01D00    CALL League_o.008DA9B0                   ; \League_o.008DA9B0 //the decrypte function !!!
006FC955   . E9 CC010000    JMP League_o.006FCB26 //here we hook
*/


void GetBlowfishKey(char* key)
{
	/*"D:/jeux/League of Legends/League of Legends/RADS/solutions/lol_game_client_sln/releases/0.0.0.133/deploy/League of Legends.exe" 
	"8394" "LoLLauncher.exe" 
	"D:/jeux/League of Legends/League of Legends/RADS/projects/lol_air_client/releases/0.0.0.133/deploy/LolClient.exe" 
	"31.186.224.153 5165 VkddQDA/U0PNc904oSSrCw== 19728698"
	*/
	GetCommandLineA();
}

/*
void __stdcall OnReceivePacket(ENetEvent* event)
{
if(event == NULL) return;

switch(event->type)
{
case ENET_EVENT_TYPE_CONNECT:

break;

case ENET_EVENT_TYPE_RECEIVE:
printf ("\n[Received : %d], DATA : ",event->packet->dataLength);

for(unsigned int i=0; i < event->packet->dataLength; ++i)
{
printf("%02x ",event->packet->data[i]);
}			

printf("\nASCII : ");
for(unsigned int i=0; i < event->packet->dataLength; ++i)
{
if(isalnum(event->packet->data[i]))
printf("%c",event->packet->data[i]);				
}
break;

case ENET_EVENT_TYPE_DISCONNECT:
printf ("%s disconected.\n", event->peer -> data);
break;
}
}
*/
/*
Naked void ASMOnReceivePacket() 
{
__asm 
{
cmp eax,0
jle no_event
LEA EAX, DWORD PTR [ESP + 0x20] // DWORD PTR [ESP + 0x1C]
PUSH EAX
CALL OnReceivePacket
no_event:			
MOV EBX,EAX
ADD ESP,0x10			
JMP DWORD PTR [ESP - 0x10]
}
}
*/

void __stdcall OnReceivePacket(ENetPacket* packet)
{
	printf ("\n[Received : %d], DATA : ",packet->dataLength);

	for(unsigned int i=0; i < packet->dataLength; ++i)
	{
		printf("%02x ",packet->data[i]);
	}			

	printf("\nASCII : ");
	for(unsigned int i=0; i < packet->dataLength; ++i)
	{
		if(isalnum(packet->data[i]))
			printf("%c",packet->data[i]);				
	}
}

Naked void ASMOnReceivePacket() 
{	
	__asm 
	{
		PUSH DWORD PTR [ESP + 0x30]
		CALL OnReceivePacket
			RET
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{	
	if(fdwReason == DLL_PROCESS_ATTACH)
	{		
		AllocConsole();
		SetConsoleTitleA( "LOL Proxy" );
		FILE* console;
		freopen_s(&console, "CONOUT$", "wb", stdout);
		freopen_s(&console, "CONOUT$", "wb", stderr);		
		printf("Attached");	

		char execname[MAX_PATH];
		GetModuleFileNameA(NULL,execname,MAX_PATH);
		printf("Executable path: %s\n",execname);

		MemoryManager::MemorySection section = MemoryManager::SearchSection(execname, ".text");
		printf(".text section : adress %p, end %p\n",section.adress,section.adress + section.length);

		unsigned char* adress = MemoryManager::SearchCodeAdress(section,decrypt_packet_pattern,
			sizeof(decrypt_packet_pattern));

		if(adress)
		{
			printf("decrypt_packet_pattern found at %p\n",adress + 0xB); //0x006FC955
			MemoryManager::ApplyCallHook(reinterpret_cast<unsigned char*>(adress + 0xB),reinterpret_cast<unsigned char*>(ASMOnReceivePacket));
		}
		else
		{
			printf("decrypt_packet_pattern not found");
		}
	}

	return TRUE;
}