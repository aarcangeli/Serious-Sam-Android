#include <Engine/StdH.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/ErrorReporting.h>
#include <Engine/Base/ErrorTable.h>
#include <Engine/Base/ProgressHook.h>
#include <Engine/Base/Synchronization.h>
#include <Engine/Base/Translation.h>

#include <Engine/Network/ClientInterface.h>
#include <Engine/Network/CommunicationInterface.h>
#include <Engine/Network/CPacket.h>
#include <Engine/Network/Network.h>
#include <Engine/Network/Server.h>

#include <Engine/GameAgent/GameAgent.h>

#include <arpa/inet.h>
#include <netdb.h>

#include <cerrno>
#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <config.h>

#define HOSTENT hostent
#define INVALID_SOCKET  (-1)
#define SOCKET_ERROR            (-1)
#define SLASHSLASH  0x2F2F   // looks like "//" in ASCII.

// modified version of Engine/Network/CommunicationInterface.cpp with network disabled

#define SERVER_LOCAL_CLIENT     0
extern INDEX net_iPort;
extern CTString net_strLocalHost;
extern INDEX net_bLookupHostNames;
extern INDEX net_bReportICMPErrors;
extern FLOAT net_fDropPackets;
extern FLOAT net_tmConnectionTimeout;
extern INDEX net_bReportPackets;

//structures used to emulate bandwidth and latency parameters - shared by all client interfaces
CPacketBufferStats _pbsSend;
CPacketBufferStats _pbsRecv;

ULONG cm_ulLocalHost;      // configured local host address
CTString cm_strAddress;   // local address
CTString cm_strName;      // local address

CTCriticalSection cm_csComm;  // critical section for access to communication data

BOOL cm_bNetworkInitialized;

// index 0 is the server's local client, this is an array used by server only
CClientInterface cm_aciClients[SERVER_CLIENTS];

// Broadcast interface - i.e. interface for 'nonconnected' communication
CClientInterface cm_ciBroadcast;

// this is used by client only
CClientInterface cm_ciLocalClient;

// global communication interface object (there is only one for the entire engine)
CCommunicationInterface _cmiComm;

// convert address to a printable string
CTString AddressToString(ULONG ulHost) {
  ULONG ulHostNet = htonl(ulHost);

  // initially not converted
  struct hostent *hostentry = NULL;

  // if DNS lookup is allowed
  if (net_bLookupHostNames) {
    // lookup the host
    hostentry = gethostbyaddr((char *) &ulHostNet, sizeof(ulHostNet), AF_INET);
  }

  // if DNS lookup succeeded
  if (hostentry != NULL) {
    // return its ascii name
    return (char *) hostentry->h_name;
    // if DNS lookup failed
  } else {
    // just convert to dotted number format
    return inet_ntoa((const in_addr &) ulHostNet);
  }
}

/*
*
*	Two helper functions - conversion from IP to words
*
*/

// convert string address to a number
ULONG StringToAddress(const CTString &strAddress) {
  // first try to convert numeric address
  ULONG ulAddress = ntohl(inet_addr(strAddress));
  // if not a valid numeric address
  if (ulAddress == INADDR_NONE) {
    // lookup the host
    HOSTENT *phe = gethostbyname(strAddress);
    // if succeeded
    if (phe != NULL) {
      // get that address
      ulAddress = ntohl(*(ULONG *) phe->h_addr_list[0]);
    }
  }

  // return what we got
  return ulAddress;
};


CCommunicationInterface::CCommunicationInterface(void) {
  cm_csComm.cs_iIndex = -1;
  CTSingleLock slComm(&cm_csComm, TRUE);

  cci_bInitialized = FALSE;
  cci_bWinSockOpen = FALSE;

  cci_bServerInitialized = FALSE;
  cci_bClientInitialized = FALSE;
  cm_ciLocalClient.ci_bClientLocal = FALSE;

  cci_hSocket = INVALID_SOCKET;

};

// initialize
void CCommunicationInterface::Init() {
  CTSingleLock slComm(&cm_csComm, TRUE);

  cci_bWinSockOpen = FALSE;
  cci_bInitialized = TRUE;

  // mark as initialized
  cm_bNetworkInitialized = FALSE;

  cci_pbMasterInput.Clear();
  cci_pbMasterOutput.Clear();
}

// close
void CCommunicationInterface::Close(void) {
  CTSingleLock slComm(&cm_csComm, TRUE);

  ASSERT(cci_bInitialized);
  ASSERT(!cci_bServerInitialized);
  ASSERT(!cci_bClientInitialized);

  // mark as closed
  cm_bNetworkInitialized = FALSE;
  cci_bInitialized = FALSE;
  cm_ciLocalClient.ci_bClientLocal = FALSE;

  cci_pbMasterInput.Clear();
  cci_pbMasterOutput.Clear();

};

