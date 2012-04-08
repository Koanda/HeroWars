/**
 * Copyright (C) 2012 Jean Guegant (Jiwan)
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <enet\enet.h>
#include <Windows.h>
#include "MemoryManager.h"

unsigned char enet_host_service_pattern[] = {0x8B,0xD8,0x83,0xC4,0x0C,0x83,0xFB,0xFE};// just after we call enet_host_service
unsigned char decrypt_packet_pattern[] = {0x8B,0x52,0x0C,0x52,0x51,0x50}; // just after we decrypt the packet
unsigned char encrypt_packet_pattern[] = {0x8B,0x53,0x0C,0x83,0xC4,0x0C,0x57};
unsigned char sendpacket[] = {0x55,0x8B,0xEC,0x83,0xE4,0xF8,0x51,0x8B,0x45,0x14,0x83,0xE8,0x00};


FILE* logFile; //Where every packets whil be logs
FILE* channelFile;
unsigned int packetCount; //Global variable that store how much have been logs, hope it won't overflow onde day :p

typedef enum 
{
	PT_SERVER_TO_CLIENT,
	PT_CLIENT_TO_SERVER
} PACKET_TYPE;

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

	SYSTEMTIME time = {0};
	GetSystemTime(&time);	
	PACKET_TYPE type = PT_SERVER_TO_CLIENT; //fwrite is gay for writing constante values
	fwrite(&type, sizeof(PACKET_TYPE),1,logFile);
	fwrite(&time,sizeof(SYSTEMTIME),1,logFile);
	fwrite(&packet->dataLength, sizeof(size_t),1,logFile);
	fwrite(packet->data,packet->dataLength,1,logFile);

	packetCount++;
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

void __stdcall OnSendPacket(unsigned char* data, unsigned int length, int channel)
{
	unsigned char cmd = data[0];
	fprintf_s(channelFile,"cmd : %02x, channel : %02x\n",cmd,channel);

	printf ("\n[Sended : %d], DATA : ",length);	
	for(unsigned int i = 0 ; i < length; ++i)
	{
		printf("%02x ",data[i]);
	}

	printf("\nASCII : ");
	for(unsigned int i = 0 ; i < length; ++i)
	{
		if(isalnum(data[i]))
			printf("%c",data[i]);				
	}

	SYSTEMTIME time = {0};
	GetSystemTime(&time);
	
	PACKET_TYPE type = PT_CLIENT_TO_SERVER; //fwrite is gay for writing constante values
	fwrite(&type, sizeof(PACKET_TYPE),1,logFile);
	fwrite(&time,sizeof(SYSTEMTIME),1,logFile);
	fwrite(&length, sizeof(size_t),1,logFile);
	fwrite(data,length,1,logFile);

	packetCount++;
}

Naked void ASMOnSendPacket()
{
	 __asm
	 {
		 PUSHAD
		 PUSH DWORD PTR [EBP + 0x10] //CHANNEL
		 PUSH DWORD PTR [EBP + 0x8] //SIZE
		 PUSH DWORD PTR [EBP + 0x0C] //DATA
		 CALL OnSendPacket
		 POPAD
		 MOV EAX,DWORD PTR SS:[EBP + 0x14]
         SUB EAX,0
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
			
			char logFileName[50] = {0};
			SYSTEMTIME time = {0};
			GetSystemTime(&time);

			sprintf_s(logFileName,"packets logs %d-%d-%d_%d-%d-%d.pkt",time.wDay,time.wMonth,time.wYear,time.wHour,time.wMinute,time.wSecond);
			printf("Packets logged in %s",logFileName);

			if(fopen_s(&logFile,logFileName,"wb"))
			{
				printf("Unable to open %s to log packets",logFileName);
				return 1;
			}
			
			sprintf_s(logFileName,"channels logs %d-%d-%d_%d-%d-%d.pkt",time.wDay,time.wMonth,time.wYear,time.wHour,time.wMinute,time.wSecond);
			printf("Packets channel logged in %s",logFileName);
			if(fopen_s(&channelFile,logFileName,"w"))
			{
				printf("Unable to open %s to log packets",logFileName);
				return 1;
			}

			fwrite(&packetCount,sizeof(packetCount),1,logFile);

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

			adress = MemoryManager::SearchCodeAdress(section,sendpacket,
				sizeof(sendpacket));
			if(adress)
			{
				printf("sendpacket found at %p\n",adress + 0x7);
				MemoryManager::ApplyCallHook(reinterpret_cast<unsigned char*>(adress + 0x7),reinterpret_cast<unsigned char*>(ASMOnSendPacket),1);
			}
			else
			{
				printf("encrypt_packet_pattern not found");
			}	
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
			if(logFile) {				
				fseek(logFile,0,SEEK_SET);
				fwrite(&packetCount,sizeof(packetCount),1,logFile);
				fclose(logFile);
			}

			if(channelFile)
			{
				fclose(channelFile);
			}
	FreeConsole();
	}

	return TRUE;
}