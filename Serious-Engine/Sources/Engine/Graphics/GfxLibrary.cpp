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
#include <Engine/Base/CTString.h>

#include <Engine/Graphics/GfxLibrary.h>

#include <Engine/Base/Translation.h>
#include <Engine/Base/ErrorReporting.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/Shell.h>
#include <Engine/Base/Statistics_Internal.h>
#include <Engine/Base/ListIterator.inl>
#include <Engine/Math/Functions.h>
#include <Engine/Math/AABBox.h>
#include <Engine/Models/Normals.h>
#include <Engine/Sound/SoundLibrary.h>
#include <Engine/Templates/Stock_CModelData.h>

#include <Engine/Graphics/Adapter.h>
#include <Engine/Graphics/ShadowMap.h>
#include <Engine/Graphics/Texture.h>
#include <Engine/Graphics/GfxProfile.h>
#include <Engine/Graphics/Raster.h>
#include <Engine/Graphics/ViewPort.h>
#include <Engine/Graphics/DrawPort.h>
#include <Engine/Graphics/Color.h>
#include <Engine/Graphics/Font.h>
#include <Engine/Graphics/MultiMonitor.h>

#include <Engine/Templates/DynamicStackArray.h>
#include <Engine/Templates/DynamicStackArray.cpp>
#include <Engine/Templates/DynamicContainer.cpp>
#include <Engine/Templates/Stock_CTextureData.h>


// control for partial usage of compiled vertex arrays
extern BOOL CVA_b2D     = FALSE;
extern BOOL CVA_bWorld  = FALSE;
extern BOOL CVA_bModels = FALSE;    

// common element arrays
CStaticStackArray<GFXVertex>   _avtxCommon;      
CStaticStackArray<GFXTexCoord> _atexCommon;   
CStaticStackArray<GFXColor>    _acolCommon;      
CStaticStackArray<INDEX>       _aiCommonElements;
CStaticStackArray<INDEX>       _aiCommonQuads;   // predefined array for rendering quads thru triangles in glDrawElements()

// global texture parameters
CTexParams _tpGlobal[GFX_MAXTEXUNITS];  

// pointer to global graphics library object
CGfxLibrary *_pGfx = NULL;

// forced texture upload quality (0 = default, 16 = force 16-bit, 32 = force 32-bit)
INDEX _iTexForcedQuality = 0;

extern PIX _fog_pixSizeH;
extern PIX _fog_pixSizeL;
extern PIX _haze_pixSize;

// control for partial usage of compiled vertex arrays
extern BOOL CVA_b2D;
extern BOOL CVA_bWorld;
extern BOOL CVA_bModels;

// gamma control
static FLOAT _fLastBrightness, _fLastContrast, _fLastGamma;
static FLOAT _fLastBiasR, _fLastBiasG, _fLastBiasB;
static INDEX _iLastLevels;
static UWORD _auwGammaTable[256*3];

// table for clipping [-512..+1024] to [0..255]
static UBYTE aubClipByte[256*2+ 256 +256*3];
extern const UBYTE *pubClipByte = &aubClipByte[256*2];

// fast square root and 1/sqrt tables
UBYTE aubSqrt[SQRTTABLESIZE];    
UWORD auw1oSqrt[SQRTTABLESIZE];

// table for conversion from compressed 16-bit gouraud normals to 8-bit
UBYTE aubGouraudConv[16384];

// flag for scene rendering in progress (i.e. between 1st lock in frame & swap-buffers)
static BOOL GFX_bRenderingScene = FALSE;
// ID of the last drawport that has been locked
static ULONG GFX_ulLastDrawPortID = 0;  

// last size of vertex buffers
extern INDEX _iLastVertexBufferSize = 0;
// pretouch flag
extern BOOL _bNeedPretouch;

// flat texture
CTextureData *_ptdFlat = NULL;
static ULONG _ulWhite = 0xFFFFFFFF;

// fast sin/cos table
static FLOAT afSinTable[256+256+64];
extern const FLOAT *pfSinTable = afSinTable +256;
extern const FLOAT *pfCosTable = afSinTable +256+64;

// texture/shadow control
extern INDEX tex_iNormalQuality    = 00;      // 0=optimal, 1=16bit, 2=32bit, 3=compressed (1st num=opaque tex, 2nd=alpha tex)
extern INDEX tex_iAnimationQuality = 11;      // 0=optimal, 1=16bit, 2=32bit, 3=compressed (1st num=opaque tex, 2nd=alpha tex)
extern INDEX tex_iNormalSize     = 9;         // log2 of texture area /2 for max texture size allowed
extern INDEX tex_iAnimationSize  = 7; 
extern INDEX tex_iEffectSize     = 7; 
extern INDEX tex_bDynamicMipmaps = FALSE;     // how many mipmaps will be bilineary filtered (0-15)
extern INDEX tex_iDithering      = 3;         // 0=none, 1-3=low, 4-7=medium, 8-10=high
extern INDEX tex_bFineEffect = FALSE;         // 32bit effect? (works only if base texture hasn't been dithered)
extern INDEX tex_bFineFog = TRUE;             // should fog be 8/32bit? (or just plain 4/16bit)
extern INDEX tex_iFogSize = 7;                // limit fog texture size 
extern INDEX tex_iFiltering = 0;              // -6 - +6; negative = sharpen, positive = blur, 0 = none
extern INDEX tex_iEffectFiltering = +4;       // filtering of fire effect textures
extern INDEX tex_bProgressiveFilter = FALSE;  // filter mipmaps in creation time (not afterwards)
extern INDEX tex_bColorizeMipmaps   = FALSE;  // DEBUG: colorize texture's mipmap levels in various colors
extern INDEX tex_bCompressAlphaChannel = FALSE;  // for compressed textures, compress alpha channel too   
extern INDEX tex_bAlternateCompression = FALSE;  // basically, this is fix for GFs (compress opaque texture as translucent)

extern INDEX shd_iStaticSize  = 8;    
extern INDEX shd_iDynamicSize = 8;    
extern INDEX shd_bFineQuality = FALSE; 
extern INDEX shd_iFiltering = 3;     // >0 = blurring, 0 = no filtering
extern INDEX shd_iDithering = 1;     // 0=none, 1,2=low, 3,4=medium, 5=high
extern INDEX shd_iAllowDynamic = 1;    // 0=disallow, 1=allow on polys w/o 'NoDynamicLights' flag, 2=allow unconditionally
extern INDEX shd_bDynamicMipmaps = TRUE;
extern FLOAT shd_tmFlushDelay = 30.0f; // in seconds
extern FLOAT shd_fCacheSize   = 8.0f;  // in megabytes
extern INDEX shd_bCacheAll    = FALSE; // cache all shadowmap at the level loading time (careful - memory eater!)
extern INDEX shd_bAllowFlats = TRUE;   // allow optimization of single-color shadowmaps
extern INDEX shd_iForceFlats = 0;      // force all shadowmaps to be flat (internal!) - 0=don't, 1=w/o overbrighting, 2=w/ overbrighting
extern INDEX shd_bShowFlats  = FALSE;  // colorize flat shadows
extern INDEX shd_bColorize   = FALSE;  // colorize shadows by size (gradieng from red=big to green=little)


// OpenGL control
extern INDEX ogl_iTextureCompressionType  = 1;    // 0=none, 1=default (ARB), 2=S3TC, 3=FXT1, 4=old S3TC
extern INDEX ogl_bUseCompiledVertexArrays = 101;  // =XYZ; X=2D, Y=world, Z=models
extern INDEX ogl_bAllowQuadArrays = FALSE;
extern INDEX ogl_bExclusive = TRUE;
extern INDEX ogl_bGrabDepthBuffer = FALSE;
extern INDEX ogl_iMaxBurstSize = 0;        // unlimited
extern INDEX ogl_bTruformLinearNormals = TRUE;
extern INDEX ogl_bAlternateClipPlane = FALSE; // signal when user clip plane requires a texture unit

extern INDEX ogl_iTBufferEffect  = 0;      // 0=none, 1=partial FSAA, 2=Motion Blur
extern INDEX ogl_iTBufferSamples = 2;      // 2, 4 or 8 (for now)
extern INDEX ogl_iFinish = 1;              // 0=never, 1=before rendering of next frame, 2=at the end of this frame, 3=at projection change

// Direct3D control
extern INDEX d3d_bUseHardwareTnL = TRUE;
extern INDEX d3d_bAlternateDepthReads = FALSE;  // should check delayed depth reads at the end of current frame (FALSE) or at begining of the next (TRUE)
extern INDEX d3d_iVertexBuffersSize   = 1024;   // KBs reserved for vertex buffers
extern INDEX d3d_iVertexRangeTreshold = 99;     // minimum vertices in buffer that triggers range optimization
extern INDEX d3d_bFastUpload = TRUE;            // use internal format conversion routines
extern INDEX d3d_iMaxBurstSize = 0;             // 0=unlimited
extern INDEX d3d_iFinish = 0;

// API common controls
extern INDEX gap_iUseTextureUnits = 4;
extern INDEX gap_iTextureFiltering  = 21;       // bilinear by default
extern INDEX gap_iTextureAnisotropy = 1;        // 1=isotropic, 2=min anisotropy
extern FLOAT gap_fTextureLODBias    = 0.0f;
extern INDEX gap_bOptimizeStateChanges = TRUE;
extern INDEX gap_iOptimizeDepthReads = 1;        // 0=imediately, 1=after frame, 2=every 0.1 seconds
extern INDEX gap_iOptimizeClipping   = 2;        // 0=no, 1=mirror plane only, 2=mirror and frustum
extern INDEX gap_bAllowGrayTextures = TRUE;
extern INDEX gap_bAllowSingleMipmap = TRUE;
extern INDEX gap_iSwapInterval = 0;
extern INDEX gap_iRefreshRate  = 0;
extern INDEX gap_iDithering = 2;        // 16-bit dithering: 0=none, 1=no alpha, 2=all
extern INDEX gap_bForceTruform = 0;     // 0 = only for models that allow truform, 1=for every model
extern INDEX gap_iTruformLevel = 3;     // 0 = no tesselation
extern INDEX gap_iDepthBits   = 0;      // 0 (as color depth), 16, 24 or 32
extern INDEX gap_iStencilBits = 0;      // 0 (no stencil buffer), 4 or 8

// models control
extern INDEX mdl_bShowTriangles    = FALSE;
extern INDEX mdl_bShowStrips       = FALSE;
extern INDEX mdl_bTruformWeapons   = FALSE;
extern INDEX mdl_bCreateStrips     = TRUE;
extern INDEX mdl_bRenderDetail     = TRUE;
extern INDEX mdl_bRenderSpecular   = TRUE;
extern INDEX mdl_bRenderReflection = TRUE;
extern INDEX mdl_bRenderBump       = TRUE;
extern INDEX mdl_bAllowOverbright  = TRUE;
extern INDEX mdl_bFineQuality      = TRUE;
extern INDEX mdl_iShadowQuality    = 1;
extern FLOAT mdl_fLODMul           = 1.0f;
extern FLOAT mdl_fLODAdd           = 0.0f;
extern INDEX mdl_iLODDisappear     = 1; // 0=never, 1=ignore bias, 2=with bias
// ska controls
extern INDEX ska_bShowSkeleton     = FALSE;
extern INDEX ska_bShowColision     = FALSE;
extern FLOAT ska_fLODMul           = 1.0f;
extern FLOAT ska_fLODAdd           = 0.0f;
// terrain controls
extern INDEX ter_bShowQuadTree     = FALSE;
extern INDEX ter_bShowWireframe    = FALSE;
extern INDEX ter_bLerpVertices     = TRUE;
extern INDEX ter_bShowInfo         = FALSE;
extern INDEX ter_bOptimizeRendering = TRUE;
extern INDEX ter_bTempFreezeCast   = FALSE;
extern INDEX ter_bNoRegeneration   = FALSE;

// rendering control
extern INDEX wld_bAlwaysAddAll         = FALSE;
extern INDEX wld_bRenderMirrors        = TRUE;
extern INDEX wld_bRenderEmptyBrushes   = TRUE;
extern INDEX wld_bRenderShadowMaps     = TRUE;
extern INDEX wld_bRenderTextures       = TRUE;
extern INDEX wld_bRenderDetailPolygons = TRUE;
extern INDEX wld_bTextureLayers        = 111;
extern INDEX wld_bShowTriangles        = FALSE;
extern INDEX wld_bShowDetailTextures   = FALSE;
extern INDEX wld_iDetailRemovingBias   = 3;
extern FLOAT wld_fEdgeOffsetI          = 0.0f; //0.125f;
extern FLOAT wld_fEdgeAdjustK          = 1.0f; //1.0001f;
                                     
extern INDEX gfx_bRenderWorld      = TRUE;
extern INDEX gfx_bRenderParticles  = TRUE;
extern INDEX gfx_bRenderModels     = TRUE;
extern INDEX gfx_bRenderPredicted  = FALSE;
extern INDEX gfx_bRenderFog        = TRUE;
extern INDEX gfx_iLensFlareQuality = 3;   // 0=none, 1=corona only, 2=corona and reflections, 3=corona, reflections and glare 

