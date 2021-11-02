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

#include <unistd.h>
#include <pthread.h>

#include <Engine/Query/MasterServerMgr.h>
#include <Engine/Query/MSLegacy.h>

#define MSPORT      28900
#define BUFFSZ      8192
#define BUFFSZSTR   4096

#define INVALID_SOCKET  (-1)
#define HOSTENT hostent
#define SOCKET_ERROR            (-1)

#define CHK_BUFFSTRLEN if ((iLen < 0) || (iLen > BUFFSZSTR)) { \
                        CPrintF("\n" \
                            "Error: the used buffer is smaller than how much needed (%d < %d)\n" \
                            "\n", iLen, BUFFSZSTR); \
                            if (cMsstring) free (cMsstring); \
                            close(_sock); \
                        }

#define CLEANMSSRUFF1       close(_sock);

#define CLEANMSSRUFF2       if (cResponse) free (cResponse); \
                            close(_sock);

#define SERIOUSSAMKEY       "AKbna4\0"
#define SERIOUSSAMSTR       "serioussamse"

#define PCK         "\\gamename\\%s" \
                    "\\enctype\\%d" \
                    "\\validate\\%s" \
                    "\\final\\" \
                    "\\queryid\\1.1" \
                    "\\list\\cmp" \
                    "\\gamename\\%s" \
                    "\\gamever\\1.05" \
                    "%s%s" \
                    "\\final\\"

#define PCKQUERY    "\\gamename\\%s" \
                    "\\gamever\\%s" \
                    "\\location\\%s" \
                    "\\hostname\\%s" \
                    "\\hostport\\%hu" \
                    "\\mapname\\%s" \
                    "\\gametype\\%s" \
                    "\\activemod\\" \
                    "\\numplayers\\%d" \
                    "\\maxplayers\\%d" \
                    "\\gamemode\\openplaying" \
                    "\\difficulty\\Normal" \
                    "\\friendlyfire\\%d" \
                    "\\weaponsstay\\%d" \
                    "\\ammosstay\\%d" \
                    "\\healthandarmorstays\\%d" \
                    "\\allowhealth\\%d" \
                    "\\allowarmor\\%d" \
                    "\\infiniteammo\\%d" \
                    "\\respawninplace\\%d" \
                    "\\password\\0" \
                    "\\vipplayers\\1"

#define PCKINFO     "\\hostname\\%s" \
                    "\\hostport\\%hu" \
                    "\\mapname\\%s" \
                    "\\gametype\\%s" \
                    "\\numplayers\\%d" \
                    "\\maxplayers\\%d" \
                    "\\gamemode\\openplaying" \
                    "\\final\\" \
                    "\\queryid\\8.1"

#define PCKBASIC    "\\gamename\\%s" \
                    "\\gamever\\%s" \
                    "\\location\\EU" \
                    "\\final\\" \
                    "\\queryid\\1.1"

extern const CSessionProperties* _getSP();
extern CTString _getGameModeName(INDEX iGameMode);

extern sockaddr_in _sinFrom;
extern CHAR* _szBuffer;
extern CHAR* _szIPPortBuffer;
extern INT   _iIPPortBufferLen;
extern CHAR* _szIPPortBufferLocal;
extern INT   _iIPPortBufferLocalLen;

extern BOOL _bServer;
extern BOOL _bInitialized;
extern BOOL _bActivated;
extern BOOL _bActivatedLocal;

extern void _initializeWinsock(void);
extern void _uninitWinsock();
extern void _sendPacket(const char* szBuffer);
extern void _sendPacket(const char* pubBuffer, INDEX iLen);
extern void _sendPacketTo(const char* szBuffer, sockaddr_in* addsin);
extern void _sendPacketTo(const char* pubBuffer, INDEX iLen, sockaddr_in* sin);
extern void _setStatus(const CTString &strStatus);

extern CDynamicStackArray<CServerRequest> ga_asrRequests;

// --------------------------------------------------------------------------------------
// Builds hearthbeat packet.
// --------------------------------------------------------------------------------------
void MSLegacy_BuildHearthbeatPacket(CTString &strPacket)
{
  strPacket.PrintF("\\heartbeat\\%hu\\gamename\\%s", (_pShell->GetINDEX("net_iPort") + 1), SERIOUSSAMSTR);
}

