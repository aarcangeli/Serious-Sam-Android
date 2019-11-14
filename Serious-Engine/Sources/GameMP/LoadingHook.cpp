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


#include "stdafx.h"
#include "LCDDrawing.h"
#include <locale.h>
#include <AndroidAdapters/binding-callbacks.h>

#define USECUSTOMTEXT 0

extern CGame *_pGame;

#if USECUSTOMTEXT
  static CTString _strCustomText = "";
#endif
static CDrawPort *_pdpLoadingHook = NULL;  // drawport for loading hook
extern BOOL _bUserBreakEnabled;
extern BOOL map_bIsFirstEncounter;


#define REFRESHTIME (0.2f)

void RemapLevelNames(INDEX &iLevel)
{
  switch( iLevel) {
  case 10:  iLevel =  1;  break;
  case 11:  iLevel =  2;  break;
  case 12:  iLevel =  3;  break;
  case 13:  iLevel =  4;  break;
  case 14:  iLevel =  5;  break;
  case 15:  iLevel =  6;  break;
  case 21:  iLevel =  7;  break;
  case 22:  iLevel =  8;  break;
  case 23:  iLevel =  9;  break;
  case 24:  iLevel = 10;  break;
  case 31:  iLevel = 11;  break;
  case 32:  iLevel = 12;  break;
  case 33:  iLevel = 13;  break;
  default:  iLevel = -1;	break;
  }
}