extern INDEX gfx_bDecoratedText   = TRUE;
extern INDEX gfx_bClearScreen = FALSE;
extern FLOAT gfx_tmProbeDecay = 30.0f;   // seconds
extern INDEX gfx_iProbeSize   = 256;     // in KBs

extern INDEX gfx_ctMonitors = 0;
extern INDEX gfx_bMultiMonDisabled = FALSE;
extern INDEX gfx_bDisableMultiMonSupport = TRUE;

extern INDEX gfx_bDisableWindowsKeys = TRUE;

extern INDEX wed_bIgnoreTJunctions = FALSE;
extern INDEX wed_bUseBaseForReplacement = FALSE;

// some nifty features
extern INDEX gfx_iHueShift   = 0;       // 0-359
extern FLOAT gfx_fSaturation = 1.0f;    // 0.0f = min, 1.0f = default
// the following vars can be influenced by corresponding gfx_ vars
extern INDEX tex_iHueShift   = 0;       // added to gfx_
extern FLOAT tex_fSaturation = 1.0f;    // multiplied by gfx_
extern INDEX shd_iHueShift   = 0;       // added to gfx_
extern FLOAT shd_fSaturation = 1.0f;    // multiplied by gfx_

// gamma table control
extern FLOAT gfx_fBrightness = 0.0f;    // -0.9 - 0.9
extern FLOAT gfx_fContrast   = 1.0f;    //  0.1 - 1.9
extern FLOAT gfx_fGamma      = 1.0f;    //  0.1 - 9.0
extern FLOAT gfx_fBiasR  = 1.0f;        //  0.0 - 1.0
extern FLOAT gfx_fBiasG  = 1.0f;        //  0.0 - 1.0
extern FLOAT gfx_fBiasB  = 1.0f;        //  0.0 - 1.0
extern INDEX gfx_iLevels = 256;         //    2 - 256

// stereo rendering control
extern INDEX gfx_iStereo = 0;                  // 0=off, 1=red/cyan
extern INDEX gfx_bStereoInvert = FALSE;        // is left eye RED or CYAN
extern INDEX gfx_iStereoOffset = 10;           // view offset (or something:)
extern FLOAT gfx_fStereoSeparation =  0.25f;   // distance between eyes

// cached integers for faster calculation
extern SLONG _slTexSaturation = 256;  // 0 = min, 256 = default
extern SLONG _slTexHueShift   = 0;
extern SLONG _slShdSaturation = 256; 
extern SLONG _slShdHueShift   = 0;

// 'supported' console variable flags
static INDEX sys_bHasTextureCompression = 0;
static INDEX sys_bHasTextureAnisotropy = 0;
static INDEX sys_bHasAdjustableGamma = 0;
static INDEX sys_bHasTextureLODBias = 0;
static INDEX sys_bHasMultitexturing = 0;
static INDEX sys_bHas32bitTextures = 0;
static INDEX sys_bHasSwapInterval = 0;
static INDEX sys_bHasHardwareTnL = 1;
static INDEX sys_bHasTruform = 0;
static INDEX sys_bHasCVAs = 0;
static INDEX sys_bUsingOpenGL = 0;
extern INDEX sys_bUsingDirect3D = 0;

/*
 * Low level hook flags
 */
#define WH_KEYBOARD_LL 13

#pragma message(">> doublecheck me!!!")
// these are commented because they are already defined in winuser.h
//#define LLKHF_EXTENDED 0x00000001
//#define LLKHF_INJECTED 0x00000010
//#define LLKHF_ALTDOWN  0x00000020
//#define LLKHF_UP       0x00000080

//#define LLMHF_INJECTED 0x00000001

/*
 * Structure used by WH_KEYBOARD_LL
 */
// this is commented because there's a variant for this struct in winuser.h
/*typedef struct tagKBDLLHOOKSTRUCT {
    DWORD   vkCode;
    DWORD   scanCode;
    DWORD   flags;
    DWORD   time;
    DWORD   dwExtraInfo;
} KBDLLHOOKSTRUCT, FAR *LPKBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;*/

static HHOOK _hLLKeyHook = NULL;

#if PLATFORM_WIN32
LRESULT CALLBACK LowLevelKeyboardProc (INT nCode, WPARAM wParam, LPARAM lParam)
{
  // By returning a non-zero value from the hook procedure, the
  // message does not get passed to the target window
  KBDLLHOOKSTRUCT *pkbhs = (KBDLLHOOKSTRUCT *) lParam;
  BOOL bControlKeyDown = 0;

  switch (nCode)
  {
      case HC_ACTION:
      {
          // Check to see if the CTRL key is pressed
          bControlKeyDown = GetAsyncKeyState (VK_CONTROL) >> ((sizeof(SHORT) * 8) - 1);
          
          // Disable CTRL+ESC
          if (pkbhs->vkCode == VK_ESCAPE && bControlKeyDown)
              return 1;

          // Disable ALT+TAB
          if (pkbhs->vkCode == VK_TAB && pkbhs->flags & LLKHF_ALTDOWN)
              return 1;

          // Disable ALT+ESC
          if (pkbhs->vkCode == VK_ESCAPE && pkbhs->flags & LLKHF_ALTDOWN)
              return 1;

          break;
      }

      default:
          break;
  }
  return CallNextHookEx (_hLLKeyHook, nCode, wParam, lParam);
}
#endif

#if PLATFORM_WIN32
void DisableWindowsKeys(void)
{
  //if( _hLLKeyHook!=NULL) UnhookWindowsHookEx(_hLLKeyHook);
  //_hLLKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL, &LowLevelKeyboardProc, NULL, GetCurrentThreadId());

  INDEX iDummy;
  SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, TRUE, &iDummy, 0);
}
#else
void DisableWindowsKeys(void) {}
#endif

#if PLATFORM_WIN32
void EnableWindowsKeys(void)
{
  INDEX iDummy;
  SystemParametersInfo(SPI_SETSCREENSAVERRUNNING, FALSE, &iDummy, 0);
  // if( _hLLKeyHook!=NULL) UnhookWindowsHookEx(_hLLKeyHook);
}
#else
void EnableWindowsKeys(void) {}
#endif

// texture size reporting

static CTString ReportQuality( INDEX iQuality)
{
  if( iQuality==0) return "optimal";
  if( iQuality==1) return "16-bit";
  if( iQuality==2) return "32-bit";
  if( iQuality==3) return "compressed";
  ASSERTALWAYS( "Invalid texture quality.");
  return "?";
}


static void TexturesInfo(void)
{
  UpdateTextureSettings();
  INDEX ctNo04O=0, ctNo64O=0, ctNoMXO=0;
  PIX   pixK04O=0, pixK64O=0, pixKMXO=0;
  SLONG slKB04O=0, slKB64O=0, slKBMXO=0;
  INDEX ctNo04A=0, ctNo64A=0, ctNoMXA=0;
  PIX   pixK04A=0, pixK64A=0, pixKMXA=0;
  SLONG slKB04A=0, slKB64A=0, slKBMXA=0;

  // walk thru all textures on stock
  {FOREACHINDYNAMICCONTAINER( _pTextureStock->st_ctObjects, CTextureData, ittd)
  { // get texture info
    CTextureData &td = *ittd;
    BOOL  bAlpha   = td.td_ulFlags&TEX_ALPHACHANNEL;
    INDEX ctFrames = td.td_ctFrames;
    SLONG slBytes  = td.GetUsedMemory();
    ASSERT( slBytes>=0);
    // get texture size
    PIX pixTextureSize = td.GetPixWidth() * td.GetPixHeight();
    PIX pixMipmapSize  = pixTextureSize;
    if( !gap_bAllowSingleMipmap || td.td_ctFineMipLevels>1) pixMipmapSize = pixMipmapSize *4/3;
    // increase corresponding counters
    if( pixTextureSize<4096) {
      if( bAlpha) { pixK04A+=pixMipmapSize;  slKB04A+=slBytes;  ctNo04A+=ctFrames; }
      else        { pixK04O+=pixMipmapSize;  slKB04O+=slBytes;  ctNo04O+=ctFrames; }
    } else if( pixTextureSize<=65536) {
      if( bAlpha) { pixK64A+=pixMipmapSize;  slKB64A+=slBytes;  ctNo64A+=ctFrames; }
      else        { pixK64O+=pixMipmapSize;  slKB64O+=slBytes;  ctNo64O+=ctFrames; }
    } else {
      if( bAlpha) { pixKMXA+=pixMipmapSize;  slKBMXA+=slBytes;  ctNoMXA+=ctFrames; }
      else        { pixKMXO+=pixMipmapSize;  slKBMXO+=slBytes;  ctNoMXO+=ctFrames; }
    }
  }}

  // report
  const PIX pixNormDim = sqrt((FLOAT)TS.ts_pixNormSize);
  const PIX pixAnimDim = sqrt((FLOAT)TS.ts_pixAnimSize);
  const PIX pixEffDim  = 1L<<tex_iEffectSize;
  CTString strTmp;
  strTmp = tex_bFineEffect ? "32-bit" : "16-bit";
  CPrintF( "\n");
  CPrintF( "Normal-opaque textures quality:         %s\n", ReportQuality(TS.ts_iNormQualityO));
  CPrintF( "Normal-translucent textures quality:    %s\n", ReportQuality(TS.ts_iNormQualityA));
  CPrintF( "Animation-opaque textures quality:      %s\n", ReportQuality(TS.ts_iAnimQualityO));
  CPrintF( "Animation-translucent textures quality: %s\n", ReportQuality(TS.ts_iAnimQualityA));
  CPrintF( "Effect textures quality:                %s\n", strTmp);
  CPrintF( "\n");
  CPrintF( "Max allowed normal texture area size:    %3dx%d\n", pixNormDim, pixNormDim);
  CPrintF( "Max allowed animation texture area size: %3dx%d\n", pixAnimDim, pixAnimDim);
  CPrintF( "Max allowed effect texture area size:    %3dx%d\n", pixEffDim,  pixEffDim);

  CPrintF( "\n");
  CPrintF( "Opaque textures memory usage:\n");
  CPrintF( "     <64 pix: %3d frames use %6.1f Kpix in %5d KB\n", ctNo04O, pixK04O/1024.0f, slKB04O/1024);
  CPrintF( "  64-256 pix: %3d frames use %6.1f Kpix in %5d KB\n", ctNo64O, pixK64O/1024.0f, slKB64O/1024);
  CPrintF( "    >256 pix: %3d frames use %6.1f Kpix in %5d KB\n", ctNoMXO, pixKMXO/1024.0f, slKBMXO/1024);
  CPrintF( "Translucent textures memory usage:\n");
  CPrintF( "     <64 pix: %3d frames use %6.1f Kpix in %5d KB\n", ctNo04A, pixK04A/1024.0f, slKB04A/1024);
  CPrintF( "  64-256 pix: %3d frames use %6.1f Kpix in %5d KB\n", ctNo64A, pixK64A/1024.0f, slKB64A/1024);
  CPrintF( "    >256 pix: %3d frames use %6.1f Kpix in %5d KB\n", ctNoMXA, pixKMXA/1024.0f, slKBMXA/1024);
  CPrintF("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n\n");
}



// reformat an extensions string to cross multiple lines
extern CTString ReformatExtensionsString( CTString strUnformatted)
{
  CTString strTmp, strDst = "\n";
  char *pcSrc = (char*)(const char*)strUnformatted;
  FOREVER {
    char *pcSpace = strchr( pcSrc, ' ');
    if( pcSpace==NULL) break;
    *pcSpace = 0;
    strTmp.PrintF( "    %s\n", pcSrc);
    strDst += strTmp;
    *pcSpace = ' ';
    pcSrc = pcSpace +1;
  }
  if(strDst=="\n") {
    strDst = "none\n";
  }
  // done
  return strDst;
}