static void _LocalSearch()
{
  // make sure that there are no requests still stuck in buffer
  ga_asrRequests.Clear();

  // we're not a server
  _bServer = FALSE;
  _pNetwork->ga_strEnumerationStatus = ".";

 // WORD     _wsaRequested;
  int _sock = NULL;
 // WSADATA  wsaData;
  struct hostent *_phHostinfo = NULL;
  ULONG    _uIP,*_pchIP = &_uIP;
  USHORT   _uPort,*_pchPort = &_uPort;
  INT      _iLen;
  char     _cName[256],*_pch,_strFinal[8] = {0};

  struct in_addr addr;

  // make the buffer that we'll use for packet reading
  if (_szIPPortBufferLocal != NULL) {
     return;
  }
  _szIPPortBufferLocal = new char[1024];

  // start WSA
 // _wsaRequested = MAKEWORD( 2, 2 );
  if (_sock != NULL) {
    CPrintF("Error initializing socket!\n");
    if (_szIPPortBufferLocal != NULL) {
      delete[] _szIPPortBufferLocal;
    }
    _szIPPortBufferLocal = NULL;
   	close(_sock);
    _uninitWinsock();
    _bInitialized = FALSE;
    _pNetwork->ga_bEnumerationChange = FALSE;
    _pNetwork->ga_strEnumerationStatus = "";

    return;
  }

  _pch = _szIPPortBufferLocal;
  _iLen = 0;
  strcpy(_strFinal,"\\final\\");

  if (gethostname ( _cName, sizeof(_cName)) == 0)
  {
    if ((_phHostinfo = gethostbyname(_cName)) != NULL)
    {
      int _iCount = 0;
      while(_phHostinfo->h_addr_list[_iCount])
      {
        
        addr.s_addr = *(u_long *) _phHostinfo->h_addr_list[_iCount];
        _uIP = htonl(addr.s_addr);
       
        CPrintF("%lu\n", _uIP);
        
        for (UINT uPort = 25601; uPort < 25622; ++uPort){
          _uPort = htons(uPort);
          memcpy(_pch,_pchIP,4);
          _pch  +=4;
          _iLen +=4;
          memcpy(_pch,_pchPort,2);
          _pch  +=2;
          _iLen +=2;
        }
        ++_iCount;
      }

      memcpy(_pch,_strFinal, 7);
      _pch  +=7;
      _iLen +=7;
      _pch[_iLen] = 0x00;
    }
  }

  _iIPPortBufferLocalLen = _iLen;

  _bActivatedLocal = TRUE;
  _bInitialized = TRUE;
  _initializeWinsock();
}

