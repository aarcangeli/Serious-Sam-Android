/* Copyright (c) 2002-2012 Croteam Ltd. 
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */


#ifndef SE_INCL_GAMEAGENT_H
#define SE_INCL_GAMEAGENT_H
#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <cerrno>
#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

extern CTString ms_strServer;
extern CTString ms_strMSLegacy;
extern CTString ms_strDarkPlacesMS;
extern CTString ms_strGameName;
extern BOOL ms_bMSLegacy;
extern BOOL ms_bDarkPlacesMS;
extern BOOL ms_bDarkPlacesDebug;

extern void MS_OnServerStart(void);
extern void MS_OnServerEnd(void);
extern void MS_OnServerUpdate(void);
extern void MS_OnServerStateChanged(void);

// Common Serverlist Enumeration
extern void MS_SendHeartbeat(INDEX iChallenge);

extern void MS_EnumTrigger(BOOL bInternet);
extern void MS_EnumUpdate(void);
extern void MS_EnumCancel(void);
//

// GameAgent Master Server
extern void GameAgent_BuildHearthbeatPacket(CTString &strPacket, INDEX iChallenge);
extern void GameAgent_EnumTrigger(BOOL bInternet);
extern void GameAgent_EnumUpdate(void);
extern void GameAgent_ServerParsePacket(INDEX iLength);

// Legacy Master Server
extern void MSLegacy_BuildHearthbeatPacket(CTString &strPacket);
extern void MSLegacy_EnumTrigger(BOOL bInternet);
extern void MSLegacy_EnumUpdate(void);
extern void MSLegacy_ServerParsePacket(INDEX iLength);

// DarkPlaces Master Server
extern void DarkPlaces_BuildHearthbeatPacket(CTString &strPacket);
extern void DarkPlaces_EnumTrigger(BOOL bInternet);
extern void DarkPlaces_EnumUpdate(void);
extern void DarkPlaces_ServerParsePacket(INDEX iLength);

void *_MS_Thread(void *lpParam);
void *_LocalNet_Thread(void *lpParam);

/// Server request structure. Primarily used for getting server pings.
class CServerRequest
{
  public:
    ULONG sr_ulAddress;
    USHORT sr_iPort;
    long long sr_tmRequestTime;

  public:
    CServerRequest(void);
    ~CServerRequest(void);

    /* Destroy all objects, and reset the array to initial (empty) state. */
    void Clear(void);
};

#endif // include once check