// printout extensive OpenGL/Direct3D info to console
static void GAPInfo(void)
{
  // check API
  const GfxAPIType eAPI = _pGfx->gl_eCurrentAPI;
#ifdef SE1_D3D
  ASSERT( eAPI==GAT_OGL || eAPI==GAT_D3D || eAPI==GAT_NONE);
#else // SE1_D3D
  ASSERT( eAPI==GAT_OGL || eAPI==GAT_NONE);
#endif // SE1_D3D
  CPrintF( "\n");

  // in case of driver hasn't been initialized yet
  if( (_pGfx->go_hglRC==NULL 
#ifdef SE1_D3D
    && _pGfx->gl_pd3dDevice==NULL
#endif // SE1_D3D
    ) || eAPI==GAT_NONE) {
    // be brief, be quick
    CPrintF( TRANS("Display driver hasn't been initialized.\n\n"));
    return;
  }

  // report API
  CPrintF( "- Graphics API: ");
  if( eAPI==GAT_OGL) CPrintF( "OpenGL\n");
  else CPrintF( "Direct3D\n");
  // and number of adapters
  CPrintF( "- Adapters found: %d\n", _pGfx->gl_gaAPI[eAPI].ga_ctAdapters);
  CPrintF( "\n");

  // report renderer
  CDisplayAdapter &da = _pGfx->gl_gaAPI[eAPI].ga_adaAdapter[_pGfx->gl_iCurrentAdapter];
  if( eAPI==GAT_OGL) CPrintF( "- Vendor:   %s\n", da.da_strVendor);
  CPrintF( "- Renderer: %s\n", da.da_strRenderer);
  CPrintF( "- Version:  %s\n", da.da_strVersion);
  CPrintF( "\n");

  // Z-buffer depth
  CPrintF( "- Z-buffer precision: ");
  if( _pGfx->gl_iCurrentDepth==0) CPrintF( "default\n");
  else CPrintF( "%d bits\n", _pGfx->gl_iCurrentDepth);

  // 32-bit textures
  CPrintF( "- 32-bit textures: ");
  if( _pGfx->gl_ulFlags & GLF_32BITTEXTURES) CPrintF( "supported\n");
  else CPrintF( "not supported\n");
  // grayscale textures
  CPrintF( "- Grayscale textures: ");
  if( gap_bAllowGrayTextures) CPrintF( "allowed\n");
  else CPrintF( "not allowed\n");
  // report maximum texture dimension
  CPrintF( "- Max texture dimension: %d pixels\n", _pGfx->gl_pixMaxTextureDimension);

  // report multitexturing capabilities
  CPrintF( "- Multi-texturing: ");
  if( _pGfx->gl_ctRealTextureUnits<2) CPrintF( "not supported\n");
  else {
    if( gap_iUseTextureUnits>1) CPrintF( "enabled (using %d texture units)\n", gap_iUseTextureUnits);
    else CPrintF( "disabled\n");
    CPrintF( "- Texture units: %d", _pGfx->gl_ctRealTextureUnits);
    if( _pGfx->gl_ctTextureUnits < _pGfx->gl_ctRealTextureUnits)  {
      CPrintF(" (%d can be used)\n", _pGfx->gl_ctTextureUnits); 
    } else CPrintF("\n"); 
  }

  // report texture anisotropy degree
  if( _pGfx->gl_iMaxTextureAnisotropy>=2) {
    CPrintF( "- Texture anisotropy: %d of %d\n", _tpGlobal[0].tp_iAnisotropy, _pGfx->gl_iMaxTextureAnisotropy);
  } else CPrintF( "- Anisotropic texture filtering: not supported\n");

  // report texture LOD bias range
  const FLOAT fMaxLODBias = _pGfx->gl_fMaxTextureLODBias;
  if( fMaxLODBias>0) {
    CPrintF( "- Texture LOD bias: %.1f of +/-%.1f\n", _pGfx->gl_fTextureLODBias, fMaxLODBias);
  } else CPrintF( "- Texture LOD biasing: not supported\n");

  // OpenGL only stuff ...
  if( eAPI==GAT_OGL) 
  {
    // report truform tessellation
    CPrintF( "- Truform tessellation: ");
    if( _pGfx->gl_iMaxTessellationLevel>0) {
      if( _pGfx->gl_iTessellationLevel>0) {
        CPrintF( "enabled ");
        if( gap_bForceTruform) CPrintF( "(for all models)\n");
        else CPrintF( "(only for Truform-ready models)\n");
        CTString strNormalMode = ogl_bTruformLinearNormals ? "linear" : "quadratic";
        CPrintF( "- Tesselation level: %d of %d (%s normals)\n", _pGfx->gl_iTessellationLevel, _pGfx->gl_iMaxTessellationLevel, strNormalMode);
      } else CPrintF( "disabled\n");
    } else CPrintF( "not supported\n");

    // report current swap interval (only if fullscreen)
    if( _pGfx->gl_ulFlags&GLF_FULLSCREEN) {
      // report current swap interval
      CPrintF( "- Swap interval: ");
      if( _pGfx->gl_ulFlags&GLF_VSYNC) {
        GLint gliWaits = pwglGetSwapIntervalEXT();
        if( gliWaits>=0) {
          ASSERT( gliWaits==_pGfx->gl_iSwapInterval);
          CPrintF( "%d frame(s)\n", gliWaits);
        } else CPrintF( "not readable\n");
      } else CPrintF( "not adjustable\n");
    }
    // report T-Buffer support
    if( _pGfx->gl_ulFlags & GLF_EXT_TBUFFER) {
      CPrintF( "- T-Buffer effect: ");
      if( _pGfx->go_ctSampleBuffers==0) CPrintF( "disabled\n");
      else {
        ogl_iTBufferEffect = Clamp( ogl_iTBufferEffect, 0L, 2L);
        CTString strEffect = "Partial anti-aliasing";
        if( ogl_iTBufferEffect<1) strEffect = "none";
        if( ogl_iTBufferEffect>1) strEffect = "Motion blur";
        CPrintF( "%s (%d buffers used)\n", strEffect, _pGfx->go_ctSampleBuffers);
      }
    }

    // compiled vertex arrays support
    CPrintF( "- Compiled Vertex Arrays: ");
    if( _pGfx->gl_ulFlags & GLF_EXT_COMPILEDVERTEXARRAY) {
      extern BOOL CVA_b2D;
      extern BOOL CVA_bWorld;
      extern BOOL CVA_bModels;    
      if( ogl_bUseCompiledVertexArrays) {
        CTString strSep="";
        CPrintF( "enabled (for ");
        if( CVA_bWorld)  { CPrintF( "world");               strSep="/"; }
        if( CVA_bModels) { CPrintF( "%smodels",    strSep); strSep="/"; }
        if( CVA_b2D)     { CPrintF( "%sparticles", strSep); }
        CPrintF( ")\n");
      } else CPrintF( "disabled\n");
    } else CPrintF( "not supported\n");

    // report texture compression type
    CPrintF( "- Supported texture compression system(s): ");
    if( !(_pGfx->gl_ulFlags&GLF_TEXTURECOMPRESSION)) CPrintF( "none\n");
    else {
      CTString strSep="";
      if( _pGfx->gl_ulFlags & GLF_EXTC_ARB)    { CPrintF( "ARB");                strSep=", "; }
      if( _pGfx->gl_ulFlags & GLF_EXTC_S3TC)   { CPrintF( "%sS3TC",     strSep); strSep=", "; }
      if( _pGfx->gl_ulFlags & GLF_EXTC_FXT1)   { CPrintF( "%sFTX1",     strSep); strSep=", "; }
      if( _pGfx->gl_ulFlags & GLF_EXTC_LEGACY) { CPrintF( "%sold S3TC", strSep); }
      CPrintF( "\n- Current texture compression system: ");
      switch( ogl_iTextureCompressionType) {
      case 0:   CPrintF( "none\n");         break;
      case 1:   CPrintF( "ARB wrapper\n");  break;
      case 2:   CPrintF( "S3TC\n");         break;
      case 3:   CPrintF( "FXT1\n");         break;
      default:  CPrintF( "old S3TC\n");     break;
      }
    }
    /* if exist, report vertex array range extension usage
    if( ulExt & GOEXT_VERTEXARRAYRANGE) {
      extern BOOL  VB_bSetupFailed;
      extern SLONG VB_slVertexBufferSize;
      extern INDEX VB_iVertexBufferType;
      extern INDEX ogl_iVertexBuffers;
      if( VB_bSetupFailed) { // didn't manage to setup vertex buffers
        CPrintF( "- Enhanced HW T&L: fail\n");
      } else if( VB_iVertexBufferType==0) {  // not used
        CPrintF( "- Enhanced HW T&L: disabled\n");
      } else {  // works! :)
        CTString strBufferType("AGP");
        if( VB_iVertexBufferType==2) strBufferType = "video";
        const SLONG slMemSize = VB_slVertexBufferSize/1024;
        CPrintF( "- Enhanced hardware T&L: %d buffers in %d KB of %s memory",
                 ogl_iVertexBuffers, slMemSize, strBufferType);
      }
    } */
    // report OpenGL externsions
    CPrintF("\n");
    CPrintF("- Published extensions: %s", ReformatExtensionsString(_pGfx->go_strExtensions));
    if( _pGfx->go_strWinExtensions != "") CPrintF("%s", ReformatExtensionsString(_pGfx->go_strWinExtensions));
    CPrintF("\n- Supported extensions: %s\n", ReformatExtensionsString(_pGfx->go_strSupportedExtensions));
  }

  // Direct3D only stuff
#ifdef SE1_D3D
  if( eAPI==GAT_D3D)
  {
    // HW T&L
    CPrintF( "- Hardware T&L: ");
    if( _pGfx->gl_ulFlags&GLF_D3D_HASHWTNL) {
      if( _pGfx->gl_ctMaxStreams<GFX_MINSTREAMS) CPrintF( "cannot be used\n");
      else if( _pGfx->gl_ulFlags&GLF_D3D_USINGHWTNL) CPrintF( "enabled (%d streams)\n", _pGfx->gl_ctMaxStreams);
      else CPrintF( "disabled\n");
    } else CPrintF( "not present\n");

    // report vtx/idx buffers size
    extern SLONG SizeFromVertices_D3D( INDEX ctVertices);
    const SLONG slMemoryUsed = SizeFromVertices_D3D(_pGfx->gl_ctVertices);
    CPrintF( "- Vertex buffer size: %.1f KB (%d vertices)\n", slMemoryUsed/1024.0f, _pGfx->gl_ctVertices);

    // N-Patches tessellation (Truform)
    CPrintF( "- N-Patches: ");
    if( _pGfx->gl_iMaxTessellationLevel>0) {
      if( !(_pGfx->gl_ulFlags&GLF_D3D_USINGHWTNL)) CPrintF( "not possible with SW T&L\n");
      else if( _pGfx->gl_iTessellationLevel>0) {
        CPrintF( "enabled ");
        if( gap_bForceTruform) CPrintF( "(for all models)\n");
        else CPrintF( "(only for Truform-ready models)\n");
        CPrintF( "- Tesselation level: %d of %d\n", _pGfx->gl_iTessellationLevel, _pGfx->gl_iMaxTessellationLevel);
      } else CPrintF( "disabled\n");
    } else CPrintF( "not supported\n");

    // texture compression
    CPrintF( "- Texture compression: ");
    if( _pGfx->gl_ulFlags&GLF_TEXTURECOMPRESSION) CPrintF( "supported\n");
    else CPrintF( "not supported\n");

    // custom clip plane support
    CPrintF( "- Custom clip plane: ");
    if( _pGfx->gl_ulFlags&GLF_D3D_CLIPPLANE) CPrintF( "supported\n");
    else CPrintF( "not supported\n");

    // color buffer writes enable/disable support
    CPrintF( "- Color masking: ");
    if( _pGfx->gl_ulFlags&GLF_D3D_COLORWRITES) CPrintF( "supported\n");
    else CPrintF( "not supported\n");

    // depth (Z) bias support
    CPrintF( "- Depth biasing: ");
    if( _pGfx->gl_ulFlags&GLF_D3D_ZBIAS) CPrintF( "supported\n");
    else CPrintF( "not supported\n");

    // current swap interval (only if fullscreen)
    if( _pGfx->gl_ulFlags&GLF_FULLSCREEN) {
      CPrintF( "- Swap interval: ");
      if( _pGfx->gl_ulFlags&GLF_VSYNC) {
        CPrintF( "%d frame(s)\n", _pGfx->gl_iSwapInterval);
      } else CPrintF( "not adjustable\n");
    }
  }
#endif // SE1_D3D
}


// update console system vars
extern void UpdateGfxSysCVars(void)
{
  sys_bHasTextureCompression = 0;
  sys_bHasTextureAnisotropy = 0;
  sys_bHasAdjustableGamma = 0;
  sys_bHasTextureLODBias = 0;
  sys_bHasMultitexturing = 0;
  sys_bHas32bitTextures = 0;
  sys_bHasSwapInterval = 0;
  sys_bHasHardwareTnL = 1;
  sys_bHasTruform = 0;
  sys_bHasCVAs = 1;
  sys_bUsingOpenGL = 0;
  sys_bUsingDirect3D = 0;
  if( _pGfx->gl_iMaxTextureAnisotropy>1) sys_bHasTextureAnisotropy = 1;
  if( _pGfx->gl_fMaxTextureLODBias>0) sys_bHasTextureLODBias = 1;
  if( _pGfx->gl_ctTextureUnits>1) sys_bHasMultitexturing = 1;
  if( _pGfx->gl_iMaxTessellationLevel>0) sys_bHasTruform = 1;
  if( _pGfx->gl_ulFlags & GLF_TEXTURECOMPRESSION) sys_bHasTextureCompression = 1;
  if( _pGfx->gl_ulFlags & GLF_ADJUSTABLEGAMMA) sys_bHasAdjustableGamma = 1;
  if( _pGfx->gl_ulFlags & GLF_32BITTEXTURES) sys_bHas32bitTextures = 1;
  if( _pGfx->gl_ulFlags & GLF_VSYNC) sys_bHasSwapInterval = 1;
  if( _pGfx->gl_eCurrentAPI==GAT_OGL && !(_pGfx->gl_ulFlags&GLF_EXT_COMPILEDVERTEXARRAY)) sys_bHasCVAs = 0;
#ifdef SE1_D3D
  if( _pGfx->gl_eCurrentAPI==GAT_D3D && !(_pGfx->gl_ulFlags&GLF_D3D_HASHWTNL)) sys_bHasHardwareTnL = 0;
#endif // SE1_D3D
  if( _pGfx->gl_eCurrentAPI==GAT_OGL) sys_bUsingOpenGL = 1;
#ifdef SE1_D3D
  if( _pGfx->gl_eCurrentAPI==GAT_D3D) sys_bUsingDirect3D = 1;
#endif // SE1_D3D
}

   
   
