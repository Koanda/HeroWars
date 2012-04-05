#ifndef LEAGUE_OF_LEGENDS_H
#define LEAGUE_OF_LEGENDS_H

#include "..\Skeleton.h"
#include "Blowfish\Blowfish.h"
#include "General\General.h"
#include <enet/enet.h>

#include <map>
using std::map;

class LeagueOfLegends : public Skeleton
{
private:
	uint16 _keySize;
	uint8 *_key;
public:
	BlowFish *blowfish;
	LeagueOfLegends();

	void initialize();
	void finalize();
};

extern LeagueOfLegends *leagueOfLegends;

#endif