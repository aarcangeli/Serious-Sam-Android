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

#include <Engine/Query/MasterServerMgr.h>

extern const CSessionProperties* _getSP();
extern CTString _getGameModeName(INDEX iGameMode);

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
extern void GameAgent_BuildHearthbeatPacket(CTString &strPacket, INDEX iChallenge)
{
  strPacket.PrintF("0;challenge;%d;players;%d;maxplayers;%d;level;%s;gametype;%s;version;%s;product;%s",
      iChallenge,
      _pNetwork->ga_srvServer.GetPlayersCount(),
      _pNetwork->ga_sesSessionState.ses_ctMaxPlayers,
      _pNetwork->ga_World.wo_strName,
      _getGameModeName(_getSP()->sp_gmGameMode),
      _SE_VER_STRING,
      ms_strGameName);
}

extern void GameAgent_ServerParsePacket(INDEX iLength)
{
  // check the received packet ID
  switch (_szBuffer[0])
  {
    case 1: // server join response
    {
      int iChallenge = *(INDEX*)(_szBuffer + 1);
      // send the challenge
      MS_SendHeartbeat(iChallenge);
      break;
    }

    case 2: // server status request
    {
      // send the status response
      CTString strPacket;
      strPacket.PrintF("0;players;%d;maxplayers;%d;level;%s;gametype;%s;version;%s;gamename;%s;sessionname;%s",
        _pNetwork->ga_srvServer.GetPlayersCount(),
        _pNetwork->ga_sesSessionState.ses_ctMaxPlayers,
        _pNetwork->ga_World.wo_strName,
        _getGameModeName(_getSP()->sp_gmGameMode),
        _SE_VER_STRING,
        ms_strGameName,
        _pShell->GetString("gam_strSessionName"));
      _sendPacketTo(strPacket, &_sinFrom);
      break;
    }

    case 3: // player status request
    {
      // send the player status response
      CTString strPacket;
      strPacket.PrintF("\x01players\x02%d\x03", _pNetwork->ga_srvServer.GetPlayersCount());
      for (INDEX i=0; i<_pNetwork->ga_srvServer.GetPlayersCount(); i++) {
        CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[i];
        CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[i];
        if (plt.plt_bActive) {
          CTString strPlayer;
          plt.plt_penPlayerEntity->GetGameAgentPlayerInfo(plb.plb_Index, strPlayer);

          // if we don't have enough space left for the next player
          if (strlen(strPacket) + strlen(strPlayer) > 2048) {
            // send the packet
            _sendPacketTo(strPacket, &_sinFrom);
            strPacket = "";
          }

          strPacket += strPlayer;
        }
      }

      strPacket += "\x04";
      _sendPacketTo(strPacket, &_sinFrom);
      break;
    }

    case 4: // ping
    {
      // just send back 1 byte and the amount of players in the server (this could be useful in some cases for external scripts)
      CTString strPacket;
      strPacket.PrintF("\x04%d", _pNetwork->ga_srvServer.GetPlayersCount());
      _sendPacketTo(strPacket, &_sinFrom);
      break;
    }
  }
}

extern void GameAgent_EnumTrigger(BOOL bInternet)
{
  // Make sure that there are no requests still stuck in buffer.
  ga_asrRequests.Clear();

  // We're not a server.
  _bServer = FALSE;
  // Initialization.
  _bInitialized = TRUE;
  // Send enumeration packet to masterserver.
  _sendPacket("e");
  _setStatus(".");
}