// determine whether texture or shadowmap needs probing
extern BOOL ProbeMode( CTimerValue tvLast)
{
  // probing off ?
  if( !_pGfx->gl_bAllowProbing) return FALSE;
  if( gfx_tmProbeDecay<1) {
    gfx_tmProbeDecay = 0;  
    return FALSE;
  }
  // clamp and determine probe mode
  if( gfx_tmProbeDecay>999) gfx_tmProbeDecay = 999;
  CTimerValue tvNow  = _pTimer->GetHighPrecisionTimer();
  const TIME tmDelta = (tvNow-tvLast).GetSeconds();
  if( tmDelta>gfx_tmProbeDecay) return TRUE;
  return FALSE;
}
   


// uncache all cached shadow maps
extern void UncacheShadows(void)
{
  // mute all sounds
  _pSound->Mute();
  // prepare new saturation factors for shadowmaps
  gfx_fSaturation  = ClampDn( gfx_fSaturation, 0.0f); 
  shd_fSaturation  = ClampDn( shd_fSaturation, 0.0f); 
  gfx_iHueShift    = Clamp(   gfx_iHueShift, 0L, 359L); 
  shd_iHueShift    = Clamp(   shd_iHueShift, 0L, 359L); 
  _slShdSaturation = (SLONG)( gfx_fSaturation*shd_fSaturation*256.0f);
  _slShdHueShift   = Clamp(  (gfx_iHueShift+shd_iHueShift)*255L/359L, 0L, 255L);
          
  CListHead &lhOriginal = _pGfx->gl_lhCachedShadows;
  // while there is some shadow in main list
  while( !lhOriginal.IsEmpty()) {
    CShadowMap &sm = *LIST_HEAD( lhOriginal, CShadowMap, sm_lnInGfx);
    sm.Uncache();
  }
  // mark that we need pretouching
  _bNeedPretouch = TRUE;
}


// refresh (uncache and eventually cache) all cached shadow maps
extern void CacheShadows(void);
static void RecacheShadows(void)
{
  // mute all sounds
  _pSound->Mute();
  UncacheShadows();
  if( shd_bCacheAll) CacheShadows();
  else CPrintF( TRANS("All shadows uncached.\n"));
}



// reload all textures that were loaded
extern void ReloadTextures(void)
{
  // mute all sounds
  _pSound->Mute();

  // prepare new saturation factors for textures
  gfx_fSaturation  = ClampDn( gfx_fSaturation, 0.0f); 
  tex_fSaturation  = ClampDn( tex_fSaturation, 0.0f); 
  gfx_iHueShift    = Clamp( gfx_iHueShift, 0L, 359L); 
  tex_iHueShift    = Clamp( tex_iHueShift, 0L, 359L); 
  _slTexSaturation = (SLONG)( gfx_fSaturation*tex_fSaturation*256.0f);
  _slTexHueShift   = Clamp(  (gfx_iHueShift+tex_iHueShift)*255L/359L, 0L, 255L);

  // update texture settings
  UpdateTextureSettings();
  // loop thru texture stock
  {FOREACHINDYNAMICCONTAINER( _pTextureStock->st_ctObjects, CTextureData, ittd) {
    CTextureData &td = *ittd;
    td.Reload();
    td.td_tpLocal.Clear();
  }}

  // reset fog/haze texture
  _fog_pixSizeH = 0;
  _fog_pixSizeL = 0;
  _haze_pixSize = 0;

  // reinit flat texture
  ASSERT( _ptdFlat!=NULL);
  _ptdFlat->td_tpLocal.Clear();
  _ptdFlat->Unbind();
  _ptdFlat->td_ulFlags = TEX_ALPHACHANNEL | TEX_32BIT | TEX_STATIC;
  _ptdFlat->td_mexWidth  = 1;
  _ptdFlat->td_mexHeight = 1;
  _ptdFlat->td_iFirstMipLevel  = 0;
  _ptdFlat->td_ctFineMipLevels = 1;
  _ptdFlat->td_slFrameSize = 1*1* BYTES_PER_TEXEL;    
  _ptdFlat->td_ctFrames = 1;
  _ptdFlat->td_ulInternalFormat = TS.ts_tfRGBA8;
  _ptdFlat->td_pulFrames = &_ulWhite;
  _ptdFlat->SetAsCurrent();

  /*
  // reset are renderable textures, too
  CListHead &lhOriginal = _pGfx->gl_lhRenderTextures;
  while( !lhOriginal.IsEmpty()) {
    CRenderTexture &rt = *LIST_HEAD( lhOriginal, CRenderTexture, rt_lnInGfx);
    rt.Reset();
  }
  */

  // mark that we need pretouching
  _bNeedPretouch = TRUE;
  CPrintF( TRANS("All textures reloaded.\n"));
}


// refreshes all textures and shadow maps
static void RefreshTextures(void)
{
  // refresh
  ReloadTextures();
  RecacheShadows();
}



// reload all models that were loaded

static void ReloadModels(void)
{
  // mute all sounds
  _pSound->Mute();
  // loop thru model stock
  {FOREACHINDYNAMICCONTAINER( _pModelStock->st_ctObjects, CModelData, itmd) {
    CModelData &md = *itmd;
    md.Reload();
  }}
  // mark that we need pretouching
  _bNeedPretouch = TRUE;
  // all done
  CPrintF( TRANS("All models reloaded.\n"));
}


// variable change post functions
static BOOL _bLastModelQuality = -1;
static void MdlPostFunc(void *pvVar)
{
  mdl_bFineQuality = Clamp( mdl_bFineQuality, 0L, 1L);
  if( _bLastModelQuality!=mdl_bFineQuality) {
    _bLastModelQuality = mdl_bFineQuality;
    ReloadModels();
  }
}


/*
 *  GfxLibrary functions
 */

static void PrepareTables(void)
{
  INDEX i;
  // prepare array for fast clamping to 0..255
  for( i=-256*2; i<256*4; i++) aubClipByte[i+256*2] = (UBYTE)Clamp( i, 0L, 255L);
  // prepare fast sqrt tables
  for( i=0; i<SQRTTABLESIZE; i++) aubSqrt[i]   = (UBYTE)(sqrt((FLOAT)(i*65536/SQRTTABLESIZE)));
  for( i=1; i<SQRTTABLESIZE; i++) auw1oSqrt[i] = (UWORD)(sqrt((FLOAT)(SQRTTABLESIZE-1)/i)*255.0f);
  auw1oSqrt[0] = MAX_UWORD;
  // prepare fast sin/cos table
  for( i=-256; i<256+64; i++) afSinTable[i+256] = Sin((i-128)/256.0f*360.0f);
  // prepare gouraud conversion table
  for( INDEX h=0; h<128; h++) {
    for( INDEX p=0; p<128; p++) {
      const FLOAT fSinH = pfSinTable[h*2];
      const FLOAT fSinP = pfSinTable[p*2];
      const FLOAT fCosH = pfCosTable[h*2];
      const FLOAT fCosP = pfCosTable[p*2];
      const FLOAT3D v( -fSinH*fCosP, +fSinP, -fCosH*fCosP);
      aubGouraudConv[h*128+p] = (UBYTE)GouraudNormal(v);
    }
  }
}


/*
 * Construct uninitialized gfx library.
 */
CGfxLibrary::CGfxLibrary(void)
{
  // reset some variables to default
  gl_iFrameNumber = 0;
  gl_slAllowedUploadBurst = 0;
  gl_bAllowProbing = FALSE;

  gl_iSwapInterval = 1234;
  gl_pixMaxTextureDimension = 8192;
  gl_ctTextureUnits = 0;
  gl_ctRealTextureUnits = 0;
  gl_fTextureLODBias = 0;
  gl_fMaxTextureLODBias = 0;   
  gl_iMaxTextureAnisotropy = 0;
  gl_iMaxTessellationLevel = 0;
  gl_iTessellationLevel = 0;
  gl_ulFlags = NONE;
  
  // create some internal tables
  PrepareTables();

  // no driver loaded
  gl_eCurrentAPI = GAT_NONE;
  gl_hiDriver = NONE;
  go_hglRC = NONE;
  gl_ctDriverChanges = 0;

  // DX8 not loaded either
#ifdef SE1_D3D
  gl_pD3D = NONE;
  gl_pd3dDevice = NULL;
  gl_d3dColorFormat = (D3DFORMAT)NONE;
  gl_d3dDepthFormat = (D3DFORMAT)NONE;
#endif // SE1_D3D
  gl_pvpActive = NULL;
  gl_ctMaxStreams = 0;
  gl_dwVertexShader = 0;
#ifdef SE1_D3D
  gl_pd3dIdx = NULL;
  gl_pd3dVtx = NULL;
  gl_pd3dNor = NULL;
  for( INDEX i=0; i<GFX_MAXLAYERS; i++) gl_pd3dCol[i] = gl_pd3dTex[i] = NULL;
#endif // SE1_D3D
  gl_ctVertices = 0;
  gl_ctIndices  = 0;

  // reset profiling counters
  gl_ctWorldTriangles    = 0;
  gl_ctModelTriangles    = 0;
  gl_ctParticleTriangles = 0;
  gl_ctTotalTriangles    = 0;

  // init flat texture
  _ptdFlat = new CTextureData;
  _ptdFlat->td_ulFlags = TEX_ALPHACHANNEL | TEX_32BIT | TEX_STATIC;

  // prepare some quad elements
  extern void AddQuadElements( const INDEX ctQuads);
  AddQuadElements(1024); // should be enough (at least for a start)

  // reset GFX API function pointers
  GFX_SetFunctionPointers( (INDEX)GAT_NONE);
}


/*
 * Destruct (and clean up).
 */
CGfxLibrary::~CGfxLibrary()
{
  extern void EnableWindowsKeys(void);
  EnableWindowsKeys();
  // free common arrays
  _avtxCommon.Clear();
  _atexCommon.Clear();
  _acolCommon.Clear();
  _aiCommonElements.Clear();
  _aiCommonQuads.Clear();
  // stop current display mode
  StopDisplayMode();
  // safe release of flat texture
  ASSERT( _ptdFlat!=NULL);
  _ptdFlat->td_pulFrames = NULL;
  delete _ptdFlat;
  _ptdFlat = NULL;
}



#define SM_CXVIRTUALSCREEN  78 
#define SM_CYVIRTUALSCREEN  79 
#define SM_CMONITORS        80 



/* Initialize library for application main window. */
void CGfxLibrary::Init(void)
{
  ASSERT( this!=NULL);

#if PLATFORM_WIN32
  // report desktop settings
  CPrintF(TRANS("Desktop settings...\n"));
  HDC hdc = GetDC(NULL);
  SLONG slBPP = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL); 
  ReleaseDC(NULL, hdc);  

  gfx_ctMonitors = GetSystemMetrics(SM_CMONITORS);

  CPrintF(TRANS("  Color Depth: %dbit\n"), slBPP);
  CPrintF(TRANS("  Screen: %dx%d\n"), GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
  CPrintF(TRANS("  Virtual screen: %dx%d\n"), GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));
  CPrintF(TRANS("  Monitors directly reported: %d\n"), gfx_ctMonitors);

  CPrintF("\n");
