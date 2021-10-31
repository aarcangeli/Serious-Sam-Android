/* Copyright (c) 2017 ZCaliptium

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

#include <Engine/StdH.h>

#include <Engine/Engine.h>
#include <Engine/CurrentVersion.h>
#include <Engine/Entities/Entity.h>
#include <Engine/Base/Shell.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/CTString.h>
#include <Engine/Network/Server.h>
#include <Engine/Network/Network.h>
#include <Engine/Network/SessionState.h>
#include <GameMP/SessionProperties.h> // TODO: GET RID OF THIS!
#include <Engine/revision.h>

#include <Engine/Query/MasterServerMgr.h>

#define DP_NET_PROTOCOL_VERSION 3

extern CTString _getGameModeShortName(INDEX iGameMode);
extern const CSessionProperties* _getSP();

extern sockaddr_in _sinFrom;
extern CHAR* _szBuffer;

extern BOOL _bServer;
extern BOOL _bInitialized;
extern BOOL _bActivated;
extern BOOL _bActivatedLocal;

extern void _sendPacket(const char* szBuffer);
extern void _sendPacket(const char* pubBuffer, INDEX iLen);
extern void _sendPacketTo(const char* szBuffer, sockaddr_in* addsin);
extern void _sendPacketTo(const char* pubBuffer, INDEX iLen, sockaddr_in* sin);
extern void _setStatus(const CTString &strStatus);
extern int _recvPacket();

extern CDynamicStackArray<CServerRequest> ga_asrRequests;

// --------------------------------------------------------------------------------------
// Builds hearthbeat packet.
// --------------------------------------------------------------------------------------
void DarkPlaces_BuildHearthbeatPacket(CTString &strPacket)
{
  strPacket.PrintF("\xFF\xFF\xFF\xFFheartbeat DarkPlaces\x0A");
}

// --------------------------------------------------------------------------------------
// Builds game status string.
// --------------------------------------------------------------------------------------
void DarkPlaces_BuildQCStatus(CTString &strStatus)
{
  INDEX ctFreeSlots = _pNetwork->ga_sesSessionState.ses_ctMaxPlayers - _pNetwork->ga_srvServer.GetClientsCount();
  
  strStatus.PrintF("%s:%s:P%d:S%d:F%d:M%s::score!!", _getGameModeShortName(_getSP()->sp_gmGameMode), "0.8.2", 0, ctFreeSlots, 0, ms_strGameName);
}

// --------------------------------------------------------------------------------------
// Builds status string.
// Reply to getStatus or getInfo
// --------------------------------------------------------------------------------------
void DarkPlaces_BuildStatusResponse(const char* challenge, CTString &strPacket, BOOL bFullStatus)
{
  CTString strStatus;
  DarkPlaces_BuildQCStatus(strStatus);
  
  INDEX ctMaxPlayers = _pNetwork->ga_sesSessionState.ses_ctMaxPlayers;
  
  strPacket.PrintF("\xFF\xFF\xFF\xFF%s\x0A"
            "\\gamename\\%s\\modname\\%s\\gameversion\\%d\\sv_maxclients\\%d"
            "\\clients\\%d\\bots\\%d\\mapname\\%s\\hostname\\%s\\protocol\\%d"
            "%s%s"
            "%s%s"
            "%s%s"
            "%s",
            bFullStatus ? "statusResponse" : "infoResponse",
            ms_strGameName, "", _ulEngineRevision, ctMaxPlayers,
            _pNetwork->ga_srvServer.GetClientsCount(), 0, _pNetwork->ga_World.wo_strName, _pShell->GetString("gam_strSessionName"), DP_NET_PROTOCOL_VERSION,
            "\\qcstatus\\", strStatus,
            challenge ? "\\challenge\\" : "", challenge ? challenge : "",
            "", "", // Crypto Key
            "");
}

// --------------------------------------------------------------------------------------
// Process packets while running server.
// --------------------------------------------------------------------------------------
void DarkPlaces_ServerParsePacket(INDEX iLength)
{
  char *string = &_szBuffer[0];
  unsigned char *data = (unsigned char*)&_szBuffer[0];
  
  if (iLength >= 5 && data[0] == 0xFF && data[1] == 0xFF && data[2] == 0xFF && data[3] == 0xFF)
  {
    data += 4;
    string += 4;
    iLength -= 4;
    
    //CPrintF("Received DarkPlaces text command!\n");
    
    // Short info.
    if (iLength >= 7 && !memcmp(data, "getinfo", 7))
    {
      if (ms_bDarkPlacesDebug) {
        CPrintF("Received 'getinfo' text command!\n");
      }

      const char *challenge = NULL;
      
			if (iLength > 8 && string[7] == ' ')
				challenge = string + 8;
      
      //CPrintF("Received 'getinfo' text command!\n");
      
      CTString strPacket;
      
      DarkPlaces_BuildStatusResponse(challenge, strPacket, false);

      _sendPacket(strPacket);
      return;
    }

    // Full status.
    if (iLength >= 9 && !memcmp(string, "getstatus", 9))
    {
      if (ms_bDarkPlacesDebug) {
        CPrintF("Received 'getstatus' text command!\n");
      }

      const char *challenge = NULL;
      
			if (iLength > 10 && string[9] == ' ')
				challenge = string + 10;
      
      CTString strPacket;

      DarkPlaces_BuildStatusResponse(challenge, strPacket, true);

      _sendPacketTo(strPacket, &_sinFrom);
      
      return;
    }
    
    if (iLength >= 12 && !memcmp(string, "getchallenge", 12))
    {
      CTString strPacket;
      
      strPacket.PrintF("\xFF\xFF\xFF\xFFreject Wrong door!");
      
      _sendPacketTo(strPacket, &_sinFrom);
      
      if (ms_bDarkPlacesDebug) {
        CPrintF("Received 'getchallenge' text command!\n");
      }
      
      return;
    }
    
    if (ms_bDarkPlacesDebug) {
      CPrintF("Received unknown text command!\n");
      CPrintF("Data[%d]: %s\n", iLength, data);    
    }
  }
  //CPrintF("Received: %s\n", &_szBuffer[0]);
}

// --------------------------------------------------------------------------------------
// Parse server-list received from the Master Server.
// --------------------------------------------------------------------------------------
void DarkPlaces_ParseServerList(unsigned char *data, INDEX iLength, BOOL bExtended)
{
  if (ms_bDarkPlacesDebug) {
    CPrintF("Data Length: %d\n", iLength);
  }
  
  while (iLength >= 7)
	{
    // IPV4 Address.
    if (data[0] == '\\') {
      
      unsigned short uPort = data[5] * 256 + data[6];
      
      if (uPort != 0 && (data[1] != 0xFF || data[2] != 0xFF || data[3] != 0xFF || data[4] != 0xFF))
      {
        CTString strIP;
        strIP.PrintF("%u.%u.%u.%u", data[1], data[2], data[3], data[4]);
    
        if (ms_bDarkPlacesDebug) {
          CPrintF("%s:%hu\n", strIP, uPort);
        }
        
        sockaddr_in sinServer;
        sinServer.sin_family = AF_INET;
        sinServer.sin_addr.s_addr = inet_addr(strIP);
        sinServer.sin_port = htons(uPort);
  
        // insert server status request into container
        CServerRequest &sreq = ga_asrRequests.Push();
        sreq.sr_ulAddress = sinServer.sin_addr.s_addr;
        sreq.sr_iPort = sinServer.sin_port;
        sreq.sr_tmRequestTime = _pTimer->GetHighPrecisionTimer().GetMilliseconds();
  
        // send packet to server
        _sendPacketTo("\xFF\xFF\xFF\xFFgetstatus", &sinServer);
      }
      
			data += 7;
			iLength -= 7;

    // IPV6 Address.
    } else if (iLength >= 19 && data[0] == '/' && bExtended) {
      
      // Just skip it. Because our game don't have IPV6 support.
			data += 19;
			iLength -= 19;
      
    // Unknown Data.
    } else {
      if (ms_bDarkPlacesDebug) {
        CPrintF("Error! Unknown data while parsing server list!\n");
      }
      break;
    }
  }
}

// --------------------------------------------------------------------------------------
// Process packets while running client.
// --------------------------------------------------------------------------------------
void DarkPlaces_ClientParsePacket(INDEX iLength)
{
  char *string = &_szBuffer[0];
  unsigned char *data = (unsigned char*)&_szBuffer[0];
  
  if (iLength >= 5 && data[0] == 0xFF && data[1] == 0xFF && data[2] == 0xFF && data[3] == 0xFF)
  {
    data += 4;
    string += 4;
    iLength -= 4;
    
    // Regular Servers Response.
    if (iLength >= 19 && !memcmp(string, "getserversResponse\\", 19))
    {
      if (ms_bDarkPlacesDebug) {
        CPrintF("Received 'getserversResponse\\'!\n");
      }

      data += 18;
      string += 18;
      iLength -= 18;

      DarkPlaces_ParseServerList(data, iLength, FALSE);

      return;
    }
    
    // Extended Servers Response.
    if (iLength >= 21 && !memcmp(string, "getserversExtResponse", 21))
    {
      if (ms_bDarkPlacesDebug) {
        CPrintF("Received 'getserversExtResponse'!\n");
      }

      data += 21;
      string += 21;
      iLength -= 21;

      DarkPlaces_ParseServerList(data, iLength, TRUE);

      return;
    }
    
    // Status Response from one of servers from Server List
    if (iLength >= 15 && !memcmp(string, "statusResponse\x0A", 15))
    {
      if (ms_bDarkPlacesDebug) {
        CPrintF("Received 'statusResponse'!\n");
      }
      
      data += 15;
      string += 15;
      iLength -= 15;

      if (ms_bDarkPlacesDebug) {
        CPrintF("Data[%d]: %s\n", iLength, data);
      }  

      CNetworkSession &ns = *new CNetworkSession;
      _pNetwork->ga_lhEnumeratedSessions.AddTail(ns.ns_lnNode);
      
      BOOL bReadValue = FALSE;
      CTString strKey;
      CTString strValue;
      
      CTString strPlayers;
      CTString strMaxPlayers;
      CTString strLevel;
      CTString strGameType;
      CTString strVersion;
      CTString strGameName;
      CTString strSessionName;

      CTString strGamePort;
      CTString strServerLocation;
      CTString strGameMode;
      CTString strActiveMod;
      
      // Skip first separator!
      if (*data == '\\') {
        data += 1;
        string += 1;
        iLength -= 1;
      }

      while (*data != 0)
      {
        switch (data[0])
        {
          case '\\': {
            //if (strKey != "gamemode") {
              if (bReadValue) {
                //CPrintF("  %s = %s\n", strKey, strValue);
                
                // we're done reading the value, check which key it was
                if (strKey == "gamename") {
                    strGameName = strValue;
                } else if (strKey == "gameversion") {
                    strVersion = strValue;
                } else if (strKey == "location") {
                    strServerLocation = strValue;
                } else if (strKey == "hostname") {
                    strSessionName = strValue;
                } else if (strKey == "hostport") {
                    strGamePort = strValue;
                } else if (strKey == "qcstatus") {
                    strGameMode = strValue;
                } else if (strKey == "mapname") {
                    strLevel = strValue;
                } else if (strKey == "modname") {
                    strActiveMod = strValue;
                } else if (strKey == "clients") {
                    strPlayers = strValue;
                } else if (strKey == "bots") {
                } else if (strKey == "protocol") {
                } else if (strKey == "sv_maxclients") {
                    strMaxPlayers = strValue;
                } else {
                  if (ms_bDarkPlacesDebug) {
                    CPrintF("Unknown DarkPlaces parameter key '%s'!\n", strKey);
                  }
                }
                // reset temporary holders
                strKey = "";
                strValue = "";
              }
            //}

            bReadValue = !bReadValue;
          } break;
          
          default: {
            
            // read into the value or into the key, depending where we are
            if (bReadValue) {
              strValue.InsertChar(strlen(strValue), *data);
            } else {
              strKey.InsertChar(strlen(strKey), *data);
            }
          }
        }
        
        data += 1;
        iLength -= 1;
      }
      
      if (strGameMode != "")
      {
        CTString strDummy;

        for (INDEX i = 0; i < strGameMode.Length(); i++)
        {
          if (strGameMode.str_String[i] == ':') {
            strGameMode.Split(i, strGameType, strDummy);
            break;
          }
        }
      }

      // add the server to the serverlist
      ns.ns_strSession = strSessionName;
      ns.ns_strAddress = inet_ntoa(_sinFrom.sin_addr) + CTString(":") + CTString(0, "%d", htons(_sinFrom.sin_port) - 1);
      ns.ns_tmPing = (10000.0F / 1000.0f);
      ns.ns_strWorld = strLevel;
      ns.ns_ctPlayers = atoi(strPlayers);
      ns.ns_ctMaxPlayers = atoi(strMaxPlayers);
      ns.ns_strGameType = strGameType;
      ns.ns_strMod = strGameName;
      ns.ns_strVer = strVersion;
      
      return;
    }

    if (ms_bDarkPlacesDebug) {
      CPrintF("Received unknown text command!\n");
      CPrintF("Data[%d]: %s\n", iLength, data);   
    }
  }
}

void DarkPlaces_EnumTrigger(BOOL bInternet)
{
  // Make sure that there are no requests still stuck in buffer.
  ga_asrRequests.Clear();
  
  // We're not a server.
  _bServer = FALSE;
  // Initialization.
  _bInitialized = TRUE;
  
  CTString strPacket;
  strPacket.PrintF("\xFF\xFF\xFF\xFFgetservers %s %u empty full", ms_strGameName, DP_NET_PROTOCOL_VERSION);
  _sendPacket(strPacket); // Send enumeration packet to masterserver.

  _setStatus(".");
}

void DarkPlaces_EnumUpdate(void)
{
  int iLength = _recvPacket();

  if (iLength == -1) {
    return;
  }
  
  DarkPlaces_ClientParsePacket(iLength);
}
