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

//#pragma comment(lib, "wsock32.lib")

//WSADATA* _wsaData = NULL;
int _socket = NULL;

sockaddr_in* _sin = NULL;
sockaddr_in* _sinLocal = NULL;
sockaddr_in _sinFrom;

CHAR* _szBuffer = NULL;
CHAR* _szIPPortBuffer = NULL;
INT   _iIPPortBufferLen = 0;
CHAR* _szIPPortBufferLocal = NULL;
INT   _iIPPortBufferLocalLen = 0;

BOOL _bServer = FALSE;
BOOL _bInitialized = FALSE;
BOOL _bActivated = FALSE;
BOOL _bActivatedLocal = FALSE;

TIME _tmLastHeartbeat = -1.0F;

CDynamicStackArray<CServerRequest> ga_asrRequests;

CTString ms_strServer = "master.333networks.com";
CTString ms_strMSLegacy = "master.333networks.com";
CTString ms_strDarkPlacesMS = "192.168.1.4";

CTString ms_strGameName = "serioussamse";

BOOL ms_bMSLegacy = TRUE;
BOOL ms_bDarkPlacesMS = FALSE;
BOOL ms_bDarkPlacesDebug = FALSE;

void _uninitWinsock();

void _initializeWinsock(void)
{
  if (_socket != NULL) {
    CPrintF("Error initializing socket! Closing\n");
    _uninitWinsock();
    return;
  }

 // _wsaData = new WSADATA;
  _socket = NULL;

  // make the buffer that we'll use for packet reading
  if (_szBuffer != NULL) {
    delete[] _szBuffer;
  }
  _szBuffer = new char[2050];

  // get the host IP
  hostent* phe;
  
  if (ms_bDarkPlacesMS) {
    phe = gethostbyname(ms_strDarkPlacesMS);
  } else if (!ms_bMSLegacy) {
    phe = gethostbyname(ms_strServer);
  } else {
    phe = gethostbyname(ms_strMSLegacy);
  }

  // if we couldn't resolve the hostname
  if (phe == NULL) {
    // report and stop
    CPrintF("Couldn't resolve GameAgent server %s.\n", ms_strServer);
	close(_socket);
    _uninitWinsock();
    return;
  }

  // create the socket destination address
  _sin = new sockaddr_in;
  _sin->sin_family = AF_INET;
  _sin->sin_addr.s_addr = *(ULONG*)phe->h_addr_list[0];
  
  // [SSE]
  if (ms_bDarkPlacesMS) {
    _sin->sin_port = htons(27950);
  } else if (!ms_bMSLegacy) {
    _sin->sin_port = htons(9005);
  } else {
    _sin->sin_port = htons(27900);
  }

  // create the socket
  _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // if we're a server
  if (_bServer) {
    // create the local socket source address
    
    // set non blocking
    int flags = fcntl(_socket, F_GETFL);
    int failed = flags;
    if (failed != -1) {
	flags |= O_NONBLOCK;
	failed = fcntl(_socket, F_SETFL, flags);
      }

	  if (failed == -1) {
		ThrowF_t(TRANS("Cannot set socket to non-blocking mode."));
	  }
	
	struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10;
    setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));
    int flagTrue = 1;
    setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &flagTrue, sizeof(flagTrue));
	
	INDEX port = _pShell->GetINDEX("net_iPort") + 1;
    _sinLocal = new sockaddr_in;
    _sinLocal->sin_family = AF_INET;
    _sinLocal->sin_addr.s_addr = INADDR_ANY;
    _sinLocal->sin_port = htons(port);
	
    int optval = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0)
    {
      CPrintF("Error while allowing UDP broadcast receiving for socket.");
	  close(_socket);
      _uninitWinsock();
      return;
    }

    // bind the socket
    int res = bind(_socket, (const struct sockaddr *)_sinLocal, sizeof(*_sinLocal));
	  if (res < 0) {
        ThrowF_t(TRANS("Cannot bind port %li. %s (%i)"), port, std::strerror(errno), errno);
      }
  }
}

