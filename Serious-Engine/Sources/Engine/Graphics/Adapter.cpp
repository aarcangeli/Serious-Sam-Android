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

#include "Engine/StdH.h"

#include <Engine/Graphics/Adapter.h>
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Base/Translation.h>
#include <Engine/Base/Console.h>


extern BOOL _bDedicatedServer;

// list of all modes avaliable through CDS
static CListHead _lhCDSModes;

class CResolution {
public:
    PIX re_pixSizeI;
    PIX re_pixSizeJ;
};

static CResolution _areResolutions[] =
        {
                {320,  240},
                {400,  300},
                {480,  360},
                {512,  384},
                {640,  480},
                {720,  540},
                {720,  576},
                {800,  600},
                {960,  720},
                {1024, 768},
                {1152, 864},
                {1280, 960},
                {1280, 1024},
                {1600, 1200},
                {1792, 1344},
                {1856, 1392},
                {1920, 1440},
                {2048, 1536},


                {1280, 480},
                {1600, 600},
                {2048, 768},


                {848,  480},
                {856,  480},
        };

static const INDEX MAX_RESOLUTIONS = sizeof(_areResolutions) / sizeof(_areResolutions[0]);



// initialize CDS support (enumerate modes at startup)
void CGfxLibrary::InitAPIs(void) {
  WarningMessage("CGfxLibrary::InitAPIs");
  gl_gaAPI[GAT_OGL].ga_ctAdapters = 1;
  gl_gaAPI[GAT_OGL].ga_iCurrentAdapter = 0;

  CDisplayAdapter *pda = &gl_gaAPI[GAT_OGL].ga_adaAdapter[0];
  pda->da_ulFlags = DAF_USEGDIFUNCTIONS;
  pda->da_strVendor   = TRANS( "unknown");
  pda->da_strRenderer = TRANS( "Default ICD");
  pda->da_strVersion  = "1.1+";

  pda->da_ctDisplayModes = 0;
  pda->da_iCurrentDisplayMode = -1;

  // make a new display mode
  CDisplayMode &dm = pda->da_admDisplayModes[pda->da_ctDisplayModes];
  dm.dm_pixSizeI = 1920;
  dm.dm_pixSizeJ = 1080;
  dm.dm_ddDepth  = DD_DEFAULT;
  pda->da_ctDisplayModes++;
}

// get list of all modes avaliable through CDS -- do not modify/free the returned list
CListHead &CDS_GetModes(void)
{
  return _lhCDSModes;
}


// set given display mode
BOOL CDS_SetMode(PIX pixSizeI, PIX pixSizeJ, enum DisplayDepth dd) {
  WarningMessage("CDS_SetMode");
  return true;
}

// reset windows to mode chosen by user within windows diplay properties
void CDS_ResetMode(void) {
  WarningMessage("CDS_ResetMode");
}


