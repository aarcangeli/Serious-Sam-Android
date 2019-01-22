#include <Engine/StdH.h>


#include <Engine/Network/CommunicationInterface.h>
#include <Engine/Base/CTString.h>
#include <Engine/Network/ClientInterface.h>

CPacketBufferStats _pbsSend;
CPacketBufferStats _pbsRecv;
CCommunicationInterface _cmiComm;

// index 0 is the server's local client, this is an array used by server only
CClientInterface cm_aciClients[SERVER_CLIENTS];

CCommunicationInterface::CCommunicationInterface(void) {};


void CCommunicationInterface::Init(void) {};

void CCommunicationInterface::Close(void) {};

void CCommunicationInterface::InitWinsock(void) {};

void CCommunicationInterface::EndWinsock(void) {};


void CCommunicationInterface::PrepareForUse(BOOL bUseNetwork, BOOL bClient) {};

void CCommunicationInterface::Unprepare(void) {};

BOOL CCommunicationInterface::IsNetworkEnabled(void) {};

void CCommunicationInterface::GetHostName(CTString &strName, CTString &strAddress) {};

void CCommunicationInterface::CreateSocket_t() {};

void CCommunicationInterface::Bind_t(ULONG ulLocalHost, ULONG ulLocalPort) {};

void CCommunicationInterface::SetNonBlocking_t(void) {};

CTString CCommunicationInterface::GetSocketError(INDEX iError) {};

void CCommunicationInterface::OpenSocket_t(ULONG ulLocalHost, ULONG ulLocalPort) {};

void CCommunicationInterface::GetLocalAddress_t(ULONG &ulHost, ULONG &ulPort) {}

void CCommunicationInterface::GetRemoteAddress_t(ULONG &ulHost, ULONG &ulPort) {}


void CCommunicationInterface::Broadcast_Send(const void *pvSend, SLONG slSendSize,
                                             CAddress &adrDestination) {}

BOOL CCommunicationInterface::Broadcast_Receive(void *pvReceive, SLONG &slReceiveSize,
                                                CAddress &adrAddress) {}

void CCommunicationInterface::Broadcast_Update_t() {};

void CCommunicationInterface::Server_Init_t(void) {};

void CCommunicationInterface::Server_Close(void) {};

void CCommunicationInterface::Server_ClearClient(INDEX iClient) {};

BOOL CCommunicationInterface::Server_IsClientLocal(INDEX iClient) {};

BOOL CCommunicationInterface::Server_IsClientUsed(INDEX iClient) {};

CTString CCommunicationInterface::Server_GetClientName(INDEX iClient) {};

void CCommunicationInterface::Server_Send_Reliable(INDEX iClient, const void *pvSend,
                                                   SLONG slSendSize) {};

BOOL CCommunicationInterface::Server_Receive_Reliable(INDEX iClient, void *pvReceive,
                                                      SLONG &slReceiveSize) {};

void CCommunicationInterface::Server_Send_Unreliable(INDEX iClient, const void *pvSend,
                                                     SLONG slSendSize) {};

BOOL CCommunicationInterface::Server_Receive_Unreliable(INDEX iClient, void *pvReceive,
                                                        SLONG &slReceiveSize) {};


BOOL CCommunicationInterface::Server_Update() {};


void CCommunicationInterface::Client_Init_t(char *strServerName) {};

void CCommunicationInterface::Client_Init_t(ULONG ulServerAddress) {};

void CCommunicationInterface::Client_Close(void) {};


void CCommunicationInterface::Client_OpenLocal(void) {};


void CCommunicationInterface::Client_OpenNet_t(ULONG ulServerAddress) {};


void CCommunicationInterface::Client_Clear(void) {};

BOOL CCommunicationInterface::Client_IsConnected(void) {};

void CCommunicationInterface::Client_Send_Reliable(const void *pvSend, SLONG slSendSize) {};

BOOL CCommunicationInterface::Client_Receive_Reliable(void *pvReceive, SLONG &slReceiveSize) {};

BOOL CCommunicationInterface::Client_Receive_Reliable(CTStream &strmReceive) {};

void
CCommunicationInterface::Client_PeekSize_Reliable(SLONG &slExpectedSize, SLONG &slReceivedSize) {}


void CCommunicationInterface::Client_Send_Unreliable(const void *pvSend, SLONG slSendSize) {};


BOOL CCommunicationInterface::Client_Receive_Unreliable(void *pvReceive, SLONG &slReceiveSize) {};


BOOL CCommunicationInterface::Client_Update(void) {};


void CCommunicationInterface::UpdateMasterBuffers() {};


// convert address to a printable string
CTString AddressToString(ULONG ulHost) {
    return "";
};
