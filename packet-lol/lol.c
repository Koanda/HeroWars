#include "lol.h"

struct dbwin_buffer *dbg;
base64_decodestate base64;

/* Listener system */
static gboolean dbg_stop = FALSE;
static HANDLE event_dbr, event_ddr, mapping;
static HANDLE dbg_thread = NULL;

void parse_cmd(HANDLE handle)
{
	int i = 0;
	int peb_address;
	int rtl_user_proc_params_address;
	UNICODE_STRING command_line_struct;
	gchar *command_line, *p, *t;

	peb_address = GetPebAddress(handle);
	ReadProcessMemory(handle, (char*)peb_address + 0x10, &rtl_user_proc_params_address, sizeof(int), NULL);
	ReadProcessMemory(handle, (char*)rtl_user_proc_params_address + 0x40, &command_line_struct, sizeof(command_line_struct), NULL);
	command_line = g_malloc(command_line_struct.Length);
	ReadProcessMemory(handle, command_line_struct.Buffer, command_line, command_line_struct.Length, NULL);

	inplace_to_ascii(command_line, command_line_struct.Length);

	//Extraction
	command_line[strlen(command_line)-2] = '\0'; //Remove last "
	p = strrchr(command_line, '"');
	p = strchr(p, ' ')+1; p[4] = '\0';
	OutputDebugStringA(p);
	gPREF_PORT = atoi(p); p+=5;
	t = strchr(p, ' '); t[0] = '\0';
	memcpy(gPREF_KEY, p, strlen(p)+1);
	g_free(command_line);

	//Port setting
	set_single_port(gPREF_PORT);

	//Key decrypting
	//base64_init_decodestate(&base64);
	base64_decode_block(gPREF_KEY, (int)strlen(gPREF_KEY), key, &base64);
	isKey = TRUE;
	OutputDebugStringA(gPREF_KEY);
}


void inplace_to_ascii(char* unicode, int length)
{
	int i, x;
	for(i = 2, x = 1; i < length; i+=2, x++)
		unicode[x] = unicode[i];
}

void dbg_listener_thread(void *thread_parameter)
{
	HANDLE gc_handle;

	while(!initialized) //Wait for the dissector to be loaded
		Sleep(50);

	while(!dbg_stop)
	{
		gc_handle = getHandleByName(LOL_EXE);
		if(gc_handle != NULL)
		{
			parse_cmd(gc_handle);
			WaitForSingleObject(gc_handle, INFINITE);
			CloseHandle(gc_handle);
		}
		Sleep(SLEEP_TIMEOUT);
	}
}

HANDLE getHandleByName(char *name)
{
	PROCESSENTRY32 entry;
	HANDLE hProcess = NULL;
	HANDLE snapshot = (HANDLE)CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	entry.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(snapshot, &entry) == TRUE)
		while (Process32Next(snapshot, &entry) == TRUE)
			if (_stricmp(entry.szExeFile, name) == 0)
				hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);

	CloseHandle(snapshot);
	return hProcess;
}

int GetPebAddress(HANDLE ProcessHandle)
{
	_NtQueryInformationProcess NtQueryInformationProcess = (_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");
	PROCESS_BASIC_INFORMATION pbi;
	NtQueryInformationProcess(ProcessHandle, 0, &pbi, sizeof(pbi), NULL);
	return (int)pbi.PebBaseAddress;
}


void stop_listener(void) 
{
	dbg_stop = TRUE;
	WaitForSingleObject(dbg_thread, INFINITE);
}

void start_listener()
{
	dbg_thread = (HANDLE)_beginthread(dbg_listener_thread, 8192, NULL);
	SetThreadPriority(dbg_thread, THREAD_PRIORITY_TIME_CRITICAL);
}
