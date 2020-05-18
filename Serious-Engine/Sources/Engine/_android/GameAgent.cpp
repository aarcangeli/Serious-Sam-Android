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

#include "StdH.h"

#include <Engine/Engine.h>
#include <Engine/CurrentVersion.h>
#include <Engine/Entities/Entity.h>
#include <Engine/Base/Shell.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/CTString.h>
#include <Engine/Network/Server.h>
#include <Engine/Network/Network.h>
#include <Engine/Network/SessionState.h>
#include <GameMP/SessionProperties.h>
#include <Engine/GameAgent/GameAgent.h>

CTString ga_strServer = "master1.42amsterdam.net";
CTString ga_strMSLegacy = "42amsterdam.net";
BOOL ga_bMSLegacy = TRUE;

const CSessionProperties *_getSP() {
return 0;
}

CServerRequest::CServerRequest(void) {}

CServerRequest::~CServerRequest(void) {}

void CServerRequest::Clear(void) {}


extern void GameAgent_ServerInit(void) {}


extern void GameAgent_ServerEnd(void) {}


extern void GameAgent_ServerUpdate(void) {}


extern void GameAgent_ServerStateChanged(void) {}


extern void GameAgent_EnumTrigger(BOOL bInternet) {}


extern void GameAgent_EnumUpdate(void) {}


extern void GameAgent_EnumCancel(void) {}