#endif

  gfx_bMultiMonDisabled = FALSE;
 
  _pfGfxProfile.Reset();

  // we will never allow glide splash screen
  putenv( "FX_GLIDE_NO_SPLASH=1");

  // declare some console vars
  _pShell->DeclareSymbol("user void MonitorsOn(void);",  (void*) &MonitorsOn);
  _pShell->DeclareSymbol("user void MonitorsOff(void);", (void*) &MonitorsOff);

  _pShell->DeclareSymbol("user void GAPInfo(void);", (void*)      &GAPInfo);
  _pShell->DeclareSymbol("user void TexturesInfo(void);", (void*) &TexturesInfo);
  _pShell->DeclareSymbol("user void UncacheShadows(void);", (void*)  &UncacheShadows);
  _pShell->DeclareSymbol("user void RecacheShadows(void);", (void*)  &RecacheShadows);
  _pShell->DeclareSymbol("user void RefreshTextures(void);", (void*) &RefreshTextures);
  _pShell->DeclareSymbol("user void ReloadModels(void);", (void*)    &ReloadModels);

  _pShell->DeclareSymbol("persistent user INDEX ogl_bUseCompiledVertexArrays;", &ogl_bUseCompiledVertexArrays);
  _pShell->DeclareSymbol("persistent user INDEX ogl_bExclusive;", &ogl_bExclusive);
  _pShell->DeclareSymbol("persistent user INDEX ogl_bAllowQuadArrays;",   &ogl_bAllowQuadArrays);
  _pShell->DeclareSymbol("persistent user INDEX ogl_iTextureCompressionType;", &ogl_iTextureCompressionType);
  _pShell->DeclareSymbol("persistent user INDEX ogl_iMaxBurstSize;",    &ogl_iMaxBurstSize);
  _pShell->DeclareSymbol("persistent user INDEX ogl_bGrabDepthBuffer;", &ogl_bGrabDepthBuffer);
  _pShell->DeclareSymbol("persistent user INDEX ogl_iFinish;", &ogl_iFinish);

  _pShell->DeclareSymbol("persistent user INDEX ogl_iTBufferEffect;",  &ogl_iTBufferEffect);
  _pShell->DeclareSymbol("persistent user INDEX ogl_iTBufferSamples;", &ogl_iTBufferSamples);
  _pShell->DeclareSymbol("persistent user INDEX ogl_bTruformLinearNormals;", &ogl_bTruformLinearNormals);
  _pShell->DeclareSymbol("persistent user INDEX ogl_bAlternateClipPlane;",   &ogl_bAlternateClipPlane);

  _pShell->DeclareSymbol("persistent user INDEX d3d_bUseHardwareTnL;", &d3d_bUseHardwareTnL);
  _pShell->DeclareSymbol("persistent user INDEX d3d_iMaxBurstSize;",   &d3d_iMaxBurstSize);
  _pShell->DeclareSymbol("persistent user INDEX d3d_iVertexBuffersSize;",   &d3d_iVertexBuffersSize);
  _pShell->DeclareSymbol("persistent user INDEX d3d_iVertexRangeTreshold;", &d3d_iVertexRangeTreshold);
  _pShell->DeclareSymbol("persistent user INDEX d3d_bAlternateDepthReads;", &d3d_bAlternateDepthReads);
  _pShell->DeclareSymbol("persistent      INDEX d3d_bFastUpload;", &d3d_bFastUpload);
  _pShell->DeclareSymbol("persistent user INDEX d3d_iFinish;", &d3d_iFinish);

  _pShell->DeclareSymbol("persistent user INDEX gap_iUseTextureUnits;",   &gap_iUseTextureUnits);
  _pShell->DeclareSymbol("persistent user INDEX gap_iTextureFiltering;",  &gap_iTextureFiltering);
  _pShell->DeclareSymbol("persistent user INDEX gap_iTextureAnisotropy;", &gap_iTextureAnisotropy);
  _pShell->DeclareSymbol("persistent user FLOAT gap_fTextureLODBias;",    &gap_fTextureLODBias);
  _pShell->DeclareSymbol("persistent user INDEX gap_bAllowGrayTextures;", &gap_bAllowGrayTextures);
  _pShell->DeclareSymbol("persistent user INDEX gap_bAllowSingleMipmap;", &gap_bAllowSingleMipmap);
  _pShell->DeclareSymbol("persistent user INDEX gap_bOptimizeStateChanges;", &gap_bOptimizeStateChanges);
  _pShell->DeclareSymbol("persistent user INDEX gap_iOptimizeDepthReads;",   &gap_iOptimizeDepthReads);
  _pShell->DeclareSymbol("persistent user INDEX gap_iOptimizeClipping;",     &gap_iOptimizeClipping);
  _pShell->DeclareSymbol("persistent user INDEX gap_iSwapInterval;", &gap_iSwapInterval);
  _pShell->DeclareSymbol("persistent user INDEX gap_iRefreshRate;",  &gap_iRefreshRate);
  _pShell->DeclareSymbol("persistent user INDEX gap_iDithering;",    &gap_iDithering);
  _pShell->DeclareSymbol("persistent user INDEX gap_bForceTruform;", &gap_bForceTruform);
  _pShell->DeclareSymbol("persistent user INDEX gap_iTruformLevel;", &gap_iTruformLevel);
  _pShell->DeclareSymbol("persistent user INDEX gap_iDepthBits;",   &gap_iDepthBits);
  _pShell->DeclareSymbol("persistent user INDEX gap_iStencilBits;", &gap_iStencilBits);

  _pShell->DeclareSymbol("void MdlPostFunc(INDEX);", (void*) &MdlPostFunc);

  _pShell->DeclareSymbol("           user INDEX gfx_bRenderPredicted;", &gfx_bRenderPredicted);
  _pShell->DeclareSymbol("           user INDEX gfx_bRenderModels;",    &gfx_bRenderModels);
  _pShell->DeclareSymbol("           user INDEX mdl_bShowTriangles;", &mdl_bShowTriangles);
  _pShell->DeclareSymbol("           user INDEX mdl_bCreateStrips;",  &mdl_bCreateStrips);
  _pShell->DeclareSymbol("           user INDEX mdl_bShowStrips;",    &mdl_bShowStrips);
  _pShell->DeclareSymbol("persistent user FLOAT mdl_fLODMul;",       &mdl_fLODMul);
  _pShell->DeclareSymbol("persistent user FLOAT mdl_fLODAdd;",       &mdl_fLODAdd);
  _pShell->DeclareSymbol("persistent user INDEX mdl_iLODDisappear;", &mdl_iLODDisappear);
  _pShell->DeclareSymbol("persistent user INDEX mdl_bRenderDetail;",     &mdl_bRenderDetail);
  _pShell->DeclareSymbol("persistent user INDEX mdl_bRenderSpecular;",   &mdl_bRenderSpecular);
  _pShell->DeclareSymbol("persistent user INDEX mdl_bRenderReflection;", &mdl_bRenderReflection);
  _pShell->DeclareSymbol("persistent user INDEX mdl_bRenderBump;",       &mdl_bRenderBump);
  _pShell->DeclareSymbol("persistent user INDEX mdl_bAllowOverbright;",  &mdl_bAllowOverbright);
  _pShell->DeclareSymbol("persistent user INDEX mdl_bFineQuality post:MdlPostFunc;", &mdl_bFineQuality);
  _pShell->DeclareSymbol("persistent user INDEX mdl_iShadowQuality;",  &mdl_iShadowQuality);
  _pShell->DeclareSymbol("                INDEX mdl_bTruformWeapons;", &mdl_bTruformWeapons);
  
  _pShell->DeclareSymbol("           user INDEX ska_bShowSkeleton;",   &ska_bShowSkeleton);
  _pShell->DeclareSymbol("           user INDEX ska_bShowColision;",   &ska_bShowColision);
  _pShell->DeclareSymbol("persistent user FLOAT ska_fLODMul;",         &ska_fLODMul);
  _pShell->DeclareSymbol("persistent user FLOAT ska_fLODAdd;",         &ska_fLODAdd);
  
  _pShell->DeclareSymbol("           user INDEX ter_bShowQuadTree;",   &ter_bShowQuadTree);
  _pShell->DeclareSymbol("           user INDEX ter_bShowWireframe;",  &ter_bShowWireframe);
  _pShell->DeclareSymbol("           user INDEX ter_bLerpVertices;",   &ter_bLerpVertices);
  _pShell->DeclareSymbol("           user INDEX ter_bShowInfo;",       &ter_bShowInfo);
  _pShell->DeclareSymbol("           user INDEX ter_bOptimizeRendering;", &ter_bOptimizeRendering);
  _pShell->DeclareSymbol("           user INDEX ter_bTempFreezeCast;   ", &ter_bTempFreezeCast);
  _pShell->DeclareSymbol("           user INDEX ter_bNoRegeneration;   ", &ter_bNoRegeneration);
  
  
  

  _pShell->DeclareSymbol("persistent user FLOAT gfx_tmProbeDecay;", &gfx_tmProbeDecay);
  _pShell->DeclareSymbol("persistent user INDEX gfx_iProbeSize;",   &gfx_iProbeSize);
  _pShell->DeclareSymbol("persistent user INDEX gfx_bClearScreen;", &gfx_bClearScreen);
  _pShell->DeclareSymbol("persistent user INDEX gfx_bDisableMultiMonSupport;", &gfx_bDisableMultiMonSupport);
  _pShell->DeclareSymbol("persistent user INDEX gfx_bDisableWindowsKeys;",     &gfx_bDisableWindowsKeys);
  _pShell->DeclareSymbol("persistent user INDEX gfx_bDecoratedText;",    &gfx_bDecoratedText);
  _pShell->DeclareSymbol("     const user INDEX gfx_ctMonitors;",        &gfx_ctMonitors);
  _pShell->DeclareSymbol("     const user INDEX gfx_bMultiMonDisabled;", &gfx_bMultiMonDisabled);

  _pShell->DeclareSymbol("persistent user INDEX tex_iNormalQuality;",    &tex_iNormalQuality);
  _pShell->DeclareSymbol("persistent user INDEX tex_iAnimationQuality;", &tex_iAnimationQuality);
  _pShell->DeclareSymbol("persistent user INDEX tex_bFineEffect;",       &tex_bFineEffect);
  _pShell->DeclareSymbol("persistent user INDEX tex_bFineFog;",          &tex_bFineFog);
  _pShell->DeclareSymbol("persistent user INDEX tex_iNormalSize;",    &tex_iNormalSize);
  _pShell->DeclareSymbol("persistent user INDEX tex_iAnimationSize;", &tex_iAnimationSize);
  _pShell->DeclareSymbol("persistent user INDEX tex_iEffectSize;",    &tex_iEffectSize);
  _pShell->DeclareSymbol("persistent user INDEX tex_iFogSize;",       &tex_iFogSize);
  _pShell->DeclareSymbol("persistent user INDEX tex_bCompressAlphaChannel;", &tex_bCompressAlphaChannel);
  _pShell->DeclareSymbol("persistent user INDEX tex_bAlternateCompression;", &tex_bAlternateCompression);
  _pShell->DeclareSymbol("persistent user INDEX tex_bDynamicMipmaps;", &tex_bDynamicMipmaps);
  _pShell->DeclareSymbol("persistent user INDEX tex_iDithering;",  &tex_iDithering);