// prepares the comm interface for use - MUST be invoked before any data can be sent/received
void CCommunicationInterface::PrepareForUse(BOOL bUseNetwork, BOOL bClient) {

  // clear the network conditions emulation data
  _pbsSend.Clear();
  _pbsRecv.Clear();

  // if the network is already initialized, shut it down before proceeding
  if (cm_bNetworkInitialized) {
    Unprepare();
  }

  if (bUseNetwork) {
    CPrintF(TRANS("Initializing TCP/IP...\n"));
    if (bClient) {
      CPrintF(TRANS("  opening as client\n"));
    } else {
      CPrintF(TRANS("  opening as server\n"));
    }

#ifdef  USE_TCP
    cci_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    cci_hSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#endif
    if (cci_hSocket == INVALID_SOCKET) {
      ThrowF_t(TRANS("Cannot open socket. %s (%i)"), std::strerror(errno), errno);
    }

    // set non blocking
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10;
    setsockopt(cci_hSocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);

    int flagTrue = 1;
    setsockopt(cci_hSocket, SOL_SOCKET, SO_REUSEADDR, &flagTrue, sizeof(flagTrue));

    if (!bClient) {

      sockaddr_in servaddr{};
      servaddr.sin_family = AF_INET;
      servaddr.sin_addr.s_addr = INADDR_ANY;
      servaddr.sin_port = htons(net_iPort);

      int res = bind(cci_hSocket, (const struct sockaddr *)&servaddr,sizeof(servaddr));
      if (res < 0) {
        ThrowF_t(TRANS("Cannot bind port %li. %s (%i)"), net_iPort, std::strerror(errno), errno);
      }

    }

    cci_bBound = TRUE;
    cci_bSocketOpen = TRUE;
    cci_bFirstByteReceived = false;
    cm_bNetworkInitialized = true;
    cm_ciBroadcast.SetLocal(nullptr);
    CPrintF(TRANS("  opened socket: \n"));
  }

};


// shut down the communication interface
void CCommunicationInterface::Unprepare(void) {
  if (cci_bWinSockOpen) {
    close(cci_hSocket);
    cm_ciBroadcast.Clear();
    cci_bBound = FALSE;
    cci_bWinSockOpen = false;
    cci_bFirstByteReceived = false;
  }
};


BOOL CCommunicationInterface::IsNetworkEnabled(void) {
  return cm_bNetworkInitialized;
};

// get address of local machine
void CCommunicationInterface::GetHostName(CTString &strName, CTString &strAddress) {
  strName = cm_strName;
  strAddress = cm_strAddress;
};



/*
*
*
*	Socket functions - creating, binding...
*
*
*/


// get generic socket error info string about last error
CTString CCommunicationInterface::GetSocketError(INDEX iError) {
  CTString strError;
  strError.PrintF(TRANS("Socket %d, Error %d"), cci_hSocket, iError);
  return strError;
};

// get address of this host
void CCommunicationInterface::GetLocalAddress_t(ULONG &ulHost, ULONG &ulPort) {
  FatalError("CCommunicationInterface::GetLocalAddress_t should not be called");
}

// get address of the peer host connected to this socket
void CCommunicationInterface::GetRemoteAddress_t(ULONG &ulHost, ULONG &ulPort) {
  FatalError("CCommunicationInterface::GetRemoteAddress_t should not be called");
}


/*
 *  ---->>>>  BROADCAST INTERFACE   <<<<----
 */

// broadcast communication
void CCommunicationInterface::Broadcast_Send(const void *pvSend, SLONG slSendSize,
                                             CAddress &adrDestination) {
  CTSingleLock slComm(&cm_csComm, TRUE);

  cm_ciBroadcast.ci_adrAddress.adr_ulAddress = adrDestination.adr_ulAddress;
  cm_ciBroadcast.ci_adrAddress.adr_uwPort = adrDestination.adr_uwPort;
  cm_ciBroadcast.ci_adrAddress.adr_uwID = adrDestination.adr_uwID;

  cm_ciBroadcast.Send(pvSend, slSendSize, FALSE);
}

BOOL CCommunicationInterface::Broadcast_Receive(void *pvReceive, SLONG &slReceiveSize,
                                                CAddress &adrAddress) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  return cm_ciBroadcast.ReceiveFrom(pvReceive, slReceiveSize, &adrAddress, FALSE);
}