static void LoadingHook_t(CProgressHookInfo *pphi)
{
  float uiScale = g_cb.globalScale;

  // if user presses escape
  ULONG ulCheckFlags = 0x8000;
  if (pphi->phi_fCompleted>0) {
    ulCheckFlags |= 0x0001;
  }
  if (_bUserBreakEnabled && _pGame->isTouchingScreen) {
    // break loading
    throw TRANS("User break!");
  }

#if USECUSTOMTEXT
  // if no custom loading text
  if (_strCustomText=="") {
    // load it
    try {
      _strCustomText.Load_t(CTFILENAME("Data\\LoadingText.txt"));
    } catch ( const char *strError) {
      _strCustomText = strError;
    }
  }
#endif

  // measure time since last call
  static CTimerValue tvLast(0.0);
  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer();

  // if not first or final update, and not enough time passed
  if (pphi->phi_fCompleted!=0 && pphi->phi_fCompleted!=1 &&
     (tvNow-tvLast).GetSeconds() < REFRESHTIME) {
    // do nothing
    return;
  }
  tvLast = tvNow;

  // skip if cannot lock drawport
  CDrawPort *pdp = _pdpLoadingHook;                           
  ASSERT(pdp!=NULL);
  CDrawPort dpHook(pdp, TRUE);
  if( !dpHook.Lock()) return;

  // clear screen
  dpHook.Fill(C_BLACK|255);

  // get session properties currently loading
  CSessionProperties *psp = (CSessionProperties *)_pNetwork->GetSessionProperties();
  ULONG ulLevelMask = psp->sp_ulLevelsMask;
  if (psp->sp_bCooperative) {
    INDEX iLevel = -1;
    INDEX iLevelNext = -1;
    CTString strLevelName = _pNetwork->ga_fnmWorld.FileName();
    CTString strNextLevelName = _pNetwork->ga_fnmNextLevel.FileName();
    
    // second encounter
    INDEX u, v;
    u = v = -1;
    strLevelName.ScanF("%01d_%01d_", &u, &v);
    iLevel = u*10+v;
    RemapLevelNames(iLevel);
    u = v = -1;
    strNextLevelName.ScanF("%01d_%01d_", &u, &v);
    iLevelNext = u*10+v;
    RemapLevelNames(iLevelNext);

    // first encounter
    if(iLevel == -1) {
      strLevelName.ScanF("%02d_", &iLevel);
      strNextLevelName.ScanF("%02d_", &iLevelNext);

      if(iLevel != -1) {
        map_bIsFirstEncounter = TRUE;
      }
    } else {
      map_bIsFirstEncounter = FALSE;
    }
   
    if (iLevel>0) {
      ulLevelMask|=1<<(iLevel-1);
    }
    if (iLevelNext>0) {
      ulLevelMask|=1<<(iLevelNext-1);
    }
  }

  if (ulLevelMask!=0 && !_pNetwork->IsPlayingDemo()) {
    // map hook
    extern void RenderMap( CDrawPort *pdp, ULONG ulLevelMask, CProgressHookInfo *pphi);
    RenderMap(&dpHook, ulLevelMask, pphi);

    // finish rendering
    dpHook.Unlock();
    dpHook.dp_Raster->ra_pvpViewPort->SwapBuffers();

    // keep current time
    tvLast = _pTimer->GetHighPrecisionTimer();
    return;
  }

  // get sizes
  PIX pixSizeI = dpHook.GetWidth();
  PIX pixSizeJ = dpHook.GetHeight();
  CFontData *pfd = _pfdConsoleFont;
  PIX pixCharSizeI = (PIX)((pfd->fd_pixCharWidth  + pfd->fd_pixCharSpacing) * uiScale);
  PIX pixCharSizeJ = (PIX)((pfd->fd_pixCharHeight + pfd->fd_pixLineSpacing) * uiScale);

  PIX pixBarSizeJ = (PIX) (17 * uiScale);//*pixSizeJ/480;

  COLOR colBcg = LerpColor(C_BLACK, SE_COL_BLUE_LIGHT, 0.30f)|0xff;
  COLOR colBar = LerpColor(C_BLACK, SE_COL_BLUE_LIGHT, 0.45f)|0xff;
  COLOR colLines = colBar; //C_vdGREEN|0xff;
  COLOR colText = LerpColor(C_BLACK, SE_COL_BLUE_LIGHT, 0.95f)|0xff;
  COLOR colEsc = C_WHITE|0xFF;

  dpHook.Fill(0, pixSizeJ-pixBarSizeJ, pixSizeI, pixBarSizeJ, colBcg);
  dpHook.Fill(0, pixSizeJ-pixBarSizeJ, pixSizeI*pphi->phi_fCompleted, pixBarSizeJ, colBar);
  dpHook.DrawBorder(0, pixSizeJ-pixBarSizeJ, pixSizeI, pixBarSizeJ, colLines);

  dpHook.SetFont( _pfdConsoleFont);
  dpHook.SetTextScaling(uiScale);
  dpHook.SetTextAspect( 1.0f);
  // print status text
  setlocale(LC_ALL, "");
  CTString strDesc(0, "%s", pphi->phi_strDescription);  strupr((char*)(const char*)strDesc);
  setlocale(LC_ALL, "C");
  CTString strPerc(0, "%3.0f%%", pphi->phi_fCompleted*100);
  //dpHook.PutText(strDesc, pixCharSizeI/2, pixSizeJ-pixBarSizeJ-2-pixCharSizeJ, C_GREEN|255);
  //dpHook.PutTextCXY(strPerc, pixSizeI/2, pixSizeJ-pixBarSizeJ/2+1, C_GREEN|255);
  dpHook.PutText(strDesc, pixCharSizeI/2, pixSizeJ-pixBarSizeJ+pixCharSizeJ/2, colText);
  dpHook.PutTextR(strPerc, pixSizeI-pixCharSizeI/2, pixSizeJ-pixBarSizeJ+pixCharSizeJ/2, colText);
  if (_bUserBreakEnabled && !_pGame->gm_bFirstLoading) {
    dpHook.PutTextC( TRANS( "TOUCH THE SCREEN TO ABORT"), pixSizeI/2, pixSizeJ-pixBarSizeJ-2-pixCharSizeJ, colEsc);
  }

/*  
  //LCDPrepare(1.0f);
  //LCDSetDrawport(&dpHook);
  
  // fill the box with background dirt and grid
  //LCDRenderClouds1();
  //LCDRenderGrid();

  // draw progress bar
  PIX pixBarCentI = pixBoxSizeI*1/2;
  PIX pixBarCentJ = pixBoxSizeJ*3/4;
  PIX pixBarSizeI = pixBoxSizeI*7/8;
  PIX pixBarSizeJ = pixBoxSizeJ*3/8;
  PIX pixBarMinI = pixBarCentI-pixBarSizeI/2;
  PIX pixBarMaxI = pixBarCentI+pixBarSizeI/2;
  PIX pixBarMinJ = pixBarCentJ-pixBarSizeJ/2;
  PIX pixBarMaxJ = pixBarCentJ+pixBarSizeJ/2;

  dpBox.Fill(pixBarMinI, pixBarMinJ, 
    pixBarMaxI-pixBarMinI, pixBarMaxJ-pixBarMinJ, C_BLACK|255);
  dpBox.Fill(pixBarMinI, pixBarMinJ, 
    (pixBarMaxI-pixBarMinI)*pphi->phi_fCompleted, pixBarMaxJ-pixBarMinJ, C_GREEN|255);

  // put more dirt
  LCDRenderClouds2Light();

  // draw borders
  COLOR colBorders = LerpColor(C_GREEN, C_BLACK, 200);
  LCDDrawBox(0,-1, PIXaabbox2D(
    PIX2D(pixBarMinI, pixBarMinJ), 
    PIX2D(pixBarMaxI, pixBarMaxJ)), 
    colBorders|255);
  LCDDrawBox(0,-1, PIXaabbox2D(
    PIX2D(0,0), PIX2D(dpBox.GetWidth(), dpBox.GetHeight())), 
    colBorders|255);

  // print status text
  dpBox.SetFont( _pfdDisplayFont);
  dpBox.SetTextScaling( 1.0f);
  dpBox.SetTextAspect( 1.0f);
  // print status text
  CTString strRes;
  strRes.PrintF( "%s", pphi->phi_strDescription);
  //strupr((char*)(const char*)strRes);
  dpBox.PutTextC( strRes, 160, 17, C_GREEN|255);
  strRes.PrintF( "%3.0f%%", pphi->phi_fCompleted*100);
  dpBox.PutTextCXY( strRes, pixBarCentI, pixBarCentJ, C_GREEN|255);
  dpBox.Unlock();

  if( Flesh.gm_bFirstLoading) {
#if USECUSTOMTEXT
    FLOAT fScaling = (FLOAT)slSizeI/640.0f;
    dpHook.Lock();
    dpHook.SetFont( _pfdDisplayFont);
    dpHook.SetTextScaling( fScaling);
    dpHook.SetTextAspect( 1.0f);
    //dpHook.Fill( 0, 0, slSizeI, pixCenterJ, C_vdGREEN|255, C_vdGREEN|255, C_vdGREEN|0, C_vdGREEN|0);
    dpHook.PutTextC( TRANS( "SERIOUS SAM - TEST VERSION"), pixCenterI, 5*fScaling, C_WHITE|255);
    dpHook.PutTextC( TRANS( "THIS IS NOT A DEMO VERSION, THIS IS A COMPATIBILITY TEST!"), pixCenterI, 25*fScaling, C_WHITE|255);
    dpHook.PutTextC( TRANS( "Serious Sam (c) 2000 Croteam LLC, All Rights Reserved.\n"), pixCenterI, 45*fScaling, C_WHITE|255);
    dpHook.PutText( _strCustomText, 1*fScaling, 85*fScaling, C_GREEN|255);
    dpHook.Unlock();
#endif
  } else if (_bUserBreakEnabled) {
    FLOAT fScaling = (FLOAT)slSizeI/640.0f;
    dpHook.Lock();
    dpHook.SetFont( _pfdDisplayFont);
    dpHook.SetTextScaling( fScaling);
    dpHook.SetTextAspect( 1.0f);
    //dpHook.Fill( 0, 0, slSizeI, pixCenterJ, C_vdGREEN|255, C_vdGREEN|255, C_vdGREEN|0, C_vdGREEN|0);
    dpHook.PutTextC( TRANS( "PRESS ESC TO ABORT"), pixCenterI, pixCenterJ+pixBoxSizeJ+5*fScaling, C_WHITE|255);
  }
  */

  dpHook.Unlock();
  // finish rendering
  dpHook.dp_Raster->ra_pvpViewPort->SwapBuffers();

  // keep current time
  tvLast = _pTimer->GetHighPrecisionTimer();
}

// loading hook functions
void CGame::EnableLoadingHook(CDrawPort *pdpDrawport)
{
  _pdpLoadingHook = pdpDrawport;
  SetProgressHook(LoadingHook_t);
}

void CGame::DisableLoadingHook(void)
{
  SetProgressHook(NULL);
  _pdpLoadingHook = NULL;
}