void MSLegacy_EnumTrigger(BOOL bInternet)
{
  // Local Search with Legacy Protocol
  if (!bInternet) {
    _LocalSearch();
    return;
	
  // Internet Search
  } else {

    // make sure that there are no requests still stuck in buffer
    ga_asrRequests.Clear();
    // we're not a server
    _bServer = FALSE;
    _pNetwork->ga_strEnumerationStatus = ".";

    struct  sockaddr_in peer;

    int  _sock = 0;
    u_int   uiMSIP;
    int     iErr,
            iLen,
            iDynsz,
            iEnctype             = 0;
    u_short usMSport             = MSPORT;

    u_char  ucGamekey[]          = {SERIOUSSAMKEY},
            ucGamestr[]          = {SERIOUSSAMSTR},
            *ucSec               = NULL,
            *ucKey               = NULL;

    char    *cFilter             = "",
            *cWhere              = "",
            cMS[128]             = {0},
            *cResponse           = NULL,
            *cMsstring           = NULL,
            *cSec                = NULL;


    strcpy(cMS, ms_strMSLegacy);

   // WSADATA wsadata;
    if (_sock != 0) {
        CPrintF("Error initializing socket!\n");
        return;
    }

    /* Open a socket and connect to the Master server */

    peer.sin_addr.s_addr = uiMSIP = resolv(cMS);
    peer.sin_port        = htons(usMSport);
    peer.sin_family      = AF_INET;

    _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (_sock < 0) {
        CPrintF("Error creating TCP socket!\n");
        return;
    }
    if (connect(_sock, (const struct sockaddr *)&peer, sizeof(peer)) < 0) {
        CPrintF("Error connecting to TCP socket!\n");
        CLEANMSSRUFF1;
        return;
    }

    /* Allocate memory for a buffer and get a pointer to it */

    cResponse = (char*) malloc(BUFFSZSTR + 1);
    if (!cResponse) {
        CPrintF("Error initializing memory buffer!\n");
        CLEANMSSRUFF1;
        return;
    }

    // Master Server should respond to the game.
    /* Reading response from Master Server - returns the string with the secret key */

    iLen = 0;
    iErr = recv(_sock, (char*)cResponse + iLen, BUFFSZSTR - iLen, 0);
    if (iErr < 0) {
        CPrintF("Error reading from TCP socket!\n");
        CLEANMSSRUFF2;
        return;
    }

    iLen += iErr;
    cResponse[iLen] = 0x00; // Terminate the response.

    /* Allocate memory for a buffer and get a pointer to it */

    ucSec = (u_char*) malloc(BUFFSZSTR + 1);
    if (!ucSec) {
        CPrintF("Error initializing memory buffer!\n");
        CLEANMSSRUFF2;
        return;
    }
    memcpy ( ucSec, cResponse,  BUFFSZSTR);
    ucSec[iLen] = 0x00;

    /* Geting the secret key from a string */

    cSec = strstr(cResponse, "\\secure\\");
    if (!cSec) {
        CPrintF("Not valid master server response!\n");
        CLEANMSSRUFF2;
        return;
    } else {
        ucSec  += 15;

    /* Creating a key for authentication (Validate key) */

        ucKey = gsseckey(ucSec, ucGamekey, iEnctype);
    }

    ucSec -= 15;
    if (cResponse) free (cResponse);
    if (ucSec) free (ucSec);

    /* Generate a string for the response (to Master Server) with the specified (Validate ucKey) */

    cMsstring = (char*) malloc(BUFFSZSTR + 1);
    if (!cMsstring) {
        CPrintF("Not valid master server response!\n");
        CLEANMSSRUFF1;
        return;
    }

    iLen = _snprintf(
        cMsstring,
        BUFFSZSTR,
        PCK,
        ucGamestr,
        iEnctype,
        ucKey,
        ucGamestr,
        cWhere,
        cFilter);

    /* Check the buffer */

    CHK_BUFFSTRLEN;

    /* The string sent to master server */

    if (send(_sock,cMsstring, iLen, 0) < 0){
        CPrintF("Error reading from TCP socket!\n");
        if (cMsstring) free (cMsstring);
        CLEANMSSRUFF1;
        return;
    }
    if (cMsstring) free (cMsstring);

    /* Allocate memory for a buffer and get a pointer to it */

    if (_szIPPortBuffer ) {
        CLEANMSSRUFF1;
        return;
    };

    _szIPPortBuffer = (char*) malloc(BUFFSZ + 1);
    if (!_szIPPortBuffer) {
        CPrintF("Error reading from TCP socket!\n");
        CLEANMSSRUFF1;
        return;
    }
    iDynsz = BUFFSZ;

    /* The received encoded data after sending the string (Validate key) */

    iLen = 0;
    while((iErr = recv(_sock, _szIPPortBuffer + iLen, iDynsz - iLen, 0)) > 0) {
        iLen += iErr;
        if (iLen >= iDynsz) {
            iDynsz += BUFFSZ;
            _szIPPortBuffer = (char*)realloc(_szIPPortBuffer, iDynsz);
            if (!_szIPPortBuffer) {
                CPrintF("Error reallocation memory buffer!\n");
                if (_szIPPortBuffer) free (_szIPPortBuffer);
                CLEANMSSRUFF1;
                return;
            }
        }
    }

    CLEANMSSRUFF1;
    _iIPPortBufferLen = iLen;

    _bActivated = TRUE;
    _bInitialized = TRUE;
    _initializeWinsock();
     
  }
}

pthread_t g_msMainThread;
pthread_t g_msLocalMainThread;

void MSLegacy_EnumUpdate(void)
{
  if (_bActivated)
  {
    int _hThread = pthread_create(&g_msMainThread, 0, &_MS_Thread, nullptr);
    if (_hThread != 0) {
      int pthread_cancel(_hThread);
    }
    _bActivated = FALSE;		
  }

  if (_bActivatedLocal)
  {
	
    int _hThread = pthread_create(&g_msLocalMainThread, 0, &_LocalNet_Thread, nullptr);
    if (_hThread != 0) {
      int pthread_cancel(_hThread);
    }

    _bActivatedLocal = FALSE;		
  }	
}