// update the broadcast input buffer - handle any incoming connection requests
void CCommunicationInterface::Broadcast_Update_t() {
  CPacket *ppaConnectionRequest;
  BOOL bIsAlready;
  BOOL bFoundEmpty;
  ULONG iClient;
  UBYTE ubDummy = 65;

  // while there is a connection request packet in the input buffer
  while ((ppaConnectionRequest = cm_ciBroadcast.ci_pbReliableInputBuffer.GetConnectRequestPacket()) != NULL) {
    // see if there is a client already connected at that address and port
    bIsAlready = FALSE;
    for (iClient = 1; iClient < SERVER_CLIENTS; iClient++) {
      if (cm_aciClients[iClient].ci_adrAddress.adr_ulAddress ==
          ppaConnectionRequest->pa_adrAddress.adr_ulAddress &&
          cm_aciClients[iClient].ci_adrAddress.adr_uwPort ==
          ppaConnectionRequest->pa_adrAddress.adr_uwPort) {
        bIsAlready = TRUE;
        break;
      }
    }
    // if the client is already connected then just ignore the packet - else, connect it
    if (!bIsAlready) {
      // find an empty client structure
      bFoundEmpty = FALSE;
      for (iClient = 1; iClient < SERVER_CLIENTS; iClient++) {
        if (cm_aciClients[iClient].ci_bUsed == FALSE) {
          bFoundEmpty = TRUE;
          // we have an empty slot, so fill it for the client
          cm_aciClients[iClient].ci_adrAddress.adr_ulAddress = ppaConnectionRequest->pa_adrAddress.adr_ulAddress;
          cm_aciClients[iClient].ci_adrAddress.adr_uwPort = ppaConnectionRequest->pa_adrAddress.adr_uwPort;
          // generate the ID
          UWORD uwID = _pTimer->GetHighPrecisionTimer().tv_llValue & 0x0FFF;
          if (uwID == 0 || uwID == SLASHSLASH) {
            uwID += 1;
          }
          cm_aciClients[iClient].ci_adrAddress.adr_uwID = (uwID << 4) + iClient;
          // form the connection response packet
          ppaConnectionRequest->pa_adrAddress.adr_uwID = SLASHSLASH;
          ppaConnectionRequest->pa_ubReliable =
            UDP_PACKET_RELIABLE | UDP_PACKET_RELIABLE_HEAD | UDP_PACKET_RELIABLE_TAIL |
            UDP_PACKET_CONNECT_RESPONSE;
          // return it to the client
          ppaConnectionRequest->WriteToPacket(&(cm_aciClients[iClient].ci_adrAddress.adr_uwID),
                                              sizeof(cm_aciClients[iClient].ci_adrAddress.adr_uwID),
                                              ppaConnectionRequest->pa_ubReliable,
                                              cm_ciBroadcast.ci_ulSequence++,
                                              ppaConnectionRequest->pa_adrAddress.adr_uwID,
                                              sizeof(cm_aciClients[iClient].ci_adrAddress.adr_uwID));
          cm_ciBroadcast.ci_pbOutputBuffer.AppendPacket(*ppaConnectionRequest, TRUE);
          cm_aciClients[iClient].ci_bUsed = TRUE;
          return;
        }
      }

      // if none found
      if (!bFoundEmpty) {
        // error
        ThrowF_t(TRANS("Server: Cannot accept new clients, all slots used!\n"));
      }
    }
  }

};


/*
 *  ---->>>>  SERVER  <<<<----
 */

/*
 *  Initialize server
 */
void CCommunicationInterface::Server_Init_t(void) {
  CTSingleLock slComm(&cm_csComm, TRUE);

  ASSERT(cci_bInitialized);
  ASSERT(!cci_bServerInitialized);

  // for each client
  for (INDEX iClient = 0; iClient < SERVER_CLIENTS; iClient++) {
    // clear its status
    cm_aciClients[iClient].Clear();
    cm_aciClients[iClient].ci_pbOutputBuffer.pb_ppbsStats = &_pbsSend;
    cm_aciClients[iClient].ci_pbInputBuffer.pb_ppbsStats = &_pbsRecv;
  }

  // mark the server's instance of the local client as such
  cm_aciClients[SERVER_LOCAL_CLIENT].ci_bClientLocal = TRUE;
  cm_aciClients[SERVER_LOCAL_CLIENT].ci_bUsed = TRUE;

  // prepare the client part of the local client
  cm_ciLocalClient.Clear();
  cm_ciLocalClient.ci_bUsed = TRUE;
  cm_ciLocalClient.ci_bClientLocal = TRUE;
  cm_ciLocalClient.ci_pbOutputBuffer.pb_ppbsStats = &_pbsSend;
  cm_ciLocalClient.ci_pbInputBuffer.pb_ppbsStats = &_pbsRecv;

  // mark that the server was initialized
  cci_bServerInitialized = TRUE;
};

/*
 *  Close server
 */
void CCommunicationInterface::Server_Close(void) {
  CTSingleLock slComm(&cm_csComm, TRUE);

  // close all clients
  for (INDEX iClient = 0; iClient < SERVER_CLIENTS; iClient++) {
    cm_aciClients[iClient].Clear();
  }

  // mark that the server is uninitialized
  cci_bServerInitialized = FALSE;
};


/*
 *  Server clear client and prepare for new connection
 */
void CCommunicationInterface::Server_ClearClient(INDEX iClient) {
  // synchronize access to communication data
  CTSingleLock slComm(&cm_csComm, TRUE);

  ASSERT(iClient >= 0 && iClient < SERVER_CLIENTS);
  cm_aciClients[iClient].Clear();
};