/*  _pShell->DeclareSymbol("persistent user INDEX tex_iFiltering;",  &tex_iFiltering);
	TEMPORARY FIX ARTEFACTS IF USING SHARPEN TEXTURE FILTERING METHOD */
  _pShell->DeclareSymbol("persistent user INDEX tex_iEffectFiltering;",   &tex_iEffectFiltering);
  _pShell->DeclareSymbol("persistent user INDEX tex_bProgressiveFilter;", &tex_bProgressiveFilter);
  _pShell->DeclareSymbol("           user INDEX tex_bColorizeMipmaps;",   &tex_bColorizeMipmaps);

  _pShell->DeclareSymbol("persistent user INDEX shd_iStaticSize;",   &shd_iStaticSize);
  _pShell->DeclareSymbol("persistent user INDEX shd_iDynamicSize;",  &shd_iDynamicSize);
  _pShell->DeclareSymbol("persistent user INDEX shd_bFineQuality;",  &shd_bFineQuality);
  _pShell->DeclareSymbol("persistent user INDEX shd_iAllowDynamic;", &shd_iAllowDynamic);
  _pShell->DeclareSymbol("persistent user INDEX shd_bDynamicMipmaps;", &shd_bDynamicMipmaps);
  _pShell->DeclareSymbol("persistent user INDEX shd_iFiltering;", &shd_iFiltering);
  _pShell->DeclareSymbol("persistent user INDEX shd_iDithering;", &shd_iDithering);
  _pShell->DeclareSymbol("persistent user FLOAT shd_tmFlushDelay;", &shd_tmFlushDelay);
  _pShell->DeclareSymbol("persistent user FLOAT shd_fCacheSize;",   &shd_fCacheSize);
  _pShell->DeclareSymbol("persistent user INDEX shd_bCacheAll;",    &shd_bCacheAll);
  _pShell->DeclareSymbol("persistent user INDEX shd_bAllowFlats;", &shd_bAllowFlats);
  _pShell->DeclareSymbol("persistent      INDEX shd_iForceFlats;", &shd_iForceFlats);
  _pShell->DeclareSymbol("           user INDEX shd_bShowFlats;",  &shd_bShowFlats);
  _pShell->DeclareSymbol("           user INDEX shd_bColorize;",   &shd_bColorize);
  
  _pShell->DeclareSymbol("           user INDEX gfx_bRenderParticles;", &gfx_bRenderParticles);
  _pShell->DeclareSymbol("           user INDEX gfx_bRenderFog;",       &gfx_bRenderFog);
  _pShell->DeclareSymbol("           user INDEX gfx_bRenderWorld;",     &gfx_bRenderWorld);
  _pShell->DeclareSymbol("persistent user INDEX gfx_iLensFlareQuality;", &gfx_iLensFlareQuality);
  _pShell->DeclareSymbol("persistent user INDEX wld_bTextureLayers;", &wld_bTextureLayers);
  _pShell->DeclareSymbol("persistent user INDEX wld_bRenderMirrors;", &wld_bRenderMirrors);
  _pShell->DeclareSymbol("persistent user FLOAT wld_fEdgeOffsetI;",   &wld_fEdgeOffsetI);
  _pShell->DeclareSymbol("persistent user FLOAT wld_fEdgeAdjustK;",   &wld_fEdgeAdjustK);
  _pShell->DeclareSymbol("persistent user INDEX wld_iDetailRemovingBias;", &wld_iDetailRemovingBias);
  _pShell->DeclareSymbol("           user INDEX wld_bRenderEmptyBrushes;", &wld_bRenderEmptyBrushes);
  _pShell->DeclareSymbol("           user INDEX wld_bRenderShadowMaps;",   &wld_bRenderShadowMaps);
  _pShell->DeclareSymbol("           user INDEX wld_bRenderTextures;",     &wld_bRenderTextures);
  _pShell->DeclareSymbol("           user INDEX wld_bRenderDetailPolygons;", &wld_bRenderDetailPolygons);
  _pShell->DeclareSymbol("           user INDEX wld_bShowTriangles;",        &wld_bShowTriangles);
  _pShell->DeclareSymbol("           user INDEX wld_bShowDetailTextures;",   &wld_bShowDetailTextures);

  _pShell->DeclareSymbol("           user INDEX wed_bIgnoreTJunctions;", &wed_bIgnoreTJunctions);
  _pShell->DeclareSymbol("persistent user INDEX wed_bUseBaseForReplacement;", &wed_bUseBaseForReplacement);


  _pShell->DeclareSymbol("persistent user INDEX tex_iHueShift;",   &tex_iHueShift);
  _pShell->DeclareSymbol("persistent user FLOAT tex_fSaturation;", &tex_fSaturation);
  _pShell->DeclareSymbol("persistent user INDEX shd_iHueShift;",   &shd_iHueShift);
  _pShell->DeclareSymbol("persistent user FLOAT shd_fSaturation;", &shd_fSaturation);
  _pShell->DeclareSymbol("persistent user INDEX gfx_iHueShift;",   &gfx_iHueShift);
  _pShell->DeclareSymbol("persistent user FLOAT gfx_fSaturation;", &gfx_fSaturation);
  _pShell->DeclareSymbol("persistent user FLOAT gfx_fBrightness;", &gfx_fBrightness);
  _pShell->DeclareSymbol("persistent user FLOAT gfx_fContrast;",   &gfx_fContrast);
  _pShell->DeclareSymbol("persistent user FLOAT gfx_fGamma;",      &gfx_fGamma);
  _pShell->DeclareSymbol("persistent user FLOAT gfx_fBiasR;",  &gfx_fBiasR);
  _pShell->DeclareSymbol("persistent user FLOAT gfx_fBiasG;",  &gfx_fBiasG);
  _pShell->DeclareSymbol("persistent user FLOAT gfx_fBiasB;",  &gfx_fBiasB);
  _pShell->DeclareSymbol("persistent user INDEX gfx_iLevels;", &gfx_iLevels);

  _pShell->DeclareSymbol("persistent user INDEX gfx_iStereo;", &gfx_iStereo);
  _pShell->DeclareSymbol("persistent user INDEX gfx_bStereoInvert;", &gfx_bStereoInvert);
  _pShell->DeclareSymbol("persistent user INDEX gfx_iStereoOffset;", &gfx_iStereoOffset);
  _pShell->DeclareSymbol("persistent user FLOAT gfx_fStereoSeparation;", &gfx_fStereoSeparation);

  _pShell->DeclareSymbol( "INDEX sys_bHasTextureCompression;", &sys_bHasTextureCompression);
  _pShell->DeclareSymbol( "INDEX sys_bHasTextureAnisotropy;", &sys_bHasTextureAnisotropy);
  _pShell->DeclareSymbol( "INDEX sys_bHasAdjustableGamma;", &sys_bHasAdjustableGamma);
  _pShell->DeclareSymbol( "INDEX sys_bHasTextureLODBias;", &sys_bHasTextureLODBias);
  _pShell->DeclareSymbol( "INDEX sys_bHasMultitexturing;", &sys_bHasMultitexturing);
  _pShell->DeclareSymbol( "INDEX sys_bHas32bitTextures;", &sys_bHas32bitTextures);
  _pShell->DeclareSymbol( "INDEX sys_bHasSwapInterval;", &sys_bHasSwapInterval);
  _pShell->DeclareSymbol( "INDEX sys_bHasHardwareTnL;", &sys_bHasHardwareTnL);
  _pShell->DeclareSymbol( "INDEX sys_bHasTruform;", &sys_bHasTruform);
  _pShell->DeclareSymbol( "INDEX sys_bHasCVAs;", &sys_bHasCVAs);
  _pShell->DeclareSymbol( "INDEX sys_bUsingOpenGL;",   &sys_bUsingOpenGL);
  _pShell->DeclareSymbol( "INDEX sys_bUsingDirect3D;", &sys_bUsingDirect3D);

  // initialize gfx APIs support
  InitAPIs();
}



// set new display mode
BOOL CGfxLibrary::SetDisplayMode( enum GfxAPIType eAPI, INDEX iAdapter, PIX pixSizeI, PIX pixSizeJ,
                                  enum DisplayDepth eColorDepth)
{
  // some safeties
  ASSERT( pixSizeI>0 && pixSizeJ>0);

  // determine new API
  GfxAPIType eNewAPI = eAPI;
  if( eNewAPI==GAT_CURRENT) eNewAPI = gl_eCurrentAPI;
  
  // shutdown old and startup new API, and mode and ... stuff, you know!
  StopDisplayMode();
  BOOL bRet = StartDisplayMode( eNewAPI, iAdapter, pixSizeI, pixSizeJ, eColorDepth);
  if( !bRet) return FALSE; // didn't make it?

  // update some info
  gl_iCurrentAdapter = gl_gaAPI[gl_eCurrentAPI].ga_iCurrentAdapter = iAdapter;
  gl_dmCurrentDisplayMode.dm_pixSizeI = pixSizeI;
  gl_dmCurrentDisplayMode.dm_pixSizeJ = pixSizeJ;
  gl_dmCurrentDisplayMode.dm_ddDepth  = eColorDepth;
  
  // prepare texture formats for this display mode
  extern void DetermineSupportedTextureFormats( GfxAPIType eAPI);
  DetermineSupportedTextureFormats(gl_eCurrentAPI);

  // made it! (eventually disable windows system keys)
  if( gfx_bDisableWindowsKeys) DisableWindowsKeys();

  return TRUE;
}


// set display mode to original desktop display mode and default ICD driver
BOOL CGfxLibrary::ResetDisplayMode( enum GfxAPIType eAPI/*=GAT_CURRENT*/)
{
  // determine new API
  GfxAPIType eNewAPI = eAPI;
  if( eNewAPI==GAT_CURRENT) eNewAPI = gl_eCurrentAPI;

  // shutdown old and startup new API, and mode and ... stuff, you know!
  StopDisplayMode();
  BOOL bRet = StartDisplayMode( eNewAPI, 0, 0, 0, DD_DEFAULT);
  if( !bRet) return FALSE; // didn't make it?

  // update some info
  gl_iCurrentAdapter = 0;
  gl_dmCurrentDisplayMode.dm_pixSizeI = 0;
  gl_dmCurrentDisplayMode.dm_pixSizeJ = 0;
  gl_dmCurrentDisplayMode.dm_ddDepth  = DD_DEFAULT;

  // prepare texture formats for this display mode
  extern void DetermineSupportedTextureFormats( GfxAPIType eAPI);
  DetermineSupportedTextureFormats(gl_eCurrentAPI);

  // made it!
  EnableWindowsKeys();

  return TRUE;
}



// startup gfx API and set given display mode
BOOL CGfxLibrary::StartDisplayMode( enum GfxAPIType eAPI, INDEX iAdapter, PIX pixSizeI, PIX pixSizeJ,
                                    enum DisplayDepth eColorDepth)
{
  // reinit gamma table
  _fLastBrightness = 999;
  _fLastContrast   = 999;
  _fLastGamma      = 999;
  _iLastLevels = 999;
  _fLastBiasR  = 999;
  _fLastBiasG  = 999;
  _fLastBiasB  = 999;

  // prepare
  BOOL bSuccess;
  ASSERT( iAdapter>=0);
  const BOOL bFullScreen = (pixSizeI>0 && pixSizeJ>0);
  gl_ulFlags &= GLF_ADJUSTABLEGAMMA;
  gl_ctDriverChanges++;
  GFX_bRenderingScene = FALSE;
  GFX_ulLastDrawPortID = 0;  
  gl_iTessellationLevel = 0;
  gl_ctRealTextureUnits = 0;
 _iLastVertexBufferSize = 0;

  // OpenGL driver ?
  if( eAPI==GAT_OGL)
  {
    // disable multimonitor support if it can interfere with OpenGL
    MonitorsOff();
    if( bFullScreen) {
      // set windows mode to fit same size
      bSuccess = CDS_SetMode( pixSizeI, pixSizeJ, eColorDepth);
      if( !bSuccess) return FALSE;
    } else {
      // reset windows mode
      CDS_ResetMode();
    }
    // startup OpenGL
    bSuccess = InitDriver_OGL(iAdapter!=0);
    // try to setup sub-driver
    if( !bSuccess) {
      // reset windows mode and fail
      CDS_ResetMode();
      return FALSE;
    } // made it
    gl_eCurrentAPI = GAT_OGL;
    gl_iSwapInterval = 1234; // need to reset
  }

  // DirectX driver ?
#ifdef SE1_D3D
  else if( eAPI==GAT_D3D)
  {
    // startup D3D
    bSuccess = InitDriver_D3D();
    if( !bSuccess) return FALSE; // what, didn't make it?
    bSuccess = InitDisplay_D3D( iAdapter, pixSizeI, pixSizeJ, eColorDepth);
    if( !bSuccess) return FALSE;
    // made it
    gl_eCurrentAPI = GAT_D3D;
  }
#endif // SE1_D3D

  // no driver
  else
  {
    ASSERT( eAPI==GAT_NONE); 
    gl_eCurrentAPI = GAT_NONE;
  }

  // initialize on first child window
  gl_iFrameNumber = 0;
  gl_pvpActive = NULL;
  gl_ulFlags |= GLF_INITONNEXTWINDOW;
  bFullScreen ? gl_ulFlags|=GLF_FULLSCREEN : gl_ulFlags&=~GLF_FULLSCREEN;
  // mark that some things needs to be reinitialized
  gl_fTextureLODBias = 0.0f;

  // set function pointers
  GFX_SetFunctionPointers( (INDEX)gl_eCurrentAPI);

  // all done
  return TRUE;
}


// Stop display mode and shutdown API
void CGfxLibrary::StopDisplayMode(void)
{
  // release all cached shadows and models' arrays
  extern void Models_ClearVertexArrays(void);
  extern void UncacheShadows(void);
  Models_ClearVertexArrays();
  UncacheShadows();

  // shutdown API
  if( gl_eCurrentAPI==GAT_OGL)
  { // OpenGL
    EndDriver_OGL();
    MonitorsOn();       // re-enable multimonitor support if disabled
    CDS_ResetMode();
  }
#ifdef SE1_D3D
  else if( gl_eCurrentAPI==GAT_D3D)
  { // Direct3D
    EndDriver_D3D();
    MonitorsOn();
  }
#endif // SE1_D3D
  else
  { // none
    ASSERT( gl_eCurrentAPI==GAT_NONE);
  }

  // free driver DLL

#if PLATFORM_WIN32
  if( gl_hiDriver!=NONE) FreeLibrary(gl_hiDriver);
#endif
  gl_hiDriver = NONE;

  // reset some vars
  gl_ctRealTextureUnits = 0;
  gl_eCurrentAPI = GAT_NONE;
  gl_pvpActive = NULL;
  gl_ulFlags &= GLF_ADJUSTABLEGAMMA;

  // reset function pointers
  GFX_SetFunctionPointers( (INDEX)GAT_NONE);
}



// prepare current viewport for rendering
BOOL CGfxLibrary::SetCurrentViewport(CViewPort *pvp)
{
  if( gl_eCurrentAPI==GAT_OGL)  return SetCurrentViewport_OGL(pvp);
#ifdef SE1_D3D
  if( gl_eCurrentAPI==GAT_D3D)  return SetCurrentViewport_D3D(pvp);
#endif // SE1_D3D
  if( gl_eCurrentAPI==GAT_NONE) return TRUE;
  ASSERTALWAYS( "SetCurrenViewport: Wrong API!");
  return FALSE;
}