void _uninitWinsock()
{
  //if (_wsaData != NULL) {
 //   delete _wsaData;
 //   _wsaData = NULL;
 // }
  _socket = NULL;
}

void _sendPacketTo(const char* pubBuffer, INDEX iLen, sockaddr_in* sin)
{
  sendto(_socket, pubBuffer, iLen, 0, (const struct sockaddr*)sin, sizeof(sockaddr_in));
}

void _sendPacketTo(const char* szBuffer, sockaddr_in* addsin)
{
  sendto(_socket, szBuffer, strlen(szBuffer), 0, (const struct sockaddr*)addsin, sizeof(sockaddr_in));
}

void _sendPacket(const char* pubBuffer, INDEX iLen)
{
  _initializeWinsock();
  _sendPacketTo(pubBuffer, iLen, _sin);
}

void _sendPacket(const char* szBuffer)
{
  _initializeWinsock();
  _sendPacketTo(szBuffer, _sin);
}

int _recvPacket()
{
  int fromLength = sizeof(_sinFrom);
  return recvfrom(_socket, _szBuffer, 2048, 0, (struct sockaddr*)&_sinFrom, &fromLength);
}

CTString _getGameModeName(INDEX iGameMode)
{
  // get function that will provide us the info about gametype
  CShellSymbol *pss = _pShell->GetSymbol("GetGameTypeNameSS", /*bDeclaredOnly=*/ TRUE);

  if (pss == NULL) {
    return "";
  }

  CTString (*pFunc)(INDEX) = (CTString (*)(INDEX))pss->ss_pvValue;
  return pFunc(iGameMode);
}

CTString _getGameModeShortName(INDEX iGameMode)
{
  // get function that will provide us the info about gametype
  CShellSymbol *pss = _pShell->GetSymbol("GetGameTypeShortNameSS", /*bDeclaredOnly=*/ TRUE);

  if (pss == NULL) {
    return "";
  }

  CTString (*pFunc)(INDEX) = (CTString (*)(INDEX))pss->ss_pvValue;
  return pFunc(iGameMode);
}

const CSessionProperties* _getSP()
{
  return ((const CSessionProperties *)_pNetwork->GetSessionProperties());
}

extern void MS_SendHeartbeat(INDEX iChallenge)
{
  CTString strPacket;
  
  // [SSE]
  if (ms_bDarkPlacesMS) {
    DarkPlaces_BuildHearthbeatPacket(strPacket);

  // GameAgent
  } else if (!ms_bMSLegacy) {
    GameAgent_BuildHearthbeatPacket(strPacket, iChallenge);

  // MSLegacy
  } else {
    MSLegacy_BuildHearthbeatPacket(strPacket);
  }

  _sendPacket(strPacket);
  _tmLastHeartbeat = _pTimer->GetRealTimeTick();
}

extern void _setStatus(const CTString &strStatus)
{
  _pNetwork->ga_bEnumerationChange = TRUE;
  _pNetwork->ga_strEnumerationStatus = strStatus;
}

CServerRequest::CServerRequest(void)
{
  Clear();
}

CServerRequest::~CServerRequest(void) { }

void CServerRequest::Clear(void)
{
  sr_ulAddress = 0;
  sr_iPort = 0;
  sr_tmRequestTime = 0;
}

// --------------------------------------------------------------------------------------
// Called on every network server startup.
// --------------------------------------------------------------------------------------
extern void MS_OnServerStart(void)
{
  // join
  _bServer = TRUE;
  _bInitialized = TRUE;
  
  // [SSE]
  if (ms_bDarkPlacesMS) {
    CTString strPacket;
    strPacket.PrintF("\xFF\xFF\xFF\xFFheartbeat DarkPlaces\x0A");
    
    _sendPacket(strPacket);
    
    return;
  }
  //

  // GameAgent
  if (!ms_bMSLegacy) {
    _sendPacket("q");
    
  // MSLegacy
  //} else {
  //  CTString strPacket;
  //  strPacket.PrintF("\\heartbeat\\%hu\\gamename\\serioussamse", (_pShell->GetINDEX("net_iPort") + 1));
  //  _sendPacket(strPacket);
  }
}