BOOL CCommunicationInterface::Server_IsClientLocal(INDEX iClient) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  ASSERT(iClient >= 0 && iClient < SERVER_CLIENTS);
  return iClient == SERVER_LOCAL_CLIENT;
};

BOOL CCommunicationInterface::Server_IsClientUsed(INDEX iClient) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  ASSERT(iClient >= 0 && iClient < SERVER_CLIENTS);
  return cm_aciClients[iClient].ci_bUsed;
};

CTString CCommunicationInterface::Server_GetClientName(INDEX iClient) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  ASSERT(iClient >= 0 && iClient < SERVER_CLIENTS);

  if (iClient == SERVER_LOCAL_CLIENT) {
    return TRANS("Local machine");
  }

  cm_aciClients[iClient].ci_strAddress = AddressToString(
    cm_aciClients[iClient].ci_adrAddress.adr_ulAddress);

  return cm_aciClients[iClient].ci_strAddress;
};

/*
 *  Server Send/Receive Reliable
 */
void
CCommunicationInterface::Server_Send_Reliable(INDEX iClient, const void *pvSend, SLONG slSendSize) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  ASSERT(iClient >= 0 && iClient < SERVER_CLIENTS);
  cm_aciClients[iClient].Send(pvSend, slSendSize, TRUE);
};

BOOL CCommunicationInterface::Server_Receive_Reliable(INDEX iClient, void *pvReceive,
                                                      SLONG &slReceiveSize) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  ASSERT(iClient >= 0 && iClient < SERVER_CLIENTS);
  return cm_aciClients[iClient].Receive(pvReceive, slReceiveSize, TRUE);
};

/*
 *  Server Send/Receive Unreliable
 */
void CCommunicationInterface::Server_Send_Unreliable(INDEX iClient, const void *pvSend,
                                                     SLONG slSendSize) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  ASSERT(iClient >= 0 && iClient < SERVER_CLIENTS);
  cm_aciClients[iClient].Send(pvSend, slSendSize, FALSE);
};

BOOL CCommunicationInterface::Server_Receive_Unreliable(INDEX iClient, void *pvReceive,
                                                        SLONG &slReceiveSize) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  ASSERT(iClient >= 0 && iClient < SERVER_CLIENTS);
  return cm_aciClients[iClient].Receive(pvReceive, slReceiveSize, FALSE);
};


BOOL CCommunicationInterface::Server_Update() {

  CTSingleLock slComm(&cm_csComm, TRUE);
  CPacket *ppaPacket;
  CPacket *ppaPacketCopy;
  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();
  INDEX iClient;

  // transfer packets for the local client
  if (cm_ciLocalClient.ci_bUsed && cm_ciLocalClient.ci_pciOther != NULL) {
    cm_ciLocalClient.ExchangeBuffers();
  };

  cm_aciClients[0].UpdateOutputBuffers();

  // if not just playing single player
  if (cci_bServerInitialized) {
    Broadcast_Update_t();
    // for each client transfer packets from the output buffer to the master output buffer
    for (iClient = 1; iClient < SERVER_CLIENTS; iClient++) {
      CClientInterface &ci = cm_aciClients[iClient];
      // if not connected
      if (!ci.ci_bUsed) {
        // skip it
        continue;
      }
      // update its buffers, if a reliable packet is overdue (has not been delivered too long)
      // disconnect the client
      if (ci.UpdateOutputBuffers() != FALSE) {
        // transfer packets ready to be sent out to the master output buffer
        while (ci.ci_pbOutputBuffer.pb_ulNumOfPackets > 0) {
          ppaPacket = ci.ci_pbOutputBuffer.PeekFirstPacket();
          if (ppaPacket->pa_tvSendWhen < tvNow) {
            ci.ci_pbOutputBuffer.RemoveFirstPacket(FALSE);
            if (ppaPacket->pa_ubReliable & UDP_PACKET_RELIABLE) {
              ppaPacketCopy = new CPacket;
              *ppaPacketCopy = *ppaPacket;
              ci.ci_pbWaitAckBuffer.AppendPacket(*ppaPacketCopy, FALSE);
            }
            cci_pbMasterOutput.AppendPacket(*ppaPacket, FALSE);
          } else {
            break;
          }
        }
      } else {
        CPrintF(TRANS("Unable to deliver data to client '%s', disconnecting.\n"),
                AddressToString(cm_aciClients[iClient].ci_adrAddress.adr_ulAddress));
        Server_ClearClient(iClient);
        _pNetwork->ga_srvServer.HandleClientDisconected(iClient);

      }
    }

    // update broadcast output buffers
    // update its buffers
    cm_ciBroadcast.UpdateOutputBuffers();
    // transfer packets ready to be sent out to the master output buffer
    while (cm_ciBroadcast.ci_pbOutputBuffer.pb_ulNumOfPackets > 0) {
      ppaPacket = cm_ciBroadcast.ci_pbOutputBuffer.PeekFirstPacket();
      if (ppaPacket->pa_tvSendWhen < tvNow) {
        cm_ciBroadcast.ci_pbOutputBuffer.RemoveFirstPacket(FALSE);
        cci_pbMasterOutput.AppendPacket(*ppaPacket, FALSE);
      } else {
        break;
      }
    }

    // send/receive packets over the TCP/IP stack
    UpdateMasterBuffers();

    // dispatch all packets from the master input buffer to the clients' input buffers
    while (cci_pbMasterInput.pb_ulNumOfPackets > 0) {
      BOOL bClientFound;
      ppaPacket = cci_pbMasterInput.GetFirstPacket();
      bClientFound = FALSE;
      if (ppaPacket->pa_adrAddress.adr_uwID == SLASHSLASH || ppaPacket->pa_adrAddress.adr_uwID == 0) {
        cm_ciBroadcast.ci_pbInputBuffer.AppendPacket(*ppaPacket, FALSE);
        bClientFound = TRUE;
      } else {
        for (iClient = 0; iClient < SERVER_CLIENTS; iClient++) {
          if (ppaPacket->pa_adrAddress.adr_uwID == cm_aciClients[iClient].ci_adrAddress.adr_uwID) {
            cm_aciClients[iClient].ci_pbInputBuffer.AppendPacket(*ppaPacket, FALSE);
            bClientFound = TRUE;
            break;
          }
        }
      }
      if (!bClientFound) {
        // warn about possible attack
        extern INDEX net_bReportMiscErrors;
        if (net_bReportMiscErrors) {
          CPrintF(TRANS("WARNING: Invalid message from: %s\n"),
                  AddressToString(ppaPacket->pa_adrAddress.adr_ulAddress));
        }
      }
    }

    for (iClient = 1; iClient < SERVER_CLIENTS; iClient++) {
      cm_aciClients[iClient].UpdateInputBuffers();
    }


  }
  cm_aciClients[0].UpdateInputBuffers();
  cm_ciLocalClient.UpdateInputBuffers();
  cm_ciBroadcast.UpdateInputBuffersBroadcast();
  Broadcast_Update_t();

  return TRUE;
};