void GameAgent_ClientParsePacket(INDEX iLength)
{
  switch (_szBuffer[0])
  {
    case 's':
    {
      struct sIPPort {
        UBYTE bFirst;
        UBYTE bSecond;
        UBYTE bThird;
        UBYTE bFourth;
        USHORT iPort;
      };

      _pNetwork->ga_strEnumerationStatus = "";
  
      sIPPort* pServers = (sIPPort*)(_szBuffer + 1);

      while(iLength - ((CHAR*)pServers - _szBuffer) >= sizeof(sIPPort)) {
        sIPPort ip = *pServers;
  
        CTString strIP;
        strIP.PrintF("%d.%d.%d.%d", ip.bFirst, ip.bSecond, ip.bThird, ip.bFourth);
  
        sockaddr_in sinServer;
        sinServer.sin_family = AF_INET;
        sinServer.sin_addr.s_addr = inet_addr(strIP);
        sinServer.sin_port = htons(ip.iPort + 1);
  
        // insert server status request into container
        CServerRequest &sreq = ga_asrRequests.Push();
        sreq.sr_ulAddress = sinServer.sin_addr.s_addr;
        sreq.sr_iPort = sinServer.sin_port;
        sreq.sr_tmRequestTime = _pTimer->GetHighPrecisionTimer().GetMilliseconds();
  
        // send packet to server
        _sendPacketTo("\x02", &sinServer);
  
        pServers++;
      }
    } break;
    
    case '0':
    {
      CTString strPlayers;
      CTString strMaxPlayers;
      CTString strLevel;
      CTString strGameType;
      CTString strVersion;
      CTString strGameName;
      CTString strSessionName;
  
      CHAR* pszPacket = _szBuffer + 2; // we do +2 because the first character is always ';', which we don't care about.
  
      BOOL bReadValue = FALSE;
      CTString strKey;
      CTString strValue;
  
      while(*pszPacket != 0)
      {
        switch (*pszPacket)
        {
          case ';': {
            if (strKey != "sessionname") {
              if (bReadValue) {
                // we're done reading the value, check which key it was
                if (strKey == "players") {
                  strPlayers = strValue;
                } else if (strKey == "maxplayers") {
                  strMaxPlayers = strValue;
                } else if (strKey == "level") {
                  strLevel = strValue;
                } else if (strKey == "gametype") {
                  strGameType = strValue;
                } else if (strKey == "version") {
                  strVersion = strValue;
                } else if (strKey == "gamename") {
                  strGameName = strValue;
                } else {
                  CPrintF("Unknown GameAgent parameter key '%s'!", strKey);
                }
    
                // reset temporary holders
                strKey = "";
                strValue = "";
              }
            }

            bReadValue = !bReadValue;
          } break;
  
          default: {
            // read into the value or into the key, depending where we are
            if (bReadValue) {
              strValue.InsertChar(strlen(strValue), *pszPacket);
            } else {
              strKey.InsertChar(strlen(strKey), *pszPacket);
            }
          } break;
        }
  
        // move to next character
        pszPacket++;
      }
  
      // check if we still have a sessionname to back up
      if (strKey == "sessionname") {
        strSessionName = strValue;
      }
  
      // insert the server into the serverlist
      CNetworkSession &ns = *new CNetworkSession;
      _pNetwork->ga_lhEnumeratedSessions.AddTail(ns.ns_lnNode);
  
      long long tmPing = -1;

      // find the request in the request array
      for (INDEX i = 0; i < ga_asrRequests.Count(); i++)
      {
        CServerRequest &req = ga_asrRequests[i];

        if (req.sr_ulAddress == _sinFrom.sin_addr.s_addr && req.sr_iPort == _sinFrom.sin_port) {
          tmPing = _pTimer->GetHighPrecisionTimer().GetMilliseconds() - req.sr_tmRequestTime;
          ga_asrRequests.Delete(&req);
          break;
        }
      }
  
      if (tmPing == -1) {
        // server status was never requested
        break;
      }
  
      // add the server to the serverlist
      ns.ns_strSession = strSessionName;
      ns.ns_strAddress = inet_ntoa(_sinFrom.sin_addr) + CTString(":") + CTString(0, "%d", htons(_sinFrom.sin_port) - 1);
      ns.ns_tmPing = (tmPing / 1000.0f);
      ns.ns_strWorld = strLevel;
      ns.ns_ctPlayers = atoi(strPlayers);
      ns.ns_ctMaxPlayers = atoi(strMaxPlayers);
      ns.ns_strGameType = strGameType;
      ns.ns_strMod = strGameName;
      ns.ns_strVer = strVersion;
    } break;
    
    default: {
      CPrintF("Unknown enum packet ID %x!\n", _szBuffer[0]);
    } break;
  }
}

extern void GameAgent_EnumUpdate(void)
{
  int iLength = _recvPacket();

  if (iLength == -1) {
    return;
  }

  _szBuffer[iLength] = 0; // Terminate the buffer with NULL.

  GameAgent_ClientParsePacket(iLength);
}