extern void MSLegacy_ServerParsePacket(INDEX iLength)
{
  unsigned char *data = (unsigned char*)&_szBuffer[0];

  char *sPch1 = NULL, *sPch2 = NULL, *sPch3 = NULL, *sPch4 = NULL, *sPch5;

  sPch1 = strstr(_szBuffer, "\\status\\");
  sPch2 = strstr(_szBuffer, "\\info\\");
  sPch3 = strstr(_szBuffer, "\\basic\\");
  sPch4 = strstr(_szBuffer, "\\players\\");

  sPch5 = strstr(_szBuffer, "\\secure\\"); // [SSE] [ZCaliptium] Validation Fix.
  
  //CPrintF("Data[%d]: %s\n", iLength, _szBuffer);

  // status request
  if (sPch1) {
    CTString strPacket;
    CTString strLocation;
    strLocation = _pShell->GetString("net_strLocalHost");

    if (strLocation == ""){
      strLocation = "Heartland";
    }

    strPacket.PrintF( PCKQUERY,
      _pShell->GetString("sam_strGameName"),
      _SE_VER_STRING,
      //_pShell->GetString("net_strLocalHost"),
      strLocation,
      _pShell->GetString("gam_strSessionName"),
      _pShell->GetINDEX("net_iPort"),
      _pNetwork->ga_World.wo_strName,
      _getGameModeName(_getSP()->sp_gmGameMode),
      _pNetwork->ga_srvServer.GetPlayersCount(),
      _pNetwork->ga_sesSessionState.ses_ctMaxPlayers,
      _pShell->GetINDEX("gam_bFriendlyFire"),
      _pShell->GetINDEX("gam_bWeaponsStay"),
      _pShell->GetINDEX("gam_bAmmoStays"),
      _pShell->GetINDEX("gam_bHealthArmorStays"),
      _pShell->GetINDEX("gam_bAllowHealth"),
      _pShell->GetINDEX("gam_bAllowArmor"),
      _pShell->GetINDEX("gam_bInfiniteAmmo"),
      _pShell->GetINDEX("gam_bRespawnInPlace"));

      for (INDEX i=0; i<_pNetwork->ga_srvServer.GetPlayersCount(); i++)
      {
        CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[i];
        CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[i];
        if (plt.plt_bActive) {
          CTString strPlayer;
          plt.plt_penPlayerEntity->GetMSLegacyPlayerInf(plb.plb_Index, strPlayer);

          // if we don't have enough space left for the next player
          if (strlen(strPacket) + strlen(strPlayer) > 2048) {
            // send the packet
            _sendPacketTo(strPacket, &_sinFrom);
            strPacket = "";
          }
          strPacket += strPlayer;
        }
      }

    strPacket += "\\final\\\\queryid\\333.1";
    _sendPacketTo(strPacket, &_sinFrom);

  // info request
  } else if (sPch2){

    CTString strPacket;
    strPacket.PrintF( PCKINFO,
      _pShell->GetString("gam_strSessionName"),
      _pShell->GetINDEX("net_iPort"),
      _pNetwork->ga_World.wo_strName,
      _getGameModeName(_getSP()->sp_gmGameMode),
      _pNetwork->ga_srvServer.GetPlayersCount(),
      _pNetwork->ga_sesSessionState.ses_ctMaxPlayers);
    _sendPacketTo(strPacket, &_sinFrom);

  // basis request
  } else if (sPch3){

    CTString strPacket;
    CTString strLocation;
    strLocation = _pShell->GetString("net_strLocalHost");
    if (strLocation == ""){
      strLocation = "Heartland";
    }
    strPacket.PrintF( PCKBASIC,
      _pShell->GetString("sam_strGameName"),
      _SE_VER_STRING,
      //_pShell->GetString("net_strLocalHost"));
      strLocation);
    _sendPacketTo(strPacket, &_sinFrom);

  // players request
  } else if (sPch4){

    // send the player status response
    CTString strPacket;
    strPacket = "";

    for (INDEX i=0; i<_pNetwork->ga_srvServer.GetPlayersCount(); i++) {
      CPlayerBuffer &plb = _pNetwork->ga_srvServer.srv_aplbPlayers[i];
      CPlayerTarget &plt = _pNetwork->ga_sesSessionState.ses_apltPlayers[i];
      if (plt.plt_bActive) {
        CTString strPlayer;
        plt.plt_penPlayerEntity->GetMSLegacyPlayerInf(plb.plb_Index, strPlayer);

        // if we don't have enough space left for the next player
        if (strlen(strPacket) + strlen(strPlayer) > 2048) {
          // send the packet
          _sendPacketTo(strPacket, &_sinFrom);
          strPacket = "";
        }

        strPacket += strPlayer;
      }
    }

    strPacket += "\\final\\\\queryid\\6.1";
    _sendPacketTo(strPacket, &_sinFrom);
  
  // [SSE] [ZCaliptium] '/validate/' - Validation request.
  } else if (sPch5) {
    //CPrintF("Received 'validate' request from MS.\n");
    data += 8;
    
    //CPrintF("SecureKey: %s\n", data);
    
    u_char  ucGamekey[]          = {SERIOUSSAMKEY};
    //u_char  ucReckey[]          = {"XUCXHC"};
    //CPrintF("SecureKey: %s\n", ucReckey);
    unsigned char *pValidateKey = NULL;
    pValidateKey = gsseckey((u_char*)data, ucGamekey, 0);
    
    CTString strPacket;
    strPacket.PrintF("\\validate\\%s\\final\\%s\\queryid\\2.1", pValidateKey, "");
    _sendPacketTo(strPacket, &_sinFrom);

  } else {
    CPrintF("Unknown query server command!\n");
    CPrintF("%s\n", _szBuffer);
    return;
  }
}