/*
 *  ---->>>>  CLIENT  <<<<----
 */

/*
 *  Initialize client
 */
void CCommunicationInterface::Client_Init_t(char *strServerName) {
  CTSingleLock slComm(&cm_csComm, TRUE);

  ASSERT(cci_bInitialized);
  ASSERT(!cci_bClientInitialized);

  // retrieve server address from server name
  ULONG ulServerAddress = StringToAddress(strServerName);
  // if lookup failed
  if (ulServerAddress == INADDR_NONE) {
    ThrowF_t(TRANS("Host '%s' not found!\n"), strServerName);
  }

  // call client init with server address
  Client_Init_t(ulServerAddress);
};

void CCommunicationInterface::Client_Init_t(ULONG ulServerAddress) {
  CTSingleLock slComm(&cm_csComm, TRUE);

  ASSERT(cci_bInitialized);
  ASSERT(!cci_bClientInitialized);

  cm_ciLocalClient.Clear();
  cm_ciLocalClient.ci_pbOutputBuffer.pb_ppbsStats = &_pbsSend;
  cm_ciLocalClient.ci_pbInputBuffer.pb_ppbsStats = &_pbsRecv;

  // if this computer is not the server
  if (!cci_bServerInitialized) {
    // open with connecting to remote server
    cm_ciLocalClient.ci_bClientLocal = FALSE;
    Client_OpenNet_t(ulServerAddress);

    // if this computer is server
  } else {
    // open local client
    cm_ciLocalClient.ci_bClientLocal = TRUE;
    Client_OpenLocal();
  }

  cci_bClientInitialized = TRUE;
};

/*
 *  Close client
 */
void CCommunicationInterface::Client_Close(void) {
  CTSingleLock slComm(&cm_csComm, TRUE);

  ASSERT(cci_bInitialized);

  // dispatch remaining packets (keep trying for half a second - 10 attempts)
  for (TIME tmWait = 0; tmWait < 500;
       Sleep(NET_WAITMESSAGE_DELAY), tmWait += NET_WAITMESSAGE_DELAY) {
    // if all packets are successfully sent, exit loop
    if ((cm_ciLocalClient.ci_pbOutputBuffer.pb_ulNumOfPackets == 0)
        && (cm_ciLocalClient.ci_pbWaitAckBuffer.pb_ulNumOfPackets == 0)) {
      break;
    }
    if (Client_Update() == FALSE) {
      break;
    }
  }

  cm_ciLocalClient.Clear();

  cm_ciLocalClient.ci_bClientLocal = FALSE;
  cci_bClientInitialized = FALSE;
};