// Lock a drawport for drawing
BOOL CGfxLibrary::LockDrawPort( CDrawPort *pdpToLock)
{
  // check API
#ifdef SE1_D3D
  ASSERT( gl_eCurrentAPI==GAT_OGL || gl_eCurrentAPI==GAT_D3D || gl_eCurrentAPI==GAT_NONE);
#else // SE1_D3D
  ASSERT( gl_eCurrentAPI==GAT_OGL || gl_eCurrentAPI==GAT_NONE);
#endif // SE1_D3D

  // don't allow locking if drawport is too small
  if( pdpToLock->dp_Width<1 || pdpToLock->dp_Height<1) return FALSE;

  // don't set if same as last
  const ULONG ulThisDrawPortID = pdpToLock->GetID();
  if( GFX_ulLastDrawPortID==ulThisDrawPortID && gap_bOptimizeStateChanges) {
    // just set projection
    pdpToLock->SetOrtho();
    return TRUE;
  }

  // OpenGL ...
  if( gl_eCurrentAPI==GAT_OGL)
  {
    // pass drawport dimensions to OpenGL
    const PIX pixMinSI = pdpToLock->dp_ScissorMinI;
    const PIX pixMaxSI = pdpToLock->dp_ScissorMaxI;
    const PIX pixMinSJ = pdpToLock->dp_Raster->ra_Height -1 - pdpToLock->dp_ScissorMaxJ;
    const PIX pixMaxSJ = pdpToLock->dp_Raster->ra_Height -1 - pdpToLock->dp_ScissorMinJ;
    pglViewport( pixMinSI, pixMinSJ, pixMaxSI-pixMinSI+1, pixMaxSJ-pixMinSJ+1);
    pglScissor(  pixMinSI, pixMinSJ, pixMaxSI-pixMinSI+1, pixMaxSJ-pixMinSJ+1);
    OGL_CHECKERROR;
  }
  // Direct3D ...
#ifdef SE1_D3D
  else if( gl_eCurrentAPI==GAT_D3D)
  { 
    // set viewport
    const PIX pixMinSI = pdpToLock->dp_ScissorMinI;
    const PIX pixMaxSI = pdpToLock->dp_ScissorMaxI;
    const PIX pixMinSJ = pdpToLock->dp_ScissorMinJ;
    const PIX pixMaxSJ = pdpToLock->dp_ScissorMaxJ;
    D3DVIEWPORT8 d3dViewPort = { pixMinSI, pixMinSJ, pixMaxSI-pixMinSI+1, pixMaxSJ-pixMinSJ+1, 0,1 };
    HRESULT hr = gl_pd3dDevice->SetViewport( &d3dViewPort);
    D3D_CHECKERROR(hr);
  }
#endif // SE1_D3D

  // mark and set default projection
  GFX_ulLastDrawPortID = ulThisDrawPortID;
  pdpToLock->SetOrtho();
  return TRUE;
}



// Unlock a drawport after drawing
void CGfxLibrary::UnlockDrawPort( CDrawPort *pdpToUnlock)
{
  // check API
#ifdef SE1_D3D
  ASSERT(gl_eCurrentAPI == GAT_OGL || gl_eCurrentAPI == GAT_D3D || gl_eCurrentAPI == GAT_NONE);
#else // SE1_D3D
  ASSERT(gl_eCurrentAPI == GAT_OGL || gl_eCurrentAPI == GAT_NONE);
#endif // SE1_D3D
  // eventually signalize that scene rendering has ended
}


/////////////////////////////////////////////////////////////////////
// Window canvas functions

/* Create a new window canvas. */
void CGfxLibrary::CreateWindowCanvas(void *hWnd, CViewPort **ppvpNew, CDrawPort **ppdpNew)
{
#if PLATFORM_WIN32
  RECT rectWindow;	// rectangle for the client area of the window

	// get the dimensions from the window
  GetClientRect( (HWND)hWnd, &rectWindow);
  PIX pixWidth  = rectWindow.right  - rectWindow.left;
	PIX pixHeight = rectWindow.bottom - rectWindow.top;

  *ppvpNew = NULL;
  *ppdpNew = NULL;
  // create a new viewport
  if (*ppvpNew = new CViewPort( pixWidth, pixHeight, (HWND)hWnd)) {
    // and it's drawport
		*ppdpNew = &(*ppvpNew)->vp_Raster.ra_MainDrawPort;
  } else {
    delete *ppvpNew;
    *ppvpNew = NULL;
  }
#else
  *ppvpNew = new CViewPort();
  *ppdpNew = &(*ppvpNew)->vp_Raster.ra_MainDrawPort;
#endif
}

/* Destroy a window canvas. */
void CGfxLibrary::DestroyWindowCanvas(CViewPort *pvpOld) {
	// delete the viewport
  delete pvpOld;
}

/////////////////////////////////////////////////////////////////////
// Work canvas functions

#define WorkCanvasCLASS "WorkCanvas Window"
static BOOL _bClassRegistered = FALSE;

/* Create a work canvas. */
void CGfxLibrary::CreateWorkCanvas(PIX pixWidth, PIX pixHeight, CDrawPort **ppdpNew)
{
#if PLATFORM_WIN32
  // must have dimensions
	ASSERT (pixWidth>0 || pixHeight>0);

  if (!_bClassRegistered) {
    _bClassRegistered = TRUE;
    WNDCLASSA wc;

    // must have owndc for opengl and dblclks to give to parent
    wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = NULL;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WorkCanvasCLASS;
    RegisterClassA(&wc);
  }

  // create a window
  HWND hWnd = ::CreateWindowExA(
	  0,
	  WorkCanvasCLASS,
	  "",   // title
    WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_POPUP,
	  0,0,
	  pixWidth, pixHeight,  // window size
	  NULL,
	  NULL,
	  NULL, //hInstance,
	  NULL);
  ASSERT(hWnd != NULL);

  *ppdpNew = NULL;
  CViewPort *pvp;
  CreateWindowCanvas(hWnd, &pvp, ppdpNew);
#else
  FatalError("Should not be called");
#endif
}

/* Destroy a work canvas. */
void CGfxLibrary::DestroyWorkCanvas(CDrawPort *pdpOld)
{
  CViewPort *pvp = pdpOld->dp_Raster->ra_pvpViewPort;
  HWND hwnd = pvp->vp_hWndParent;
  DestroyWindowCanvas(pvp);
//  ::DestroyWindow(hwnd);
}



// optimize memory used by cached shadow maps

#define SHADOWMAXBYTES (256*256*4*4/3)
static SLONG slCachedShadowMemory=0, slDynamicShadowMemory=0;
static INDEX ctCachedShadows=0, ctFlatShadows=0, ctDynamicShadows=0;
extern BOOL _bShadowsUpdated = TRUE;

void CGfxLibrary::ReduceShadows(void)
{
  _sfStats.StartTimer( CStatForm::STI_SHADOWUPDATE);

  // clamp shadow caching variables
  shd_fCacheSize    = Clamp( shd_fCacheSize,   0.1f, 128.0f);
  shd_tmFlushDelay  = Clamp( shd_tmFlushDelay, 0.1f, 120.0f);
  CTimerValue tvNow = _pTimer->GetHighPrecisionTimer(); // readout current time
  const TIME tmAcientDelay = Clamp( shd_tmFlushDelay*3, 60.0f, 300.0f);

  // determine cached shadowmaps stats (if needed)
  if( _bShadowsUpdated)
  {
    _bShadowsUpdated = FALSE;
    slCachedShadowMemory=0; slDynamicShadowMemory=0;
    ctCachedShadows=0; ctFlatShadows=0; ctDynamicShadows=0;
    {FORDELETELIST( CShadowMap, sm_lnInGfx, _pGfx->gl_lhCachedShadows, itsm) { 
      CShadowMap &sm = *itsm;
      ASSERT( sm.sm_pulCachedShadowMap!=NULL); // must be cached
      ASSERT( sm.sm_slMemoryUsed>0 && sm.sm_slMemoryUsed<=SHADOWMAXBYTES); // and have valid size
      // remove acient shadowmaps from list (if allowed)
      const TIME tmDelta = (tvNow-sm.sm_tvLastDrawn).GetSeconds();
      if( tmDelta>tmAcientDelay && !(sm.sm_ulFlags&SMF_PROBED) && !shd_bCacheAll) {
        sm.Uncache();
        continue;
      }
      // determine type and occupied space
      const BOOL bDynamic = sm.sm_pulDynamicShadowMap!=NULL;
      const BOOL bFlat    = sm.sm_pulCachedShadowMap==&sm.sm_colFlat;
      if( bDynamic) { slDynamicShadowMemory += sm.sm_slMemoryUsed;    ctDynamicShadows++; }
      if( !bFlat)   { slCachedShadowMemory  += sm.sm_slMemoryUsed;    ctCachedShadows++;  }
      else          { slCachedShadowMemory  += sizeof(sm.sm_colFlat); ctFlatShadows++;    }
    }}
  }
  // update statistics counters
  _pfGfxProfile.IncrementCounter(CGfxProfile::PCI_CACHEDSHADOWBYTES,  slCachedShadowMemory);
  _pfGfxProfile.IncrementCounter(CGfxProfile::PCI_CACHEDSHADOWS,      ctCachedShadows);
  _pfGfxProfile.IncrementCounter(CGfxProfile::PCI_FLATSHADOWS,        ctFlatShadows);
  _pfGfxProfile.IncrementCounter(CGfxProfile::PCI_DYNAMICSHADOWBYTES, slDynamicShadowMemory);
  _pfGfxProfile.IncrementCounter(CGfxProfile::PCI_DYNAMICSHADOWS,     ctDynamicShadows);
       _sfStats.IncrementCounter(  CStatForm::SCI_CACHEDSHADOWBYTES,  slCachedShadowMemory);
       _sfStats.IncrementCounter(  CStatForm::SCI_CACHEDSHADOWS,      ctCachedShadows);
       _sfStats.IncrementCounter(  CStatForm::SCI_FLATSHADOWS,        ctFlatShadows);
       _sfStats.IncrementCounter(  CStatForm::SCI_DYNAMICSHADOWBYTES, slDynamicShadowMemory);
       _sfStats.IncrementCounter(  CStatForm::SCI_DYNAMICSHADOWS,     ctDynamicShadows);

  // done if reducing is not allowed 
  if( shd_bCacheAll) {
    _sfStats.StopTimer( CStatForm::STI_SHADOWUPDATE);
    return;  
  }

  // optimize only if low on memory                                
  ULONG ulShadowCacheSize  = (ULONG)(shd_fCacheSize*1024*1024); // in bytes
  ULONG ulUsedShadowMemory = slCachedShadowMemory + slDynamicShadowMemory;
  if( ulUsedShadowMemory  <= ulShadowCacheSize) {
    _sfStats.StopTimer( CStatForm::STI_SHADOWUPDATE);
    return;
  }

  // reduce shadow delay if needed
  // (lineary from specified value to 2sec for cachedsize>specsize to cachedsize>2*specsize)
  TIME tmFlushDelay = shd_tmFlushDelay;
  if( tmFlushDelay>2.0f) {
    FLOAT fRatio = (FLOAT)ulUsedShadowMemory / ulShadowCacheSize;
    ASSERT( fRatio>=1.0f);
    fRatio = ClampUp( fRatio/2.0f, 1.0f);
    tmFlushDelay = Lerp( tmFlushDelay, 2.0f, fRatio);
  }

  // loop thru cached shadowmaps list
  {FORDELETELIST( CShadowMap, sm_lnInGfx, _pGfx->gl_lhCachedShadows, itsm)
  { // stop iteration if current shadow map it is not too old (list is sorted by time)
    // or we have enough memory for cached shadows that remain
    CShadowMap &sm = *itsm;
    const TIME tmDelta = (tvNow-sm.sm_tvLastDrawn).GetSeconds();
    if( tmDelta<tmFlushDelay || ulUsedShadowMemory<ulShadowCacheSize) break;
    // uncache shadow (this returns ammount of memory that has been freed)
    ulUsedShadowMemory -= sm.Uncache();
    ASSERT( ulUsedShadowMemory>=0);
  }}
  // done
  _sfStats.StopTimer( CStatForm::STI_SHADOWUPDATE);
}



// some vars for probing
extern INDEX _ctProbeTexs = 0;
extern INDEX _ctProbeShdU = 0;
extern INDEX _ctProbeShdB = 0;
extern INDEX _ctFullShdU  = 0;
extern SLONG _slFullShdUBytes = 0;
static BOOL GenerateGammaTable(void);



/*
 * Swap buffers in a viewport.
 */
