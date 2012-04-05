/*
HeroWars server for League of Legends 
Copyright (C) 2012  Intline9

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "NetworkListener.h"

#define SERVER_HOST ENET_HOST_ANY 
#define SERVER_PORT 5109
#define SERVER_KEY "ODlks70+e2SYHI2doO6LKw=="

#define SERVER_VERSION "0.0.1"

int main (int argc, char ** argv) 
{
	printf("HeroWars %s\n", SERVER_VERSION);

	NetworkListener *listener = new NetworkListener();
	ENetAddress address;
	address.host = SERVER_HOST;
	address.port = SERVER_PORT;

	listener->initialize(&address, SERVER_KEY);
	printf("Starting netloop\n");
	listener->netLoop();
	delete listener;

	printf("Goodbye");
}