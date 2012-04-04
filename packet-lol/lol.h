#include "define.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#undef UNICODE
#undef _UNICODE
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#include <glib.h>
#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/frame_data.h>
#include <epan/reassemble.h>

#define LOL_PORT_MIN 5000
#define LOL_PORT_MAX 5500
#define LOL_EXE "League of Legends.exe"
#define SLEEP_TIMEOUT 50

#include "blowfish.h"
#include "cdecode.h"
#include <enet\enet.h>

/* Externals */
extern gboolean initialized;
extern guint8 *gPREF_KEY;
extern guint gPREF_PORT;
extern base64_decodestate base64;
extern byte key[16];
extern gboolean isKey;

/* forward reference */
void proto_register_lol();
void proto_reg_handoff_lol();
void dissect_lol(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree);
void proto_reg_handoff_lol(void);
void set_single_port(guint port);

/* Listener */
void start_listener();
void stop_listener(void);
HANDLE getHandleByName(char *name);
void dbg_listener_thread(void *thread_parameter);
int GetPebAddress(HANDLE ProcessHandle);
void inplace_to_ascii(char* unicode, int length);

#pragma pack(1)
typedef struct _LolPacket
{
	enet_uint16 command;
	enet_uint16 length;
	enet_uint8 data[MAX_PATH];
} LolPacket;
#pragma pack()

/* Structs for command line getting */
typedef ULONG (NTAPI *_NtQueryInformationProcess)(
	HANDLE ProcessHandle,
	DWORD ProcessInformationClass,
	PVOID ProcessInformation,
	DWORD ProcessInformationLength,
	PDWORD ReturnLength
);

typedef struct _UNICODE_STRING
{
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PROCESS_BASIC_INFORMATION
{
    LONG ExitStatus;
    PVOID PebBaseAddress;
    ULONG_PTR AffinityMask;
    LONG BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR ParentProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;