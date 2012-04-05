#include "LeagueOfLegends.h"

LeagueOfLegends *leagueOfLegends = NULL;
BOOL APIENTRY DllMain(HANDLE thisHandle, DWORD callReason, LPVOID reserved)
{
	switch(callReason)
	{
		case DLL_PROCESS_ATTACH:
			leagueOfLegends = new LeagueOfLegends();
			leagueOfLegends->initialize();
		break;
		case DLL_PROCESS_DETACH:
			if(leagueOfLegends != NULL)
				leagueOfLegends->finalize();
		break;
	}
	return true;
}