// --------------------------------------------------------------------------------------
// Called if server has been stopped.
// --------------------------------------------------------------------------------------
extern void MS_OnServerEnd(void)
{
  if (!_bInitialized) {
    return;
  }

  if (ms_bDarkPlacesMS) {
    MS_SendHeartbeat(0);
    MS_SendHeartbeat(0);
    // TODO: Write here something
  } else if (ms_bMSLegacy) {
    CTString strPacket;
    strPacket.PrintF("\\heartbeat\\%hu\\gamename\\serioussamse\\statechanged", (_pShell->GetINDEX("net_iPort") + 1));
    _sendPacket(strPacket);
  }
   close(_socket);
  _uninitWinsock();
  _bInitialized = FALSE;
}

// --------------------------------------------------------------------------------------
// Regular network server update.
// Responds to enumeration pings and sends pings to masterserver.
// --------------------------------------------------------------------------------------
extern void MS_OnServerUpdate(void)
{
  if ((_socket == NULL) || (!_bInitialized)) {
    return;
  }

  memset(&_szBuffer[0], 0, 2050);
  INDEX iLength = _recvPacket();

  if (iLength > 0)
  {
    // [SSE]
    if (ms_bDarkPlacesMS) {
      DarkPlaces_ServerParsePacket(iLength);
    } else if (!ms_bMSLegacy) {
      GameAgent_ServerParsePacket(iLength);
    } else {
      _szBuffer[iLength] = 0;
      MSLegacy_ServerParsePacket(iLength);
    }
 }

 // send a heartbeat every 150 seconds
 if (_pTimer->GetRealTimeTick() - _tmLastHeartbeat >= 150.0f) {
    MS_SendHeartbeat(0);
 }
}

// --------------------------------------------------------------------------------------
// Notify master server that the server state has changed.
// --------------------------------------------------------------------------------------
extern void MS_OnServerStateChanged(void)
{
  if (!_bInitialized) {
    return;
  }

  if (ms_bDarkPlacesMS) {
    // TODO: Write here something
  } else if (!ms_bMSLegacy) {
    _sendPacket("u");
  } else {
    CTString strPacket;
    strPacket.PrintF("\\heartbeat\\%hu\\gamename\\serioussamse\\statechanged", (_pShell->GetINDEX("net_iPort") + 1));
    _sendPacket(strPacket);
  }
}

// --------------------------------------------------------------------------------------
// Request serverlist enumeration. Sends request packet.
// --------------------------------------------------------------------------------------
extern void MS_EnumTrigger(BOOL bInternet)
{
  if (_pNetwork->ga_bEnumerationChange) {
    return;
  }
  
  if (ms_bDarkPlacesMS) {
    DarkPlaces_EnumTrigger(bInternet);
    return; 
  }
  
  if (ms_bMSLegacy) {
    MSLegacy_EnumTrigger(bInternet);
    return;
  }
  
  GameAgent_EnumTrigger(bInternet);
}

// --------------------------------------------------------------------------------------
// Client update for enumerations.
// --------------------------------------------------------------------------------------
extern void MS_EnumUpdate(void)
{
  if ((_socket == NULL) || (!_bInitialized)) {
    return;
  }

  // [SSE]
  if (ms_bDarkPlacesMS) {
    DarkPlaces_EnumUpdate();
  
  // GameAgent
  } else if (!ms_bMSLegacy) {
    GameAgent_EnumUpdate();

  // MSLegacy
  } else {
    MSLegacy_EnumUpdate();
  }
}

// --------------------------------------------------------------------------------------
// Cancel the master server serverlist enumeration.
// --------------------------------------------------------------------------------------
extern void MS_EnumCancel(void)
{
  if (_bInitialized) {
    CPrintF("...MS_EnumCancel!\n");
    ga_asrRequests.Clear();
	close(_socket);
    _uninitWinsock();
  }
}
