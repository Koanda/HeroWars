#ifndef MEMORY_MANAGER_H_
#define MEMORY_MANAGER_H_

#include <Windows.h>
#include <stdio.h>

#define Naked __declspec( naked )

namespace MemoryManager
{
	typedef struct
	{
		unsigned char* adress;
		unsigned int length;
	} MemorySection;

	MemorySection SearchSection(char* executable_name, char* section_name)
	{
		MemorySection ms = {0};

		FILE* fh = NULL; // Bouh to me for using C file handler, but ho well in this case it's easier than C++ stream
		if(fopen_s(&fh,executable_name,"rb"))
		{
			printf("Error while trying to open %s",executable_name);
			return ms;
		}

		IMAGE_DOS_HEADER imdh;
		fread(&imdh,sizeof(imdh),1,fh);

		//set the pointer of the file to the location of the IMAGE_NT_HEADERS record
		fseek(fh, imdh.e_lfanew, 0);
		IMAGE_NT_HEADERS imnth;
		fread(&imnth,sizeof(imnth),1,fh);


		IMAGE_SECTION_HEADER *pimsh;
		pimsh = new IMAGE_SECTION_HEADER[imnth.FileHeader.NumberOfSections];

		if(pimsh == NULL)
		{
			perror("Error while trying to allocate IMAGE_SECTION_HEADER");
			exit(1);
		}

		fread(pimsh,sizeof(IMAGE_SECTION_HEADER),imnth.FileHeader.NumberOfSections,fh);

		for(int i=0;i<imnth.FileHeader.NumberOfSections;++i)
		{
			if(!strcmp(reinterpret_cast<char*>(pimsh[i].Name),section_name))
			{
				ms.adress = reinterpret_cast<unsigned char*>(pimsh[i].VirtualAddress + imnth.OptionalHeader.ImageBase);
				ms.length = pimsh[i].Misc.VirtualSize;
			}
		}
		delete pimsh;
		fclose(fh);
		return ms;
	}

	int ApplyJumpHook(unsigned char* code_adress, unsigned char* replace_routine, unsigned char nop = 0)
	{
		DWORD oldProtection = 0;
		if(!VirtualProtect(code_adress,5 + nop,PAGE_EXECUTE_READWRITE,&oldProtection))
		{
			printf("Error while applying jump at %p", code_adress);
			return -1;
		}

		*code_adress = 0xE9; //JMP int32
		*reinterpret_cast<unsigned int*>(code_adress + 1) = replace_routine - (code_adress + 5); //relative adress
		for(int i=0; i < nop; ++i) code_adress[i + 5] = 0x90; //fill with NOPs

		VirtualProtect(code_adress,5 + nop,oldProtection,&oldProtection);
		return 0;
	}

	int ApplyCallHook(unsigned char* code_adress, unsigned char* replace_routine, unsigned char nop = 0)
	{
		DWORD oldProtection = 0;
		if(!VirtualProtect(code_adress,5 + nop,PAGE_EXECUTE_READWRITE,&oldProtection))
		{
			printf("Error while applying jump at %p", code_adress);
			return -1;
		}

		*code_adress = 0xE8; //CALL int32
		*reinterpret_cast<unsigned int*>(code_adress + 1) = replace_routine - (code_adress + 5); //relative adress
		for(int i=0; i < nop; ++i) code_adress[i + 5] = 0x90; //fill with NOPs

		VirtualProtect(code_adress,5 + nop,oldProtection,&oldProtection);
		return 0;
	}

	/**
	* Search a code pattern from section_begin_adress to section_end_adress
	* return the first pattern adress found, NULL otherwise
	*/
	unsigned char* SearchCodeAdress(unsigned char* section_begin_adress, unsigned char* section_end_adress, unsigned char* pattern,unsigned int pattern_length)
	{
		if((section_begin_adress > section_end_adress) || pattern_length == 0)
		{
			return NULL;
		}

		DWORD oldProtection = 0;
		unsigned char* adress = NULL;

		if(!VirtualProtect(section_begin_adress,section_end_adress - section_begin_adress,PAGE_EXECUTE_READWRITE,&oldProtection))
		{
			printf("Unable to search code pattern");
			return NULL;
		}

		for(;(section_begin_adress < section_end_adress) && !adress; section_begin_adress++)
		{
			if(*section_begin_adress == pattern[0]) //first pattern byte match
			{
				unsigned int i=1;
				for(;i < pattern_length && section_begin_adress[i] == pattern[i];++i); //check other bytes
				if(i == pattern_length) adress = section_begin_adress;
			}
		}

		VirtualProtect(section_begin_adress,section_end_adress - section_begin_adress,oldProtection,&oldProtection);
		return adress;
	}

	unsigned char* SearchCodeAdress(MemorySection& memory_section,unsigned char* pattern,unsigned int pattern_length)
	{
		return SearchCodeAdress(memory_section.adress,memory_section.adress + memory_section.length,pattern,pattern_length);
	}
};
#endif