/*
 *  Open client local
 */
void CCommunicationInterface::Client_OpenLocal(void) {
  CTSingleLock slComm(&cm_csComm, TRUE);

  CClientInterface &ci0 = cm_ciLocalClient;
  CClientInterface &ci1 = cm_aciClients[SERVER_LOCAL_CLIENT];

  ci0.ci_bUsed = TRUE;
  ci0.SetLocal(&ci1);
  ci1.ci_bUsed = TRUE;
  ci1.SetLocal(&ci0);
};


/*
 *  Open client remote
 */
void CCommunicationInterface::Client_OpenNet_t(ULONG ulServerAddress) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  CPacket *ppaInfoPacket;
  CPacket *ppaReadPacket;
  UBYTE ubDummy = 65;
  UBYTE ubReliable;

  // check for reconnection
  static ULONG ulLastServerAddress = -1;
  BOOL bReconnecting = ulServerAddress == ulLastServerAddress;
  ulLastServerAddress = ulServerAddress;

  const INDEX iRefresh = 100; // (in miliseconds)
  // determine connection timeout
  INDEX ctRetries = bReconnecting ? (180 * 1000 / iRefresh) : 3;

  // start waiting for server's response
  if (ctRetries > 1) {
    SetProgressDescription(TRANS("waiting for server"));
    CallProgressHook_t(0.0f);
  }



  // form the connection request packet
  ppaInfoPacket = new CPacket;
  ubReliable = UDP_PACKET_RELIABLE | UDP_PACKET_RELIABLE_HEAD | UDP_PACKET_RELIABLE_TAIL |
               UDP_PACKET_CONNECT_REQUEST;
  ppaInfoPacket->pa_adrAddress.adr_ulAddress = ulServerAddress;
  ppaInfoPacket->pa_adrAddress.adr_uwPort = net_iPort;
  ppaInfoPacket->pa_ubRetryNumber = 0;
  ppaInfoPacket->WriteToPacket(&ubDummy, 1, ubReliable, cm_ciLocalClient.ci_ulSequence++, SLASHSLASH, 1);

  cm_ciLocalClient.ci_pbOutputBuffer.AppendPacket(*ppaInfoPacket, TRUE);

  // set client destination address to server address
  cm_ciLocalClient.ci_adrAddress.adr_ulAddress = ulServerAddress;
  cm_ciLocalClient.ci_adrAddress.adr_uwPort = net_iPort;

  // for each retry
  for (INDEX iRetry = 0; iRetry < ctRetries; iRetry++) {
    // send/receive and juggle the buffers
    if (Client_Update() == FALSE) {
      break;
    }

    // if there is something in the input buffer
    if (cm_ciLocalClient.ci_pbReliableInputBuffer.pb_ulNumOfPackets > 0) {
      ppaReadPacket = cm_ciLocalClient.ci_pbReliableInputBuffer.GetFirstPacket();
      CPrintF("Received packet\n");
      // and it is a connection confirmation
      if (ppaReadPacket->pa_ubReliable && UDP_PACKET_CONNECT_RESPONSE) {
        // the client has succedeed to connect, so read the uwID from the packet
        cm_ciLocalClient.ci_adrAddress.adr_ulAddress = ulServerAddress;
        cm_ciLocalClient.ci_adrAddress.adr_uwPort = net_iPort;
        cm_ciLocalClient.ci_adrAddress.adr_uwID = *((UWORD * )(
          ppaReadPacket->pa_pubPacketData + MAX_HEADER_SIZE));
        cm_ciLocalClient.ci_bUsed = TRUE;
        cm_ciLocalClient.ci_bClientLocal = FALSE;
        cm_ciLocalClient.ci_pciOther = NULL;

        cm_ciLocalClient.ci_pbReliableInputBuffer.RemoveConnectResponsePackets();

        delete ppaReadPacket;

        // finish waiting
        CallProgressHook_t(1.0f);
        return;
      }
    }

    Sleep(iRefresh);
    CallProgressHook_t(FLOAT(iRetry % 10) / 10);
  }

  cci_bBound = FALSE;
  ThrowF_t(TRANS("Client: Timeout receiving UDP port"));
};


/*
 *  Clear local client
 */

void CCommunicationInterface::Client_Clear(void) {
  // synchronize access to communication data
  CTSingleLock slComm(&cm_csComm, TRUE);

  cm_ciLocalClient.Clear();
};

/*
 *  Client get status
 */
BOOL CCommunicationInterface::Client_IsConnected(void) {
  // synchronize access to communication data
  CTSingleLock slComm(&cm_csComm, TRUE);

  return cm_ciLocalClient.ci_bUsed;
};

/*
 *  Client Send/Receive Reliable
 */
void CCommunicationInterface::Client_Send_Reliable(const void *pvSend, SLONG slSendSize) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  cm_ciLocalClient.Send(pvSend, slSendSize, TRUE);
};