void CGfxLibrary::SwapBuffers(CViewPort *pvp)
{
  // check API
#ifdef SE1_D3D
  ASSERT(gl_eCurrentAPI == GAT_OGL || gl_eCurrentAPI == GAT_D3D || gl_eCurrentAPI == GAT_NONE);
#else // SE1_D3D
  ASSERT(gl_eCurrentAPI == GAT_OGL || gl_eCurrentAPI == GAT_NONE);
#endif // SE1_D3D

  // safety check
  ASSERT( gl_pvpActive!=NULL);
  if( pvp!=gl_pvpActive) {
    ASSERTALWAYS( "Swapping viewport that was not last drawn to!");
    return;
  }

  // optimize memory used by cached shadow maps and update shadowmap counters
  ReduceShadows();

  // check and eventually adjust texture filtering and LOD biasing
  gfxSetTextureFiltering( gap_iTextureFiltering, gap_iTextureAnisotropy);
  gfxSetTextureBiasing( gap_fTextureLODBias);

  // clamp some cvars
  gap_iDithering = Clamp( gap_iDithering, 0L, 2L);
  gap_iSwapInterval = Clamp( gap_iSwapInterval, 0L, 4L);
  gap_iOptimizeClipping = Clamp( gap_iOptimizeClipping, 0L, 2L);
  gap_iTruformLevel = Clamp( gap_iTruformLevel, 0L, _pGfx->gl_iMaxTessellationLevel);
  ogl_iFinish = Clamp( ogl_iFinish, 0L, 3L);
  d3d_iFinish = Clamp( d3d_iFinish, 0L, 3L);

  // OpenGL  
  if( gl_eCurrentAPI==GAT_OGL)
  {
    // force finishing of all rendering operations (if required)
    if( ogl_iFinish==2) gfxFinish();

    // check state of swap interval extension usage
    if( gl_ulFlags & GLF_VSYNC) {
      if( gl_iSwapInterval != gap_iSwapInterval) {
        gl_iSwapInterval = gap_iSwapInterval;
        pwglSwapIntervalEXT( gl_iSwapInterval);
      }
    }
    // swap buffers
    if (!eglSwapBuffers(pvp->display, pvp->surface)) {
      WarningMessage("eglSwapBuffers() returned error 0x%04X", eglGetError());
    }

    // force finishing of all rendering operations (if required)
    if( ogl_iFinish==3) gfxFinish();

    // reset CVA usage if ext is not present
    if( !(gl_ulFlags&GLF_EXT_COMPILEDVERTEXARRAY)) ogl_bUseCompiledVertexArrays = 0;
  }

  // Direct3D
#ifdef SE1_D3D
  else if( gl_eCurrentAPI==GAT_D3D)
  {
    // force finishing of all rendering operations (if required)
    if( d3d_iFinish==2) gfxFinish();

    // end scene rendering
    HRESULT hr;
    if( GFX_bRenderingScene) {
      hr = gl_pd3dDevice->EndScene(); 
      D3D_CHECKERROR(hr);
    }
    CDisplayMode dm;
    GetCurrentDisplayMode(dm);
    ASSERT( (dm.dm_pixSizeI==0 && dm.dm_pixSizeJ==0) || (dm.dm_pixSizeI!=0 && dm.dm_pixSizeJ!=0));
    if( dm.dm_pixSizeI==0 || dm.dm_pixSizeJ==0 ) {
      // windowed mode
      hr = pvp->vp_pSwapChain->Present( NULL, NULL, NULL, NULL);
    } else {
      // full screen mode
      hr = gl_pd3dDevice->Present( NULL, NULL, NULL, NULL);
    } // done swapping
    D3D_CHECKERROR(hr); 

    // force finishing of all rendering operations (if required)
    if( d3d_iFinish==3) gfxFinish();

    // eventually reset vertex buffer if something got changed
    if( _iLastVertexBufferSize!=d3d_iVertexBuffersSize
    || (gl_iTessellationLevel<1 && gap_iTruformLevel>0)
    || (gl_iTessellationLevel>0 && gap_iTruformLevel<1)) {
      extern void SetupVertexArrays_D3D( INDEX ctVertices);
      extern void SetupIndexArray_D3D( INDEX ctVertices);
      extern DWORD SetupShader_D3D( ULONG ulStreamsMask);
      SetupShader_D3D(NONE); 
      SetupVertexArrays_D3D(0); 
      SetupIndexArray_D3D(0);
      extern INDEX VerticesFromSize_D3D( SLONG &slSize);
      const INDEX ctVertices = VerticesFromSize_D3D(d3d_iVertexBuffersSize);
      SetupVertexArrays_D3D(ctVertices); 
      SetupIndexArray_D3D(2*ctVertices);
     _iLastVertexBufferSize = d3d_iVertexBuffersSize;
    } 
  }
#endif // SE1_D3D
  // update tessellation level
  gl_iTessellationLevel = gap_iTruformLevel;

  // must reset drawport and rendering status for subsequent locks
  GFX_ulLastDrawPortID = 0;  
  GFX_bRenderingScene  = FALSE;
  // reset frustum/ortho matrix, too
  extern BOOL  GFX_bViewMatrix;
  extern FLOAT GFX_fLastL, GFX_fLastR, GFX_fLastT, GFX_fLastB, GFX_fLastN, GFX_fLastF;
  GFX_fLastL = GFX_fLastR = GFX_fLastT = GFX_fLastB = GFX_fLastN = GFX_fLastF = 0;
  GFX_bViewMatrix = TRUE;

  // set maximum allowed upload ammount
  gfx_iProbeSize = Clamp( gfx_iProbeSize, 1L, 16384L);
  gl_slAllowedUploadBurst = gfx_iProbeSize *1024; 
  _ctProbeTexs = 0;
  _ctProbeShdU = 0;
  _ctProbeShdB = 0;
  _ctFullShdU  = 0;
  _slFullShdUBytes = 0;

  // keep time when swap buffer occured and maintain counter of frames for temporal coherence checking
  gl_tvFrameTime = _pTimer->GetHighPrecisionTimer();
  gl_iFrameNumber++;
  // reset profiling counters
  gl_ctWorldTriangles    = 0;
  gl_ctModelTriangles    = 0;
  gl_ctParticleTriangles = 0;
  gl_ctTotalTriangles    = 0;

  // re-adjust multi-texturing support
  gap_iUseTextureUnits = Clamp( gap_iUseTextureUnits, 1L, _pGfx->gl_ctTextureUnits);
  ASSERT( gap_iUseTextureUnits>=1 && gap_iUseTextureUnits<=GFX_MAXTEXUNITS);

  // re-get usage of compiled vertex arrays
  CVA_b2D     = ogl_bUseCompiledVertexArrays /100;
  CVA_bWorld  = ogl_bUseCompiledVertexArrays /10 %10;
  CVA_bModels = ogl_bUseCompiledVertexArrays %10;    
  ogl_bUseCompiledVertexArrays = 0;
  if( CVA_b2D)     ogl_bUseCompiledVertexArrays += 100;
  if( CVA_bWorld)  ogl_bUseCompiledVertexArrays += 10;
  if( CVA_bModels) ogl_bUseCompiledVertexArrays += 1;

  // eventually advance to next sample buffer
  if( (gl_ulFlags&GLF_EXT_TBUFFER) && go_ctSampleBuffers>1) {
    go_iCurrentWriteBuffer--;
    if( go_iCurrentWriteBuffer<0) go_iCurrentWriteBuffer = go_ctSampleBuffers-1;
    pglDisable( GL_MULTISAMPLE_3DFX);
  }

  // clear viewport if needed
  if( gfx_bClearScreen) pvp->vp_Raster.ra_MainDrawPort.Fill( C_BLACK|CT_OPAQUE);
  //pvp->vp_Raster.ra_MainDrawPort.FillZBuffer(ZBUF_BACK);

  // adjust gamma table if supported ...
  if( gl_ulFlags & GLF_ADJUSTABLEGAMMA) {
    // ... and required
    const BOOL bTableSet = GenerateGammaTable();
    if( bTableSet) {
      if( gl_eCurrentAPI==GAT_OGL) {
//        CTempDC tdc(pvp->vp_hWnd);
//        SetDeviceGammaRamp( tdc.hdc, &_auwGammaTable[0]);
      } 
#ifdef SE1_D3D
      else if( gl_eCurrentAPI==GAT_D3D) {
        gl_pd3dDevice->SetGammaRamp( D3DSGR_NO_CALIBRATION, (D3DGAMMARAMP*)&_auwGammaTable[0]);
      }
#endif // SE1_D3D
    }
  }
  // if not supported
  else {
    // just reset settings to default
    gfx_fBrightness = 0;
    gfx_fContrast   = 1;
    gfx_fGamma      = 1;
    gfx_fBiasR  = 1;
    gfx_fBiasG  = 1;
    gfx_fBiasB  = 1;
    gfx_iLevels = 256;
  }
}




// get array of all supported display modes
CDisplayMode *CGfxLibrary::EnumDisplayModes( INDEX &ctModes, enum GfxAPIType eAPI/*=GAT_CURRENT*/, INDEX iAdapter/*=0*/)
{
  if( eAPI==GAT_CURRENT) eAPI = gl_eCurrentAPI;
  if( iAdapter==0) iAdapter = gl_iCurrentAdapter;
  CDisplayAdapter *pda = &gl_gaAPI[eAPI].ga_adaAdapter[iAdapter];
  ctModes = pda->da_ctDisplayModes;
  return &pda->da_admDisplayModes[0];
}



// Lock a raster for drawing.
BOOL CGfxLibrary::LockRaster( CRaster *praToLock)
{
  // don't do this! it can break sync consistency in entities!
  // SetFPUPrecision(FPT_24BIT); 
  ASSERT( praToLock->ra_pvpViewPort!=NULL);
  BOOL bRes = SetCurrentViewport( praToLock->ra_pvpViewPort);
  if( bRes) {
    // must signal to picky Direct3D
#ifdef SE1_D3D
    if( gl_eCurrentAPI==GAT_D3D && !GFX_bRenderingScene) {  
      HRESULT hr = gl_pd3dDevice->BeginScene(); 
      D3D_CHECKERROR(hr);
      bRes = (hr==D3D_OK);
    } // mark it
#endif // SE1_D3D
    GFX_bRenderingScene = TRUE;
  } // done
  return bRes;
}


// Unlock a raster after drawing.
void CGfxLibrary::UnlockRaster( CRaster *praToUnlock)
{
  // don't do this! it can break sync consistency in entities!
  // SetFPUPrecision(FPT_53BIT);
  ASSERT( GFX_bRenderingScene);
}



// generates gamma table and returns true if gamma table has been changed
static BOOL GenerateGammaTable(void)
{
  // only if needed
  if( _fLastBrightness == gfx_fBrightness
   && _fLastContrast   == gfx_fContrast
   && _fLastGamma      == gfx_fGamma
   && _iLastLevels == gfx_iLevels
   && _fLastBiasR  == gfx_fBiasR
   && _fLastBiasG  == gfx_fBiasG
   && _fLastBiasB  == gfx_fBiasB) return FALSE;

  // guess it's needed
  INDEX i;
  gfx_fBrightness = Clamp( gfx_fBrightness, -0.8f, 0.8f);
  gfx_fContrast   = Clamp( gfx_fContrast,    0.2f, 4.0f);
  gfx_fGamma      = Clamp( gfx_fGamma,  0.2f, 4.0f);    
  gfx_iLevels = Clamp( gfx_iLevels, 2L,  256L);
  gfx_fBiasR  = Clamp( gfx_fBiasR, 0.0f, 2.0f);
  gfx_fBiasG  = Clamp( gfx_fBiasG, 0.0f, 2.0f);
  gfx_fBiasB  = Clamp( gfx_fBiasB, 0.0f, 2.0f);
  _fLastBrightness = gfx_fBrightness;
  _fLastContrast   = gfx_fContrast;
  _fLastGamma      = gfx_fGamma;
  _iLastLevels = gfx_iLevels;
  _fLastBiasR  = gfx_fBiasR;
  _fLastBiasG  = gfx_fBiasG;
  _fLastBiasB  = gfx_fBiasB;
                
  // fill and adjust gamma
  const FLOAT f1oGamma = 1.0f / gfx_fGamma;
  for( i=0; i<256; i++) {
    FLOAT fVal = i/255.0f;
    fVal = Clamp( (FLOAT)pow(fVal,f1oGamma), 0.0f, 1.0f);
    _auwGammaTable[i] = (UWORD)(fVal*65280);
  }

  // adjust contrast
  for( i=0; i<256; i++) {
    FLOAT fVal = _auwGammaTable[i]/65280.0f;
    fVal = Clamp( (fVal-0.5f)*gfx_fContrast +0.5f, 0.0f, 1.0f);
    _auwGammaTable[i] = (UWORD)(fVal*65280);
  }

  // adjust brightness
  INDEX iAdd = 256* 256*gfx_fBrightness;
  for( i=0; i<256; i++) {
    _auwGammaTable[i] = Clamp( _auwGammaTable[i]+iAdd, 0L, 65280L);
  }

  // adjust levels (posterize)
  if( gfx_iLevels<256) {
    const FLOAT fLevels = 256 * 256.0f/gfx_iLevels;
    for( i=0; i<256; i++) {
      INDEX iVal = _auwGammaTable[i];
      iVal = ((INDEX)(iVal/fLevels)) *fLevels;
      _auwGammaTable[i] = ClampUp( iVal, 0xFF00L);
    }
  }

  // copy R to G and B array
  for( i=0; i<256; i++) {
    FLOAT fR,fG,fB;
    fR=fG=fB = _auwGammaTable[i]/65280.0f;
    fR = Clamp( fR*gfx_fBiasR, 0.0f, 1.0f);
    fG = Clamp( fG*gfx_fBiasG, 0.0f, 1.0f);
    fB = Clamp( fB*gfx_fBiasB, 0.0f, 1.0f);
    _auwGammaTable[i+0]   = (UWORD)(fR*65280);
    _auwGammaTable[i+256] = (UWORD)(fG*65280);
    _auwGammaTable[i+512] = (UWORD)(fB*65280);
  }

  // done
  return TRUE;
}



#if 0


DeclareSymbol( "[persistent] [hidden] [const] [type] name [minval] [maxval] [func()]", &shd_iStaticQuality, func()=NULL);


_pShell->DeclareSymbol( "INDEX GfxVarPreFunc(INDEX);", (void*) &GfxVarPreFunc);
_pShell->DeclareSymbol( "void GfxVarPostFunc(INDEX);", (void*) &GfxVarPostFunc);

static BOOL GfxVarPreFunc(void *pvVar)
{
  if (pvVar==&gfx_fSaturation) {
    CPrintF("cannot change saturation: just for test\n");
    return FALSE;
  } else {
    CPrintF("gfx var about to be changed\n");
    return TRUE;
  }
}

static void GfxVarPostFunc(void *pvVar)
{
  if (pvVar==&shd_bFineQuality) {
    CPrintF("This requires RefreshTextures() to take effect!\n");
  }
}


#endif