static void MSLegacy_ParseStatusResponse(sockaddr_in &_sinClient, BOOL bIgnorePing)
{
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

  CHAR* pszPacket = _szBuffer + 1; // we do +1 because the first character is always '\', which we don't care about.

  BOOL bReadValue = FALSE;
  CTString strKey;
  CTString strValue;

  while(*pszPacket != 0)
  {
    switch (*pszPacket)
    {
      case '\\': {
        if (strKey != "gamemode") {
          if (bReadValue) {
            // we're done reading the value, check which key it was
            if (strKey == "gamename") {
                strGameName = strValue;
            } else if (strKey == "gamever") {
                strVersion = strValue;
            } else if (strKey == "location") {
                strServerLocation = strValue;
            } else if (strKey == "hostname") {
                strSessionName = strValue;
            } else if (strKey == "hostport") {
                strGamePort = strValue;
            } else if (strKey == "mapname") {
                strLevel = strValue;
            } else if (strKey == "gametype") {
                strGameType = strValue;
            } else if (strKey == "activemod") {
                strActiveMod = strValue;
            } else if (strKey == "numplayers") {
                strPlayers = strValue;
            } else if (strKey == "maxplayers") {
                strMaxPlayers = strValue;
            } else {
                //CPrintF("Unknown GameAgent parameter key '%s'!", strKey);
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

  // check if we still have a maxplayers to back up
  if (strKey == "gamemode") {
      strGameMode = strValue;
  }

  if (strActiveMod != "") {
      strGameName = strActiveMod;
  }

  long long tmPing = -1;
  // find the request in the request array
  for (INDEX i=0; i<ga_asrRequests.Count(); i++) {
      CServerRequest &req = ga_asrRequests[i];
      if (req.sr_ulAddress == _sinClient.sin_addr.s_addr && req.sr_iPort == _sinClient.sin_port) {
          tmPing = _pTimer->GetHighPrecisionTimer().GetMilliseconds() - req.sr_tmRequestTime;
          ga_asrRequests.Delete(&req);
          break;
      }
  }
  
  if (bIgnorePing) {
    tmPing = 0;
  }

  if (bIgnorePing || (tmPing > 0 && tmPing < 2500000))
  {
    // insert the server into the serverlist
    CNetworkSession &ns = *new CNetworkSession;
    _pNetwork->ga_lhEnumeratedSessions.AddTail(ns.ns_lnNode);

    // add the server to the serverlist
    ns.ns_strSession = strSessionName;
    ns.ns_strAddress = inet_ntoa(_sinClient.sin_addr) + CTString(":") + CTString(0, "%d", htons(_sinClient.sin_port) - 1);
    ns.ns_tmPing = (tmPing / 1000.0f);
    ns.ns_strWorld = strLevel;
    ns.ns_ctPlayers = atoi(strPlayers);
    ns.ns_ctMaxPlayers = atoi(strMaxPlayers);
    ns.ns_strGameType = strGameType;
    ns.ns_strMod = strGameName;
    ns.ns_strVer = strVersion;
  }
}

void *_MS_Thread(void *lpParam)
{
  int _sockudp = NULL;
  struct _sIPPort {
    UBYTE bFirst;
    UBYTE bSecond;
    UBYTE bThird;
    UBYTE bFourth;
    USHORT iPort;
  };

  _setStatus("");
  _sockudp = socket(AF_INET, SOCK_DGRAM, 0);

  if (_sockudp == INVALID_SOCKET){
	ThrowF_t(TRANS("Cannot open socket. %s (%i)"), std::strerror(errno), errno);
  }

  _sIPPort* pServerIP = (_sIPPort*)(_szIPPortBuffer);
  while(_iIPPortBufferLen >= 6)
  {
    if (!strncmp((char *)pServerIP, "\\final\\", 7)) {
      break;
    }

    _sIPPort ip = *pServerIP;

    CTString strIP;
    strIP.PrintF("%d.%d.%d.%d", ip.bFirst, ip.bSecond, ip.bThird, ip.bFourth);

    sockaddr_in sinServer;
    sinServer.sin_family = AF_INET;
    sinServer.sin_addr.s_addr = inet_addr(strIP);
    sinServer.sin_port = ip.iPort;

    // insert server status request into container
    CServerRequest &sreq = ga_asrRequests.Push();
    sreq.sr_ulAddress = sinServer.sin_addr.s_addr;
    sreq.sr_iPort = sinServer.sin_port;
    sreq.sr_tmRequestTime = _pTimer->GetHighPrecisionTimer().GetMilliseconds();

    // send packet to server
    sendto(_sockudp,"\\status\\",8,0,
        (const struct sockaddr *) &sinServer, sizeof(sinServer));

    sockaddr_in _sinClient;
    int _iClientLength = sizeof(_sinClient);

    fd_set readfds_udp;                         // declare a read set
    struct timeval timeout_udp;                 // declare a timeval for our timer
    int iRet = -1;

    FD_ZERO(&readfds_udp);                      // zero out the read set
    FD_SET(_sockudp, &readfds_udp);                // add socket to the read set
    timeout_udp.tv_sec = 0;                     // timeout = 0 seconds
    timeout_udp.tv_usec = 50000;               // timeout += 0.05 seconds
    int _iN = select(_sockudp + 1, &readfds_udp, NULL, NULL, &timeout_udp);

    if (_iN > 0)
    {
      /** do recvfrom stuff **/
      iRet =  recvfrom(_sockudp, _szBuffer, 2048, 0, (struct sockaddr*)&_sinClient, &_iClientLength);
      FD_CLR(_sockudp, &readfds_udp);

      if (iRet != -1 && iRet > 100 && iRet != SOCKET_ERROR) {
        // null terminate the buffer
        _szBuffer[iRet] = 0;
        char *sPch = NULL;
        sPch = strstr(_szBuffer, "\\gamename\\serioussamse\\");

        if (!sPch) {
            CPrintF("Unknown query server response!\n");
        } else {
          MSLegacy_ParseStatusResponse(_sinClient, FALSE);
        }

      } else {
        // find the request in the request array
        for (INDEX i = 0; i < ga_asrRequests.Count(); i++)
        {
          CServerRequest &req = ga_asrRequests[i];
          if (req.sr_ulAddress == _sinClient.sin_addr.s_addr && req.sr_iPort == _sinClient.sin_port) {
            ga_asrRequests.Delete(&req);
            break;
          }
        }
      }
    }

    pServerIP++;
    _iIPPortBufferLen -= 6;
  }

  if (_szIPPortBuffer) free (_szIPPortBuffer);
  _szIPPortBuffer = NULL;

  close(_sockudp);
  _uninitWinsock();
  _bInitialized = FALSE;
  _pNetwork->ga_bEnumerationChange = FALSE;
  //WSACleanup();

  return 0;
}

void *_LocalNet_Thread(void *lpParam)
{
  int _sockudp = NULL;
  struct _sIPPort {
    UBYTE bFirst;
    UBYTE bSecond;
    UBYTE bThird;
    UBYTE bFourth;
    USHORT iPort;
  };

  _sockudp = socket(AF_INET, SOCK_DGRAM, 0);

  if (_sockudp == INVALID_SOCKET)
  {
    //WSACleanup();
    _pNetwork->ga_strEnumerationStatus = "";

    if (_szIPPortBufferLocal != NULL) {
      delete[] _szIPPortBufferLocal;
    }

    _szIPPortBufferLocal = NULL;		
  }

  _sIPPort* pServerIP = (_sIPPort*)(_szIPPortBufferLocal);
  
  int optval = 1;
  if (setsockopt(_sockudp, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0)
  {
    ThrowF_t(TRANS("Cannot set socket broadcast. %s (%i)"), std::strerror(errno), errno);
  }

  struct   sockaddr_in saddr;
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = 0xFFFFFFFF;
  
  unsigned short startport = 25601;
  unsigned short endport =  startport + 20;
  
	for (int i = startport ; i <= endport ; i += 1)
	{
    saddr.sin_port = htons(i);
    sendto(_sockudp, "\\status\\", 8, 0, (const struct sockaddr *) &saddr, sizeof(saddr));
  }

  //while(_iIPPortBufferLocalLen >= 6)
  {
    /*if (!strncmp((char *)pServerIP, "\\final\\", 7)) {
      break;
    }*/

    _sIPPort ip = *pServerIP;

    CTString strIP;
    strIP.PrintF("%d.%d.%d.%d", ip.bFourth, ip.bThird, ip.bSecond, ip.bFirst);

    /*
    sockaddr_in sinServer;
    sinServer.sin_family = AF_INET;
    sinServer.sin_addr.s_addr = inet_addr(strIP);
    sinServer.sin_port = ip.iPort;
    */

    // insert server status request into container
    /*
    CServerRequest &sreq = ga_asrRequests.Push();
    sreq.sr_ulAddress = sinServer.sin_addr.s_addr;
    sreq.sr_iPort = sinServer.sin_port;
    sreq.sr_tmRequestTime = _pTimer->GetHighPrecisionTimer().GetMilliseconds();*/

    // send packet to server
    //sendto(_sockudp,"\\status\\",8,0, (sockaddr *) &sinServer, sizeof(sinServer));

    sockaddr_in _sinClient;
    int _iClientLength = sizeof(_sinClient);

    fd_set readfds_udp;                         // declare a read set
    struct timeval timeout_udp;                 // declare a timeval for our timer
    int iRet = -1;

    FD_ZERO(&readfds_udp);                      // zero out the read set
    FD_SET(_sockudp, &readfds_udp);             // add socket to the read set
    timeout_udp.tv_sec = 0;                     // timeout = 0 seconds
    timeout_udp.tv_usec = 250000; // 0.25 sec //50000;                // timeout += 0.05 seconds

    int _iN = select(_sockudp + 1, &readfds_udp, NULL, NULL, &timeout_udp);
    
    CPrintF("Received %d answers.\n", _iN);

    if (_iN > 0)
    {
      /** do recvfrom stuff **/
      iRet =  recvfrom(_sockudp, _szBuffer, 2048, 0, (struct sockaddr*)&_sinClient, &_iClientLength);
      FD_CLR(_sockudp, &readfds_udp);

      if (iRet != -1 && iRet > 100 && iRet != SOCKET_ERROR)
      {
        // null terminate the buffer
        _szBuffer[iRet] = 0;
        char *sPch = NULL;
        sPch = strstr(_szBuffer, "\\gamename\\serioussamse\\");

        if (!sPch) {
          CPrintF("Unknown query server response!\n");

          if (_szIPPortBufferLocal != NULL) {
            delete[] _szIPPortBufferLocal;
          }
          _szIPPortBufferLocal = NULL;               
         // WSACleanup();
        } else {
          MSLegacy_ParseStatusResponse(_sinClient, TRUE);
        }

      } else {
        // find the request in the request array
        for (INDEX i=0; i<ga_asrRequests.Count(); i++)
        {
          CServerRequest &req = ga_asrRequests[i];
          if (req.sr_ulAddress == _sinClient.sin_addr.s_addr && req.sr_iPort == _sinClient.sin_port) {
            ga_asrRequests.Delete(&req);
            break;
          }
        }
      }
    }

   // pServerIP++;
    //_iIPPortBufferLocalLen -= 6;
  }

  if (_szIPPortBufferLocal != NULL) {
    delete[] _szIPPortBufferLocal;
  }

  _szIPPortBufferLocal = NULL;

  close(_sockudp);
  _uninitWinsock();
  _bInitialized = FALSE;
  _pNetwork->ga_bEnumerationChange = FALSE;
  _pNetwork->ga_strEnumerationStatus = "";
 // WSACleanup();

  return 0;
}