BOOL CCommunicationInterface::Client_Receive_Reliable(void *pvReceive, SLONG &slReceiveSize) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  return cm_ciLocalClient.Receive(pvReceive, slReceiveSize, TRUE);
};

BOOL CCommunicationInterface::Client_Receive_Reliable(CTStream &strmReceive) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  return cm_ciLocalClient.Receive(strmReceive, TRUE);
};

void
CCommunicationInterface::Client_PeekSize_Reliable(SLONG &slExpectedSize, SLONG &slReceivedSize) {
  slExpectedSize = cm_ciLocalClient.GetExpectedReliableSize();
  slReceivedSize = cm_ciLocalClient.GetCurrentReliableSize();
}


/*
 *  Client Send/Receive Unreliable
 */
void CCommunicationInterface::Client_Send_Unreliable(const void *pvSend, SLONG slSendSize) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  cm_ciLocalClient.Send(pvSend, slSendSize, FALSE);
};

BOOL CCommunicationInterface::Client_Receive_Unreliable(void *pvReceive, SLONG &slReceiveSize) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  return cm_ciLocalClient.Receive(pvReceive, slReceiveSize, FALSE);
};


BOOL CCommunicationInterface::Client_Update(void) {
  CTSingleLock slComm(&cm_csComm, TRUE);
  CPacket *ppaPacket;
  CPacket *ppaPacketCopy;
  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();

  // update local client's output buffers
  if (cm_ciLocalClient.UpdateOutputBuffers() == FALSE) {
    return FALSE;
  }

  // if not playing on the server (i.e. connectet to a remote server)
  if (!cci_bServerInitialized) {
    // put all pending packets in the master output buffer
    while (cm_ciLocalClient.ci_pbOutputBuffer.pb_ulNumOfPackets > 0) {
      ppaPacket = cm_ciLocalClient.ci_pbOutputBuffer.PeekFirstPacket();
      if (ppaPacket->pa_tvSendWhen < tvNow) {
        cm_ciLocalClient.ci_pbOutputBuffer.RemoveFirstPacket(FALSE);
        if (ppaPacket->pa_ubReliable & UDP_PACKET_RELIABLE) {
          ppaPacketCopy = new CPacket;
          *ppaPacketCopy = *ppaPacket;
          cm_ciLocalClient.ci_pbWaitAckBuffer.AppendPacket(*ppaPacketCopy, FALSE);
        }
        cci_pbMasterOutput.AppendPacket(*ppaPacket, FALSE);

      } else {
        break;
      }
    }

    // update broadcast output buffers
    // update its buffers
    cm_ciBroadcast.UpdateOutputBuffers();
    // transfer packets ready to be sent out to the master output buffer
    while (cm_ciBroadcast.ci_pbOutputBuffer.pb_ulNumOfPackets > 0) {
      ppaPacket = cm_ciBroadcast.ci_pbOutputBuffer.PeekFirstPacket();
      if (ppaPacket->pa_tvSendWhen < tvNow) {
        cm_ciBroadcast.ci_pbOutputBuffer.RemoveFirstPacket(FALSE);
        cci_pbMasterOutput.AppendPacket(*ppaPacket, FALSE);
      } else {
        break;
      }
    }

    // send/receive packets over the TCP/IP stack
    UpdateMasterBuffers();

    // dispatch all packets from the master input buffer to the clients' input buffers
    while (cci_pbMasterInput.pb_ulNumOfPackets > 0) {
      BOOL bClientFound;
      ppaPacket = cci_pbMasterInput.GetFirstPacket();
      bClientFound = FALSE;

      // if the packet address is broadcast and it's an unreliable transfer, put it in the broadcast buffer
      if ((ppaPacket->pa_adrAddress.adr_uwID == SLASHSLASH || ppaPacket->pa_adrAddress.adr_uwID == 0) &&
          ppaPacket->pa_ubReliable == UDP_PACKET_UNRELIABLE) {
        cm_ciBroadcast.ci_pbInputBuffer.AppendPacket(*ppaPacket, FALSE);
        bClientFound = TRUE;
        // if the packet is for this client, accept it
      } else if ((ppaPacket->pa_adrAddress.adr_uwID == cm_ciLocalClient.ci_adrAddress.adr_uwID) ||
                 ppaPacket->pa_adrAddress.adr_uwID == SLASHSLASH ||
                 ppaPacket->pa_adrAddress.adr_uwID == 0) {
        cm_ciLocalClient.ci_pbInputBuffer.AppendPacket(*ppaPacket, FALSE);
        bClientFound = TRUE;
      }
      if (!bClientFound) {
        // warn about possible attack
        extern INDEX net_bReportMiscErrors;
        if (net_bReportMiscErrors) {
          CPrintF(TRANS("WARNING: Invalid message from: %s\n"),
                  AddressToString(ppaPacket->pa_adrAddress.adr_ulAddress));
        }
      }
    }

  }

  cm_ciLocalClient.UpdateInputBuffers();
  cm_ciBroadcast.UpdateInputBuffersBroadcast();

  return TRUE;
};


// update master UDP socket and route its messages
void CCommunicationInterface::UpdateMasterBuffers() {

  UBYTE aub[MAX_PACKET_SIZE];
  CAddress adrIncomingAddress;
  sockaddr_in cliaddr;
  int size = sizeof(cliaddr);
  SLONG slSizeReceived;
  ssize_t slSizeSent;
  BOOL bSomethingDone;
  CPacket *ppaNewPacket;
  CTimerValue tvNow;

  if (cci_bBound) {
    // read from the socket while there is incoming data
    do {

      // initially, nothing is done
      bSomethingDone = FALSE;
      slSizeReceived = recvfrom(cci_hSocket, (char *)aub, MAX_PACKET_SIZE, 0,
                                (struct sockaddr *)&cliaddr, &size);
      tvNow = _pTimer->GetHighPrecisionTimer();

      adrIncomingAddress.adr_ulAddress = ntohl(cliaddr.sin_addr.s_addr);
      adrIncomingAddress.adr_uwPort = ntohs(cliaddr.sin_port);

      //On error, report it to the console (if error is not a no data to read message)
      if (slSizeReceived < 0) {
        // if block received
        if (errno != EAGAIN) {
          CPrintF(TRANS("Socket error during UDP receive. %s (%i)\n"), std::strerror(errno), errno);
        }
      } else {
        //CPrintF("Received %i bytes\n", slSizeReceived);
        if (!cci_bFirstByteReceived) {
          cci_bFirstByteReceived = true;
          CPrintF("Receiving data\n");
        }
        // if there is not at least one byte more in the packet than the header size
        if (slSizeReceived <= MAX_HEADER_SIZE) {
          // the packet is in error
          extern INDEX net_bReportMiscErrors;
          if (net_bReportMiscErrors) {
            CPrintF(TRANS("WARNING: Bad UDP packet from '%s'\n"),
                    AddressToString(adrIncomingAddress.adr_ulAddress));
          }
          // there might be more to do
          bSomethingDone = TRUE;
        } else if (net_fDropPackets <= 0 || (FLOAT(rand()) / RAND_MAX) > net_fDropPackets) {
          // if no packet drop emulation (or the packet is not dropped), form the packet
          // and add it to the end of the UDP Master's input buffer
          ppaNewPacket = new CPacket;
          ppaNewPacket->WriteToPacketRaw(aub, slSizeReceived);
          ppaNewPacket->pa_adrAddress.adr_ulAddress = adrIncomingAddress.adr_ulAddress;
          ppaNewPacket->pa_adrAddress.adr_uwPort = adrIncomingAddress.adr_uwPort;

          if (net_bReportPackets == TRUE) {
            CPrintF("%lu: Received sequence: %d from ID: %d, reliable flag: %d\n",
                    (ULONG) tvNow.GetMilliseconds(), ppaNewPacket->pa_ulSequence,
                    ppaNewPacket->pa_adrAddress.adr_uwID, ppaNewPacket->pa_ubReliable);
          }

          cci_pbMasterInput.AppendPacket(*ppaNewPacket, FALSE);
          // there might be more to do
          bSomethingDone = TRUE;

        }
      }

    } while (bSomethingDone);
  }

  // write from the output buffer to the socket
  while (cci_pbMasterOutput.pb_ulNumOfPackets > 0) {
    ppaNewPacket = cci_pbMasterOutput.PeekFirstPacket();

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = htonl(ppaNewPacket->pa_adrAddress.adr_ulAddress);
    cliaddr.sin_port = htons(ppaNewPacket->pa_adrAddress.adr_uwPort);

    slSizeSent = sendto(cci_hSocket, (char *) ppaNewPacket->pa_pubPacketData,
                        (int) ppaNewPacket->pa_slSize, 0, (const struct sockaddr *) &cliaddr, sizeof(cliaddr));
    cci_bBound = TRUE;   // UDP socket that did a send is considered bound
    tvNow = _pTimer->GetHighPrecisionTimer();
    //CPrintF("Sent %i bytes\n", slSizeSent);

    // if some error
    if (slSizeSent < 0) {
      CPrintF(TRANS("Socket error during UDP send. %s (%i)\n"), std::strerror(errno), errno);
      return;
      // if all sent ok
    } else {

      if (net_bReportPackets == TRUE) {
        CPrintF("%lu: Sent sequence: %d to ID: %d, reliable flag: %d\n", (ULONG) tvNow.GetMilliseconds(), ppaNewPacket->pa_ulSequence, ppaNewPacket->pa_adrAddress.adr_uwID, ppaNewPacket->pa_ubReliable);
      }

      cci_pbMasterOutput.RemoveFirstPacket(TRUE);
      bSomethingDone = TRUE;
    }

  }

};


