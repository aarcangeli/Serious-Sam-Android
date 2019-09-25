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

#include <Engine/Brushes/Brush.h>
#include <Engine/Brushes/BrushTransformed.h>
#include <Engine/Light/LightSource.h>
#include <Engine/Light/Gradient.h>
#include <Engine/Base/ListIterator.inl>
#include <Engine/Base/Statistics_Internal.h>
#include <Engine/Graphics/Color.h>
#include <Engine/Math/FixInt.h>
#include <Engine/Entities/Entity.h>
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Math/Clipping.inl>

#include <Engine/Light/Shadows_internal.h>
#include <Engine/World/WorldEditingProfile.h>

#include <Engine/Templates/StaticArray.cpp>
#include <Engine/Templates/DynamicArray.cpp>
#include <Engine/Base/LargeColor.h>
#include <cstdint>

#define WITH_BOUND_CHECK 0

// asm shortcuts
#define O offset
#define Q qword ptr
#define D dword ptr
#define W  word ptr
#define B  byte ptr

#define ASMOPT 0


extern INDEX shd_bFineQuality;
extern INDEX shd_iFiltering;
extern INDEX shd_iDithering;

extern const UBYTE *pubClipByte;
extern UBYTE aubSqrt[  SQRTTABLESIZE];
extern UWORD auw1oSqrt[SQRTTABLESIZE];
extern UWORD auw1oSqrt[SQRTTABLESIZE];
// static FLOAT3D _v00;

#if WITH_BOUND_CHECK
CBrushShadowMap *g_pbsm;
#endif

// internal class for layer mixing
class CLayerMixer
{
public:
  CBrushShadowMap *lm_pbsmShadowMap;   // shadow map whose layers are mixed
  CBrushPolygon   *lm_pbpoPolygon;     // polygon of the shadow map
  BOOL lm_bDynamic;    // set while doing dynamic light mixing

  // dimensions of currently processed shadow map
  MEX   lm_mexOffsetU;   // offsets in mex
  MEX   lm_mexOffsetV;
  INDEX lm_iFirstLevel;  // mip level of first possible mip-map
  INDEX lm_iMipLevel;    // mip level
  INDEX lm_iMipShift;    // relative mip level (miplevel-firstlevel)
  PIX   lm_pixCanvasSizeU;     // shadowmap canvas size in pixels
  PIX   lm_pixCanvasSizeV;
  PIX   lm_pixPolygonSizeU;    // polygon (used part of shadowmap size in pixels
  PIX   lm_pixPolygonSizeV;
  ULONG*lm_pulShadowMap;       // buffer for final shadow map
  ULONG*lm_pulStaticShadowMap; // precached static shadow map

  // gradients for shadow map walking
  FLOAT3D lm_vO;     // upper left corner of shadow map in 3D
  FLOAT3D lm_vStepU; // step between pixels in same row
  FLOAT3D lm_vStepV; // step between rows
  FLOAT3D lm_vLightDirection; // light direction for directional light sources
  CLightSource *lm_plsLight;  // current light source
  // color components of current light
  COLOR lm_colLight;
  COLOR lm_colAmbient;

  // constructor
  CLayerMixer( CBrushShadowMap *pbsm, INDEX iFirstMip, INDEX iLastMip, BOOL bDynamic);

  // remember general data
  void CalculateData( CBrushShadowMap *pbsm, INDEX iMipmap);
  // mix one mip-map
  void MixOneMipmap( CBrushShadowMap *pbsm, INDEX iMipmap);
  // mix dynamic lights
  void MixOneMipmapDynamic(CBrushShadowMap *pbsm, INDEX iMipmap);

  // find start of a mip-map inside a layer
  void FindLayerMipmap( CBrushShadowLayer *pbsl, UBYTE *&pub, UBYTE &ubMask);

  // add one point layer to the shadow map
  void AddAmbientPoint(void);
  void AddAmbientMaskPoint( UBYTE *pubMask, UBYTE ubMask);
  void AddDiffusionPoint(void);
  void AddDiffusionMaskPoint( UBYTE *pubMask, UBYTE ubMask);
  BOOL PrepareOneLayerPoint( CBrushShadowLayer *pbsl, BOOL bNoMask);
  void AddOneLayerPoint( CBrushShadowLayer *pbsl, UBYTE *pub, UBYTE ubMask=0);

  // add one directional layer to the shadow map
  void AddDirectional(void);
  void AddMaskDirectional( UBYTE *pubMask, UBYTE ubMask);
  void AddOneLayerDirectional( CBrushShadowLayer *pbsl, UBYTE *pub, UBYTE ubMask=0);

  // add one gradient layer to the shadow map
  void AddOneLayerGradient( CGradientParameters &gp);

  // add the intensity to the pixel
  inline void AddToCluster( UBYTE *pub);
  inline void AddAmbientToCluster( UBYTE *pub);
  inline void AddToCluster( UBYTE *pub, FLOAT fIntensity);

  // additional functions
  __forceinline void CopyShadowLayer(void);
  __forceinline void FillShadowLayer( COLOR col);
  // inline FLOAT IntensityAtDistance( FLOAT fDistance, FLOAT fMinDistance);
  // FLOAT lm_fLightMax, lm_fLightStep;
};


// increment a byte without overflowing it
static inline void IncrementByteWithClip( UBYTE &ub, SLONG slAdd)
{
  ub = pubClipByte[(SLONG)ub+slAdd];
}

// increment a color without overflowing it
static inline void IncrementColorWithClip( UBYTE &ubR, UBYTE &ubG, UBYTE &ubB,
                                           SLONG  slR, SLONG  slG, SLONG  slB)
{
  IncrementByteWithClip( ubR, slR);
  IncrementByteWithClip( ubG, slG);
  IncrementByteWithClip( ubB, slB);
}

// add the intensity to the pixel
inline void CLayerMixer::AddToCluster( UBYTE *pub)
{
  IncrementByteWithClip(pub[0], ((UBYTE*)&lm_colLight)[3]);
  IncrementByteWithClip(pub[1], ((UBYTE*)&lm_colLight)[2]);
  IncrementByteWithClip(pub[2], ((UBYTE*)&lm_colLight)[1]);
}
inline void CLayerMixer::AddAmbientToCluster( UBYTE *pub)
{
  IncrementByteWithClip(pub[0], ((UBYTE*)&lm_colAmbient)[3]);
  IncrementByteWithClip(pub[1], ((UBYTE*)&lm_colAmbient)[2]);
  IncrementByteWithClip(pub[2], ((UBYTE*)&lm_colAmbient)[1]);
}
inline void CLayerMixer::AddToCluster( UBYTE *pub, FLOAT fIntensity)
{
  IncrementByteWithClip(pub[0], ((UBYTE*)&lm_colLight)[3] *fIntensity);
  IncrementByteWithClip(pub[1], ((UBYTE*)&lm_colLight)[2] *fIntensity);
  IncrementByteWithClip(pub[2], ((UBYTE*)&lm_colLight)[1] *fIntensity);
}

  
// remember general data
void CLayerMixer::CalculateData( CBrushShadowMap *pbsm, INDEX iMipmap)
{
  _pfWorldEditingProfile.StartTimer(CWorldEditingProfile::PTI_CALCULATEDATA);

  // cache class vars
  lm_pbsmShadowMap = pbsm;
  lm_pbpoPolygon   = pbsm->GetBrushPolygon();

  lm_mexOffsetU  = pbsm->sm_mexOffsetX;
  lm_mexOffsetV  = pbsm->sm_mexOffsetY;
  lm_iFirstLevel = pbsm->sm_iFirstMipLevel;
  lm_iMipLevel   = iMipmap;
  lm_iMipShift   = lm_iMipLevel - lm_iFirstLevel;
  lm_pixCanvasSizeU  = pbsm->sm_mexWidth >>lm_iMipLevel;
  lm_pixCanvasSizeV  = pbsm->sm_mexHeight>>lm_iMipLevel;
  lm_pixPolygonSizeU = Min( lm_pixCanvasSizeU, (PIX)(lm_pbsmShadowMap->sm_pixPolygonSizeU >>lm_iMipShift)+1L);
  lm_pixPolygonSizeV = Min( lm_pixCanvasSizeV, (PIX)(lm_pbsmShadowMap->sm_pixPolygonSizeV >>lm_iMipShift)+1L);

  // determine where this mip-map is relative to the allocated shadow map memory
  PIX pixOffset = pbsm->sm_slMemoryUsed/BYTES_PER_TEXEL
                - GetMipmapOffset( 15, lm_pixCanvasSizeU, lm_pixCanvasSizeV);

  // get right pointers to the shadow mipmap
  if( lm_bDynamic) {
    lm_pulShadowMap       = pbsm->sm_pulDynamicShadowMap + pixOffset;
    lm_pulStaticShadowMap = pbsm->sm_pulCachedShadowMap  + pixOffset;
  } else {
    lm_pulShadowMap       = pbsm->sm_pulCachedShadowMap  + pixOffset;
    lm_pulStaticShadowMap = NULL;
  }

  // prepare 3D positions
  CEntity *penWithPolygon = lm_pbpoPolygon->bpo_pbscSector->bsc_pbmBrushMip->bm_pbrBrush->br_penEntity;
  ASSERT(penWithPolygon!=NULL);
  const FLOATmatrix3D &mPolygonRotation = penWithPolygon->en_mRotation;
  const FLOAT3D &vPolygonTranslation = penWithPolygon->GetPlacement().pl_PositionVector;

  // get first pixel in texture in 3D
  Vector<MEX, 2> vmex0;
  vmex0(1) = -lm_mexOffsetU+(1<<(lm_iMipLevel-1));
  vmex0(2) = -lm_mexOffsetV+(1<<(lm_iMipLevel-1));
  lm_pbpoPolygon->bpo_mdShadow.GetSpaceCoordinates(
    lm_pbpoPolygon->bpo_pbplPlane->bpl_pwplWorking->wpl_mvRelative, vmex0, lm_vO);
  lm_vO = lm_vO*mPolygonRotation+vPolygonTranslation;

  // get steps for walking in texture in 3D
  Vector<MEX, 2> vmexU, vmexV;
  vmexU(1) = (1<<lm_iMipLevel)-lm_mexOffsetU+(1<<(lm_iMipLevel-1));
  vmexU(2) = (0<<lm_iMipLevel)-lm_mexOffsetV+(1<<(lm_iMipLevel-1));
  vmexV(1) = (0<<lm_iMipLevel)-lm_mexOffsetU+(1<<(lm_iMipLevel-1));
  vmexV(2) = (1<<lm_iMipLevel)-lm_mexOffsetV+(1<<(lm_iMipLevel-1));

  lm_pbpoPolygon->bpo_mdShadow.GetSpaceCoordinates(
    lm_pbpoPolygon->bpo_pbplPlane->bpl_pwplWorking->wpl_mvRelative, vmexU, lm_vStepU);
  lm_vStepU = lm_vStepU*mPolygonRotation+vPolygonTranslation;
  lm_pbpoPolygon->bpo_mdShadow.GetSpaceCoordinates(
    lm_pbpoPolygon->bpo_pbplPlane->bpl_pwplWorking->wpl_mvRelative, vmexV, lm_vStepV);
  lm_vStepV = lm_vStepV*mPolygonRotation+vPolygonTranslation;
  lm_vStepU-= lm_vO;
  lm_vStepV-= lm_vO;

  ASSERT( lm_pixPolygonSizeU>0 && lm_pixPolygonSizeV>0);
  _pfWorldEditingProfile.StopTimer(CWorldEditingProfile::PTI_CALCULATEDATA);
}


// find start of a mip-map inside a layer
void CLayerMixer::FindLayerMipmap( CBrushShadowLayer *pbsl, UBYTE *&pub, UBYTE &ubMask)
{
  struct MipmapTable mmtLayer;
  // find mip-mapping information for the layer
  MakeMipmapTable(pbsl->bsl_pixSizeU, pbsl->bsl_pixSizeV, mmtLayer);
  // get pixel offset of the mipmap
  SLONG slPixOffset = mmtLayer.mmt_aslOffsets[lm_iMipLevel-lm_iFirstLevel];
  // convert offset to bits
  pub = pbsl->bsl_pubLayer + (slPixOffset>>3);
  ubMask = 1<<(slPixOffset&7);
}



#define FTOX   0x10000000
#define SHIFTX (28-SQRTTABLESIZELOG2)

// static variables for easier transfers
static const FLOAT3D *_vLight;
static FLOAT _fMinLightDistance, _f1oFallOff;
static INDEX _iPixCt, _iRowCt;
static SLONG _slModulo;
static ULONG _ulLightFlags, _ulPolyFlags;
static SLONG _slL2Row, _slDDL2oDU, _slDDL2oDV, _slDDL2oDUoDV, _slDL2oDURow, _slDL2oDV;
static SLONG _slLightMax, _slHotSpot, _slLightStep;
static ULONG *_pulLayer;


// add one layer point light without diffusion and mask
void CLayerMixer::AddAmbientPoint(void)
{

#if ASMOPT == 1

  // prepare some local variables
  __int64 mmDDL2oDU = _slDDL2oDU;
  __int64 mmDDL2oDV = _slDDL2oDV;
  ULONG ulLightRGB = ByteSwap(lm_colLight);
  _slLightMax<<=7;
  _slLightStep>>=1;

  __asm {
    // prepare interpolants
    movd    mm0,D [_slL2Row]
    movd    mm1,D [_slDL2oDURow]
    psllq   mm1,32
    por     mm1,mm0         // MM1 = slDL2oDURow | slL2Row
    movd    mm0,D [_slDL2oDV]
    movd    mm2,D [_slDDL2oDUoDV]
    psllq   mm2,32
    por     mm2,mm0         // MM2 = slDDL2oDUoDV | slDL2oDV
    // prepare color
    pxor    mm0,mm0
    movd    mm7,D [ulLightRGB]
    punpcklbw mm7,mm0
    psllw   mm7,1
    // loop thru rows
    mov     edi,D [_pulLayer]
    mov     ebx,D [_iRowCt]
rowLoop:
    push    ebx
    movd    ebx,mm1         // EBX = slL2Point
    movq    mm3,mm1
    psrlq   mm3,32          // MM3 = 0 | slDL2oDU
    // loop thru pixels in current row
    mov     ecx,D [_iPixCt]
pixLoop:
    // check if pixel need to be drawn
    cmp     ebx,FTOX
    jge     skipPixel
    // calculate intensities and do actual drawing of shadow pixel ARGB
    movd    mm4,ecx
    mov     eax,ebx
    sar     eax,SHIFTX
    and     eax,(SQRTTABLESIZE-1)
    movzx   eax,B aubSqrt[eax]
    mov     ecx,D [_slLightMax]
    cmp     eax,D [_slHotSpot]
    jle     skipInterpolation
    mov     ecx,255
    sub     ecx,eax
    imul    ecx,D [_slLightStep]
skipInterpolation:
    // calculate rgb pixel to add
    movd    mm6,ecx
    punpcklwd mm6,mm6
    punpckldq mm6,mm6
    pmulhw  mm6,mm7
    // add dynamic light pixel to underlying pixel
    movd    mm5,D [edi]
    punpcklbw mm5,mm0
    paddw   mm5,mm6
    packuswb mm5,mm0
    movd    D [edi],mm5
    movd    ecx,mm4
skipPixel:
    // advance to next pixel
    add     edi,4
    movd    eax,mm3
    add     ebx,eax
    paddd   mm3,Q [mmDDL2oDU]
    dec     ecx
    jnz     pixLoop
    // advance to the next row
    pop     ebx
    add     edi,D [_slModulo]
    paddd   mm1,mm2
    paddd   mm2,Q [mmDDL2oDV]
    dec     ebx
    jnz     rowLoop
    emms
  }

#else

  // for each pixel in the shadow map
  for( PIX pixV=0; pixV<_iRowCt; pixV++)
  {
    SLONG slL2Point = _slL2Row;
    SLONG slDL2oDU  = _slDL2oDURow;
    for( PIX pixU=0; pixU<_iPixCt; pixU++)
    {
      // if the point is not masked
      if(slL2Point<FTOX) {
        SLONG sl1oL = (slL2Point>>SHIFTX)&(SQRTTABLESIZE-1);  // and is just for degenerate cases
        sl1oL = aubSqrt[sl1oL];
        SLONG slIntensity = _slLightMax;
        if( sl1oL>_slHotSpot) slIntensity = ((255-sl1oL)*_slLightStep)>>8;
        // add the intensity to the pixel
        AddToCluster( (UBYTE*)_pulLayer, slIntensity/255.0f);
      }
      // advance to next pixel
      _pulLayer++;
      slL2Point +=  slDL2oDU;
      slDL2oDU  += _slDDL2oDU;
    }
    // advance to next row
    _pulLayer    += _slModulo/BYTES_PER_TEXEL;
    _slL2Row     += _slDL2oDV;
    _slDL2oDV    += _slDDL2oDV;
    _slDL2oDURow += _slDDL2oDUoDV;
  }

#endif

}

// add one layer point light without diffusion and with mask
void CLayerMixer::AddAmbientMaskPoint( UBYTE *pubMask, UBYTE ubMask)
{

#if ASMOPT == 1

  // prepare some local variables
  __int64 mmDDL2oDU = _slDDL2oDU;
  __int64 mmDDL2oDV = _slDDL2oDV;
  ULONG ulLightRGB = ByteSwap(lm_colLight);
  _slLightMax<<=7;
  _slLightStep>>=1;

  __asm {
    // prepare interpolants
    movd    mm0,D [_slL2Row]
    movd    mm1,D [_slDL2oDURow]
    psllq   mm1,32
    por     mm1,mm0         // MM1 = slDL2oDURow | slL2Row
    movd    mm0,D [_slDL2oDV]
    movd    mm2,D [_slDDL2oDUoDV]
    psllq   mm2,32
    por     mm2,mm0         // MM2 = slDDL2oDUoDV | slDL2oDV
    // prepare color
    pxor    mm0,mm0         // MM0 = 0 | 0 (for unpacking purposes)
    movd    mm7,D [ulLightRGB]
    punpcklbw mm7,mm0
    psllw   mm7,1
    // loop thru rows
    mov     esi,D [pubMask]
    mov     edi,D [_pulLayer]
    movzx   edx,B [ubMask]
    mov     ebx,D [_iRowCt]
rowLoop:
    push    ebx
    movd    ebx,mm1         // EBX = slL2Point
    movq    mm3,mm1
    psrlq   mm3,32          // MM3 = 0 | slDL2oDU
    // loop thru pixels in current row
    mov     ecx,D [_iPixCt]
pixLoop:
    // check if pixel need to be drawn; i.e. draw if( [esi] & ubMask && (slL2Point<FTOX))
    cmp     ebx,FTOX
    jge     skipPixel
    test    dl,B [esi]
    je      skipPixel
    // calculate intensities and do actual drawing of shadow pixel ARGB
    movd    mm4,ecx
    mov     eax,ebx
    sar     eax,SHIFTX
    and     eax,(SQRTTABLESIZE-1)
    movzx   eax,B aubSqrt[eax]
    mov     ecx,D [_slLightMax]
    cmp     eax,D [_slHotSpot]
    jle     skipInterpolation
    mov     ecx,255
    sub     ecx,eax
    imul    ecx,D [_slLightStep]
skipInterpolation:
    // mix underlaying pixels with the calculated one
    movd    mm6,ecx 
    punpcklwd mm6,mm6
    punpckldq mm6,mm6
    pmulhw  mm6,mm7
    // add light pixel to underlying pixel
    movd    mm5,D [edi]
    punpcklbw mm5,mm0
    paddw   mm5,mm6
    packuswb mm5,mm0
    movd    D [edi],mm5
    movd    ecx,mm4
skipPixel:
    // advance to next pixel
    add     edi,4
    movd    eax,mm3
    add     ebx,eax
    paddd   mm3,Q [mmDDL2oDU]
    rol     dl,1
    adc     esi,0
    dec     ecx
    jnz     pixLoop
    // advance to the next row
    pop     ebx
    add     edi,D [_slModulo]
    paddd   mm1,mm2
    paddd   mm2,Q [mmDDL2oDV]
    dec     ebx
    jnz     rowLoop
    emms
  }

#else

  // for each pixel in the shadow map
  for( PIX pixV=0; pixV<_iRowCt; pixV++)
  {
    SLONG slL2Point = _slL2Row;
    SLONG slDL2oDU  = _slDL2oDURow;
    for( PIX pixU=0; pixU<_iPixCt; pixU++)
    {
      // if the point is not masked
      if( *pubMask&ubMask && (slL2Point<FTOX)) {
        SLONG sl1oL = (slL2Point>>SHIFTX)&(SQRTTABLESIZE-1);  // and is just for degenerate cases
        sl1oL = aubSqrt[sl1oL];
        SLONG slIntensity = _slLightMax;
        if( sl1oL>_slHotSpot) slIntensity = ((255-sl1oL)*_slLightStep)>>8;
        // add the intensity to the pixel
        AddToCluster( (UBYTE*)_pulLayer, slIntensity/255.0f);
      }
      // advance to next pixel
      _pulLayer++;
      slL2Point +=  slDL2oDU;
      slDL2oDU  += _slDDL2oDU;
      ubMask<<=1;
      if( ubMask==0) {
        pubMask++;
        ubMask = 1;
      }
    }
    // advance to next row
    _pulLayer    += _slModulo/BYTES_PER_TEXEL;
    _slL2Row     += _slDL2oDV;
    _slDL2oDV    += _slDDL2oDV;
    _slDL2oDURow += _slDDL2oDUoDV;
  }

#endif

}

// add one layer point light with diffusion and without mask
void CLayerMixer::AddDiffusionPoint(void)
{
  // adjust params for diffusion lighting
  SLONG slMax1oL = MAX_SLONG;
  _slLightStep = FloatToInt(_slLightStep * _fMinLightDistance * _f1oFallOff);
  if( _slLightStep!=0) slMax1oL = (256<<8) / _slLightStep +256;

#if ASMOPT == 1

  // prepare some local variables
  __int64 mmDDL2oDU = _slDDL2oDU;
  __int64 mmDDL2oDV = _slDDL2oDV;
  ULONG ulLightRGB = ByteSwap(lm_colLight);
  _slLightMax<<=7;
  _slLightStep>>=1;

  __asm {
    // prepare interpolants
    movd    mm0,D [_slL2Row]
    movd    mm1,D [_slDL2oDURow]
    psllq   mm1,32
    por     mm1,mm0         // MM1 = slDL2oDURow | slL2Row
    movd    mm0,D [_slDL2oDV]
    movd    mm2,D [_slDDL2oDUoDV]
    psllq   mm2,32
    por     mm2,mm0         // MM2 = slDDL2oDUoDV | slDL2oDV
    // prepare color
    pxor    mm0,mm0
    movd    mm7,D [ulLightRGB]
    punpcklbw mm7,mm0
    psllw   mm7,1
    // loop thru rows
    mov     edi,D [_pulLayer]
    mov     ebx,D [_iRowCt]
rowLoop:
    push    ebx
    movd    ebx,mm1         // EBX = slL2Point
    movq    mm3,mm1
    psrlq   mm3,32          // MM3 = 0 | slDL2oDU
    // loop thru pixels in current row
    mov     ecx,D [_iPixCt]
pixLoop:
    // check if pixel need to be drawn
    cmp     ebx,FTOX
    jge     skipPixel
    // calculate intensities and do actual drawing of shadow pixel ARGB
    movd    mm4,ecx
    mov     eax,ebx
    sar     eax,SHIFTX
    and     eax,(SQRTTABLESIZE-1)
    movzx   eax,W auw1oSqrt[eax*2]
    mov     ecx,D [_slLightMax]
    cmp     eax,D [slMax1oL]
    jge     skipInterpolation
    lea     ecx,[eax-256]
    imul    ecx,D [_slLightStep]
skipInterpolation:
    // calculate rgb pixel to add
    movd    mm6,ecx
    punpcklwd mm6,mm6
    punpckldq mm6,mm6
    pmulhw  mm6,mm7
    // add dynamic light pixel to underlying pixel
    movd    mm5,D [edi]
    punpcklbw mm5,mm0
    paddw   mm5,mm6
    packuswb mm5,mm0
    movd    D [edi],mm5
    movd    ecx,mm4
skipPixel:
    // advance to next pixel
    add     edi,4
    movd    eax,mm3
    add     ebx,eax
    paddd   mm3,Q [mmDDL2oDU]
    dec     ecx
    jnz     pixLoop
    // advance to the next row
    pop     ebx
    add     edi,D [_slModulo]
    paddd   mm1,mm2
    paddd   mm2,Q [mmDDL2oDV]
    dec     ebx
    jnz     rowLoop
    emms
  }

#else

  // for each pixel in the shadow map
  for( PIX pixV=0; pixV<_iRowCt; pixV++)
  {
    SLONG slL2Point = _slL2Row;
    SLONG slDL2oDU  = _slDL2oDURow;
    for( PIX pixU=0; pixU<_iPixCt; pixU++)
    {
      // if the point is not masked
      if((slL2Point<FTOX)) {
        SLONG sl1oL = (slL2Point>>SHIFTX)&(SQRTTABLESIZE-1);  // and is just for degenerate cases
        sl1oL = auw1oSqrt[sl1oL];
        SLONG slIntensity = _slLightMax;
        if( sl1oL<slMax1oL) slIntensity = ((sl1oL-256)*_slLightStep)>>8;
        // add the intensity to the pixel
        AddToCluster( (UBYTE*)_pulLayer, slIntensity/255.0f);
      }
      // advance to next pixel
      _pulLayer++;
      slL2Point +=  slDL2oDU;
      slDL2oDU  += _slDDL2oDU;
    }
    // advance to next row
    _pulLayer    += _slModulo/BYTES_PER_TEXEL;
    _slL2Row     += _slDL2oDV;
    _slDL2oDV    += _slDDL2oDV;
    _slDL2oDURow += _slDDL2oDUoDV;
  }


#endif
}

// add one layer point light with diffusion and mask
void CLayerMixer::AddDiffusionMaskPoint( UBYTE *pubMask, UBYTE ubMask)
{
  // adjust params for diffusion lighting
  SLONG slMax1oL = MAX_SLONG;
  _slLightStep = FloatToInt(_slLightStep * _fMinLightDistance * _f1oFallOff);
  if( _slLightStep!=0) slMax1oL = (256<<8) / _slLightStep +256;

#if ASMOPT == 1

  // prepare some local variables
  __int64 mmDDL2oDU = _slDDL2oDU;
  __int64 mmDDL2oDV = _slDDL2oDV;
  ULONG ulLightRGB = ByteSwap(lm_colLight);
  _slLightMax<<=7;
  _slLightStep>>=1;

  __asm {
    // prepare interpolants
    movd    mm0,D [_slL2Row]
    movd    mm1,D [_slDL2oDURow]
    psllq   mm1,32
    por     mm1,mm0         // MM1 = slDL2oDURow | slL2Row
    movd    mm0,D [_slDL2oDV]
    movd    mm2,D [_slDDL2oDUoDV]
    psllq   mm2,32
    por     mm2,mm0         // MM2 = slDDL2oDUoDV | slDL2oDV
    // prepare color
    pxor    mm0,mm0         // MM0 = 0 | 0 (for unpacking purposes)
    movd    mm7,D [ulLightRGB]
    punpcklbw mm7,mm0
    psllw   mm7,1
    // loop thru rows
    mov     esi,D [pubMask]
    mov     edi,D [_pulLayer]
    movzx   edx,B [ubMask]
    mov     ebx,D [_iRowCt]
rowLoop:
    push    ebx
    movd    ebx,mm1         // EBX = slL2Point
    movq    mm3,mm1
    psrlq   mm3,32          // MM3 = 0 | slDL2oDU
    // loop thru pixels in current row
    mov     ecx,D [_iPixCt]
pixLoop:
    // check if pixel need to be drawn; i.e. draw if( [esi] & ubMask && (slL2Point<FTOX))
    cmp     ebx,FTOX
    jge     skipPixel
    test    dl,B [esi]
    je      skipPixel
    // calculate intensities and do actual drawing of shadow pixel ARGB
    movd    mm4,ecx
    mov     eax,ebx
    sar     eax,SHIFTX
    and     eax,(SQRTTABLESIZE-1)
    movzx   eax,W auw1oSqrt[eax*2]
    mov     ecx,D [_slLightMax]
    cmp     eax,D [slMax1oL]
    jge     skipInterpolation
    lea     ecx,[eax-256]
    imul    ecx,D [_slLightStep]
skipInterpolation:
    // mix underlaying pixels with the calculated one
    movd    mm6,ecx 
    punpcklwd mm6,mm6
    punpckldq mm6,mm6
    pmulhw  mm6,mm7
    // add light pixel to underlying pixel
    movd    mm5,D [edi]
    punpcklbw mm5,mm0
    paddw   mm5,mm6
    packuswb mm5,mm0
    movd    D [edi],mm5
    movd    ecx,mm4
skipPixel:
    // advance to next pixel
    add     edi,4
    movd    eax,mm3
    add     ebx,eax
    paddd   mm3,Q [mmDDL2oDU]
    rol     dl,1
    adc     esi,0
    dec     ecx
    jnz     pixLoop
    // advance to the next row
    pop     ebx
    add     edi,D [_slModulo]
    paddd   mm1,mm2
    paddd   mm2,Q [mmDDL2oDV]
    dec     ebx
    jnz     rowLoop
    emms
  }

#else


  // for each pixel in the shadow map
  for( PIX pixV=0; pixV<_iRowCt; pixV++)
  {
    SLONG slL2Point = _slL2Row;
    SLONG slDL2oDU  = _slDL2oDURow;
    for( PIX pixU=0; pixU<_iPixCt; pixU++)
    {
      // if the point is not masked
      if( *pubMask&ubMask && (slL2Point<FTOX)) {
        SLONG sl1oL = (slL2Point>>SHIFTX)&(SQRTTABLESIZE-1);  // and is just for degenerate cases
        sl1oL = auw1oSqrt[sl1oL];
        SLONG slIntensity = _slLightMax;
        if( sl1oL<slMax1oL) slIntensity = ((sl1oL-256)*_slLightStep)>>8;
        // add the intensity to the pixel
        AddToCluster( (UBYTE*)_pulLayer, slIntensity/255.0f);
      } 
      // advance to next pixel
      _pulLayer++;
      slL2Point +=  slDL2oDU;
      slDL2oDU  += _slDDL2oDU;
      ubMask<<=1;
      if( ubMask==0) {
        pubMask++;
        ubMask = 1;
      }
    }
    // advance to next row
    _pulLayer    += _slModulo/BYTES_PER_TEXEL;
    _slL2Row     += _slDL2oDV;
    _slDL2oDV    += _slDDL2oDV;
    _slDL2oDURow += _slDDL2oDUoDV;
  }

#endif

}

// prepares point light that creates layer (returns TRUE if there is infulence)
BOOL CLayerMixer::PrepareOneLayerPoint( CBrushShadowLayer *pbsl, BOOL bNoMask)
{
  // determine light infulence dimensions
  _iPixCt = pbsl->bsl_pixSizeU >>lm_iMipShift;
  _iRowCt = pbsl->bsl_pixSizeV >>lm_iMipShift;
  PIX pixMinU = pbsl->bsl_pixMinU >>lm_iMipShift;
  PIX pixMinV = pbsl->bsl_pixMinV >>lm_iMipShift;
  // clamp influence to polygon size
  if( (pixMinU+_iPixCt) > lm_pixPolygonSizeU && bNoMask) _iPixCt = lm_pixPolygonSizeU-pixMinU;
  if( (pixMinV+_iRowCt) > lm_pixPolygonSizeV)            _iRowCt = lm_pixPolygonSizeV-pixMinV;
  _slModulo = (lm_pixCanvasSizeU-_iPixCt) *BYTES_PER_TEXEL;
  _pulLayer = lm_pulShadowMap + (pixMinV*lm_pixCanvasSizeU)+pixMinU;
  ASSERT( pixMinU>=0 && pixMinU<lm_pixCanvasSizeU && pixMinV>=0 && pixMinV<lm_pixCanvasSizeV);

  // get the light source properties of the layer
  lm_plsLight = pbsl->bsl_plsLightSource;
  _vLight = &lm_plsLight->ls_penEntity->GetPlacement().pl_PositionVector;
  _fMinLightDistance = lm_pbpoPolygon->bpo_pbplPlane->bpl_plAbsolute.PointDistance(*_vLight);
  _f1oFallOff   = 1.0f / lm_plsLight->ls_rFallOff;
  _ulLightFlags = lm_plsLight->ls_ulFlags;
  _ulPolyFlags  = lm_pbpoPolygon->bpo_ulFlags;
  lm_colLight   = lm_plsLight->GetLightColor();
  pbsl->bsl_colLastAnim = lm_colLight;

  // if there is no influence, do nothing
  if( (pbsl->bsl_pixSizeU>>lm_iMipShift)==0 || (pbsl->bsl_pixSizeV>>lm_iMipShift)==0
    || _iPixCt<=0 || _iRowCt<=0) return FALSE;

  // adjust for sector ambient
  if( _ulLightFlags&LSF_SUBSTRACTSECTORAMBIENT) {
    COLOR colAmbient = lm_pbpoPolygon->bpo_pbscSector->bsc_colAmbient;
    IncrementByteWithClip( ((UBYTE*)&lm_colLight)[1], -((UBYTE*)&colAmbient)[1]);
    IncrementByteWithClip( ((UBYTE*)&lm_colLight)[2], -((UBYTE*)&colAmbient)[2]);
    IncrementByteWithClip( ((UBYTE*)&lm_colLight)[3], -((UBYTE*)&colAmbient)[3]);
    if( _ulPolyFlags&BPOF_HASDIRECTIONALAMBIENT)
    { // find directional layers for each shadow layer
      FOREACHINLIST( CBrushShadowLayer, bsl_lnInShadowMap, lm_pbsmShadowMap->bsm_lhLayers, itbsl)
      { // loop thru layers
        CBrushShadowLayer &bsl = *itbsl;
        if( bsl.bsl_plsLightSource->ls_ulFlags&LSF_DIRECTIONAL)
        { // skip if no ambient color
          colAmbient = bsl.bsl_plsLightSource->ls_colAmbient & 0xFFFFFF00;
          if( IsBlack(colAmbient)) continue;
          // substract ambient
          IncrementByteWithClip( ((UBYTE*)&lm_colLight)[1], -((UBYTE*)&colAmbient)[1]);
          IncrementByteWithClip( ((UBYTE*)&lm_colLight)[2], -((UBYTE*)&colAmbient)[2]);
          IncrementByteWithClip( ((UBYTE*)&lm_colLight)[3], -((UBYTE*)&colAmbient)[3]);
        }
      }
    }
  }

  // prepare intermediate light interpolants
  FLOAT3D v00 = (lm_vO+lm_vStepU*pixMinU + lm_vStepV*pixMinV) - *_vLight;
  FLOAT fFactor = FTOX * _f1oFallOff*_f1oFallOff;
  FLOAT fL2Row  = v00%v00;
  FLOAT fDDL2oDU    = lm_vStepU%lm_vStepU;
  FLOAT fDDL2oDV    = lm_vStepV%lm_vStepV;
  FLOAT fDDL2oDUoDV = lm_vStepU%lm_vStepV;
  FLOAT fDL2oDURow  = fDDL2oDU + 2*(lm_vStepU%v00);
  FLOAT fDL2oDV     = fDDL2oDV + 2*(lm_vStepV%v00);
  //_v00 = v00;

#if ASMOPT == 1
  __asm {
    fld     D [fDDL2oDU]
    fadd    D [fDDL2oDU]
    fld     D [fDDL2oDV]
    fadd    D [fDDL2oDV]
    fld     D [fDDL2oDUoDV]
    fadd    D [fDDL2oDUoDV]
    // st0=2*fDDL2oDUoDV, st1=2*fDDL2oDV, st2=2*fDDL2oDU
    fld     D [fL2Row]
    fmul    D [fFactor]
    fld     D [fDL2oDURow]
    fmul    D [fFactor]
    fld     D [fDL2oDV]
    fmul    D [fFactor]
    // st0=fDL2oDV*fFactor, st1=fDL2oDURow*fFactor, st2=fL2Row*fFactor,
    // st3=2*fDDL2oDUoDV, st4=2*fDDL2oDV, st5=2*fDDL2oDU
    fld     D [fFactor]
    fmul    st(4),st(0)
    fmul    st(5),st(0)
    fmulp   st(6),st(0)
    fistp   D [_slDL2oDV]
    fistp   D [_slDL2oDURow]
    fistp   D [_slL2Row]
    fistp   D [_slDDL2oDUoDV]
    fistp   D [_slDDL2oDV]
    fistp   D [_slDDL2oDU]
  }
#else
  fDDL2oDU     *= 2;
  fDDL2oDV     *= 2;
  fDDL2oDUoDV  *= 2;
  _slL2Row      = FloatToInt( fL2Row      * fFactor);
  _slDDL2oDU    = FloatToInt( fDDL2oDU    * fFactor);
  _slDDL2oDV    = FloatToInt( fDDL2oDV    * fFactor);
  _slDDL2oDUoDV = FloatToInt( fDDL2oDUoDV * fFactor);
  _slDL2oDURow  = FloatToInt( fDL2oDURow  * fFactor);
  _slDL2oDV     = FloatToInt( fDL2oDV     * fFactor);
#endif

  // prepare final light interpolants
  _slLightMax  = 255;
  _slHotSpot   = FloatToInt( 255.0f * lm_plsLight->ls_rHotSpot * _f1oFallOff);
  _slLightStep = FloatToInt( 65535.0f / (255.0f - _slHotSpot));
  // dark light inverts parameters
  if( _ulLightFlags & LSF_DARKLIGHT) {
    _slLightMax  = -_slLightMax;
    _slLightStep = -_slLightStep;
  }

  // saturate light color
  lm_colLight = AdjustColor( lm_colLight, _slShdHueShift, _slShdSaturation);
  // all done
  return TRUE;
}


// add one layer to the shadow map (pubMask=NULL for no mask)
void CLayerMixer::AddOneLayerPoint( CBrushShadowLayer *pbsl, UBYTE *pubMask, UBYTE ubMask)
{
  // try to prepare layer for this point light
  _pfWorldEditingProfile.StartTimer( CWorldEditingProfile::PTI_ADDONELAYERPOINT);
  if( !PrepareOneLayerPoint( pbsl, pubMask==NULL)) {
    _pfWorldEditingProfile.StopTimer( CWorldEditingProfile::PTI_ADDONELAYERPOINT);
    return;
  }

  // determine diffusion presence and corresponding routine
  BOOL bDiffusion = (_ulLightFlags&LSF_DIFFUSION) && !(_ulPolyFlags&BPOF_NOPLANEDIFFUSION);
  // masked or non-masked?
  if( pubMask==NULL) {
    // non-masked
    if( !lm_bDynamic && bDiffusion) {
      // non-masked diffusion
      AddDiffusionPoint();
    } else {
      // non-masked ambient
      AddAmbientPoint();
    }
  } else {
    // masked
    if( bDiffusion) {
      // masked diffusion
      AddDiffusionMaskPoint( pubMask, ubMask);
    } else {
      AddAmbientMaskPoint( pubMask, ubMask);
    }
  }

  // all done
  _pfWorldEditingProfile.StopTimer(CWorldEditingProfile::PTI_ADDONELAYERPOINT);
}



// apply gradient to layer
void CLayerMixer::AddOneLayerGradient( CGradientParameters &gp)
{
  // convert gradient parameters for plane
  ASSERT( Abs(gp.gp_fH1-gp.gp_fH0)>0.0001f);
  FLOAT f1oDH = 1.0f / (gp.gp_fH1-gp.gp_fH0);
  FLOAT fGr00 = (lm_vO % gp.gp_vGradientDir - gp.gp_fH0) *f1oDH;
  FLOAT fDGroDI = (lm_vStepU % gp.gp_vGradientDir) *f1oDH;
  FLOAT fDGroDJ = (lm_vStepV % gp.gp_vGradientDir) *f1oDH;
  fDGroDI += fDGroDI/lm_pixPolygonSizeU;
  fDGroDJ += fDGroDJ/lm_pixPolygonSizeV;
  SLONG fixDGroDI = FloatToInt(fDGroDI*32767.0f); // 16:15
  SLONG fixDGroDJ = FloatToInt(fDGroDJ*32767.0f); // 16:15
  COLOR col0 = gp.gp_col0;
  COLOR col1 = gp.gp_col1;
  _pulLayer  = lm_pulShadowMap;
  FLOAT fStart = Clamp( fGr00-(fDGroDJ+fDGroDI)*0.5f, 0.0f, 1.0f);

#if ASMOPT == 1

  __int64 mmRowAdv;
  SLONG fixGRow  = (fGr00-(fDGroDJ+fDGroDI)*0.5f)*32767.0f; // 16:15
  SLONG slModulo = (lm_pixCanvasSizeU-lm_pixPolygonSizeU) *BYTES_PER_TEXEL;
  COLOR colStart = LerpColor( col0, col1, fStart);
  INDEX ctCols = lm_pixPolygonSizeU;
  INDEX ctRows = lm_pixPolygonSizeV;
  BOOL  bDarkLight = gp.gp_bDark;
  
  __asm {
    mov     eax,D [colStart]
    mov     ebx,D [col0]
    mov     edx,D [col1]
    bswap   eax
    bswap   ebx
    bswap   edx
    movd    mm1,eax
    movd    mm2,ebx
    movd    mm3,edx
    punpcklbw mm1,mm1
    punpcklbw mm2,mm2
    punpcklbw mm3,mm3
    psrlw   mm1,2
    psrlw   mm2,1
    psrlw   mm3,1
    // eventually adjust for dark light
    cmp     D [bDarkLight],0
    je      skipDark
    pcmpeqd mm0,mm0
    pxor    mm1,mm0
    pxor    mm2,mm0
    pxor    mm3,mm0
    psubw   mm1,mm0
    psubw   mm2,mm0
    psubw   mm3,mm0
skipDark:
    // keep colors
    movq    mm0,mm2
    movq    mm7,mm3
    // find row and column advancers
    psubw   mm3,mm2
    movq    mm2,mm3
    movd    mm5,D [fixDGroDI]
    movd    mm6,D [fixDGroDJ]
    punpcklwd mm5,mm5
    punpcklwd mm6,mm6
    punpckldq mm5,mm5
    punpckldq mm6,mm6
    pmulhw  mm2,mm5           // column color advancer (8:6)
    pmulhw  mm3,mm6           // row color advancer    (8:6)
    movq    Q [mmRowAdv],mm3 

    // prepare starting variables
    movq    mm5,mm0
    movq    mm6,mm7
    psraw   mm5,1   // starting color
    psraw   mm6,1   // ending color
    pxor    mm0,mm0
    mov     esi,D [fixGRow]
    mov     edi,D [_pulLayer]
    mov     edx,D [ctRows]
rowLoop:
    mov     ebx,esi
    movq    mm4,mm1
    mov     ecx,D [ctCols]
pixLoop:
    // add or substract light pixel to underlying pixel
    movq    mm7,mm4
    psraw   mm7,6
    movd    mm3,D [edi]
    punpcklbw mm3,mm0
    paddw   mm3,mm7
    packuswb mm3,mm0
    movd    D [edi],mm3

    // advance to next pixel
    add     ebx,D [fixDGroDI]
    cmp     ebx,0x8000
    ja      pixClamp
    paddw   mm4,mm2
    add     edi,4
    dec     ecx
    jnz     pixLoop
    jmp     pixDone
pixClamp:
    movq    mm4,mm6
    jg      pixNext
    movq    mm4,mm5
pixNext:
    add     edi,4
    dec     ecx
    jnz     pixLoop
pixDone:

    // advance to next row
    add     esi,D [fixDGroDJ]
    cmp     esi,0x8000
    ja      rowClamp
    paddw   mm1,Q [mmRowAdv]
    add     edi,D [slModulo]
    dec     edx
    jnz     rowLoop
    jmp     rowDone
rowClamp:
    movq    mm1,mm6
    jg      rowNext
    movq    mm1,mm5
rowNext:
    add     edi,D [slModulo]
    dec     edx
    jnz     rowLoop
rowDone:
    emms
  }

#else

  // well, make gradient ...
  SLONG slR0=0,slG0=0,slB0=0;
  SLONG slR1=0,slG1=0,slB1=0;
  ColorToRGB( col0, (UBYTE&)slR0,(UBYTE&)slG0,(UBYTE&)slB0);
  ColorToRGB( col1, (UBYTE&)slR1,(UBYTE&)slG1,(UBYTE&)slB1);
  if( gp.gp_bDark) {
    slR0 = -slR0;  slG0 = -slG0;  slB0 = -slB0;
    slR1 = -slR1;  slG1 = -slG1;  slB1 = -slB1;
  }
  fixDGroDI >>= 1; // 16:14
  fixDGroDJ >>= 1; // 16:14
  SWORD fixRrow = Lerp( slR0,slR1,fStart) <<6; // 8:6
  SWORD fixGrow = Lerp( slG0,slG1,fStart) <<6; // 8:6
  SWORD fixBrow = Lerp( slB0,slB1,fStart) <<6; // 8:6
  SWORD fixDRoDI = ((slR1-slR0)*fixDGroDI)>>8;
  SWORD fixDGoDI = ((slG1-slG0)*fixDGroDI)>>8;
  SWORD fixDBoDI = ((slB1-slB0)*fixDGroDI)>>8;
  SWORD fixDRoDJ = ((slR1-slR0)*fixDGroDJ)>>8;
  SWORD fixDGoDJ = ((slG1-slG0)*fixDGroDJ)>>8;
  SWORD fixDBoDJ = ((slB1-slB0)*fixDGroDJ)>>8;

  // loop it, baby
  FLOAT fGrRow = fGr00 - (fDGroDJ+fDGroDI)*0.5f;
  PIX   pixOffset = 0;
  PIX   pixModulo = lm_pixCanvasSizeU-lm_pixPolygonSizeU;
  for( INDEX j=0; j<lm_pixPolygonSizeV; j++)
  { // prepare row
    FLOAT fGrCol  = fGrRow;
    SWORD fixRcol = fixRrow;
    SWORD fixGcol = fixGrow;
    SWORD fixBcol = fixBrow;
    for( INDEX i=0; i<lm_pixPolygonSizeU; i++)
    { // loop pixels
      SLONG slR = Clamp( fixRcol>>6, -255, +255);
      SLONG slG = Clamp( fixGcol>>6, -255, +255);
      SLONG slB = Clamp( fixBcol>>6, -255, +255);
      IncrementByteWithClip( ((UBYTE*)&_pulLayer[pixOffset])[0], slR);
      IncrementByteWithClip( ((UBYTE*)&_pulLayer[pixOffset])[1], slG);
      IncrementByteWithClip( ((UBYTE*)&_pulLayer[pixOffset])[2], slB);
      // advance to next pixel
      fGrCol += fDGroDI;
      pixOffset++;
      if( fGrCol<0) {
        fixRcol = slR0<<6;
        fixGcol = slG0<<6;
        fixBcol = slB0<<6;
      } else if( fGrCol>1) {
        fixRcol = slR1<<6;
        fixGcol = slG1<<6;
        fixBcol = slB1<<6;
      } else {
        fixRcol += fixDRoDI;
        fixGcol += fixDGoDI;
        fixBcol += fixDBoDI;
      }
    }
    // advance to next row
    fGrRow += fDGroDJ;
    pixOffset += pixModulo;
    if( fGrRow<0) {
      fixRrow = slR0<<6;
      fixGrow = slG0<<6;
      fixBrow = slB0<<6;
    } else if( fGrRow>1) {
      fixRrow = slR1<<6;
      fixGrow = slG1<<6;
      fixBrow = slB1<<6;
    } else {
      fixRrow += fixDRoDJ;
      fixGrow += fixDGoDJ;
      fixBrow += fixDBoDJ;
    }
  }

#endif

}



// apply directional light or ambient to layer
void CLayerMixer::AddDirectional(void)
{

#if ASMOPT == 1

  ULONG ulLight = ByteSwap( lm_colLight);
  __asm {
    // prepare pointers and variables
    mov     edi,D [_pulLayer]
    mov     ebx,D [_iRowCt]
    movd    mm6,D [ulLight]
    punpckldq mm6,mm6
rowLoop: 
    mov     ecx,D [_iPixCt]
    shr     ecx,1
    jz      pixRest
pixLoop:
    // mix underlaying pixels with the constant color pixel
    movq    mm5,Q [edi]
    paddusb mm5,mm6
    movq    Q [edi],mm5
    // advance to next pixel
    add     edi,8
    dec     ecx
    jnz     pixLoop
pixRest:
    test    D [_iPixCt],1
    jz      rowNext
    movd    mm5,D [edi]
    paddusb mm5,mm6
    movd    D [edi],mm5
    add     edi,4
rowNext:
    // advance to the next row
    add     edi,D [_slModulo]
    dec     ebx
    jnz     rowLoop
    emms
  }

#else

  // for each pixel in the shadow map
  for( PIX pixV=0; pixV<_iRowCt; pixV++) {
    for( PIX pixU=0; pixU<_iPixCt; pixU++) {
      // add the intensity to the pixel
      AddToCluster( (UBYTE*)_pulLayer);
      _pulLayer++; // go to the next pixel
    } // go to the next row
    _pulLayer += _slModulo/BYTES_PER_TEXEL;
  }

#endif

}

// apply directional light thru mask to layer
void CLayerMixer::AddMaskDirectional( UBYTE *pubMask, UBYTE ubMask)
{

#if ASMOPT == 1

  // prepare some local variables
  ULONG ulLight = ByteSwap( lm_colLight);
  __asm {
    // prepare pointers and variables
    movzx   edx,B [ubMask]
    mov     esi,D [pubMask]
    mov     edi,D [_pulLayer]
    mov     ebx,D [_iRowCt]
    movd    mm6,D [ulLight]
rowLoop:
    mov     ecx,D [_iPixCt]
pixLoop:
    // mix underlaying pixels with the constant light color if not shaded
    test    dl,B [esi]
    jz      skipLight
    movd    mm5,D [edi]
    paddusb mm5,mm6
    movd    D [edi],mm5
skipLight:
    // advance to next pixel
    add     edi,4
    rol     dl,1
    adc     esi,0
    dec     ecx
    jnz     pixLoop
    // advance to the next row
    add     edi,D [_slModulo]
    dec     ebx
    jnz     rowLoop
    emms
  }

#else

  // for each pixel in the shadow map
  for( PIX pixV=0; pixV<_iRowCt; pixV++) {
    for( PIX pixU=0; pixU<_iPixCt; pixU++) {
      // if the point is not masked
      if( *pubMask&ubMask) {
        // add the intensity to the pixel
        AddToCluster( (UBYTE*)_pulLayer);
      } // go to the next pixel
      _pulLayer++;
      ubMask<<=1;
      if( ubMask==0) {
        pubMask ++;
        ubMask = 1;
      }
    } // go to the next row
    _pulLayer += _slModulo/BYTES_PER_TEXEL;
  }

#endif

}


// apply directional light to layer
// (pubMask=NULL for no mask, ubMask = 0xFF for full mask)
void CLayerMixer::AddOneLayerDirectional( CBrushShadowLayer *pbsl, UBYTE *pubMask, UBYTE ubMask)
{
  // only if there is color light (ambient is added at initial fill)
  if( !(lm_pbpoPolygon->bpo_ulFlags&BPOF_HASDIRECTIONALLIGHT)) return;
  _pfWorldEditingProfile.StartTimer(CWorldEditingProfile::PTI_ADDONELAYERDIRECTIONAL);

  // determine light influence dimensions
  _iPixCt = pbsl->bsl_pixSizeU >>lm_iMipShift;
  _iRowCt = pbsl->bsl_pixSizeV >>lm_iMipShift;
  PIX pixMinU = pbsl->bsl_pixMinU >>lm_iMipShift;
  PIX pixMinV = pbsl->bsl_pixMinV >>lm_iMipShift;
  ASSERT( pixMinU==0 && pixMinV==0);
  // clamp influence to polygon size
  if( _iPixCt > lm_pixPolygonSizeU && pubMask==NULL) _iPixCt = lm_pixPolygonSizeU;
  if( _iRowCt > lm_pixPolygonSizeV)                  _iRowCt = lm_pixPolygonSizeV;
  _slModulo = (lm_pixCanvasSizeU-_iPixCt) *BYTES_PER_TEXEL;
  _pulLayer = lm_pulShadowMap;

  // if there is no influence, do nothing
  if( (pbsl->bsl_pixSizeU>>lm_iMipShift)==0 || (pbsl->bsl_pixSizeV>>lm_iMipShift)==0
    || _iPixCt<=0 || _iRowCt<=0) {
    _pfWorldEditingProfile.StopTimer(CWorldEditingProfile::PTI_ADDONELAYERDIRECTIONAL);
    return;
  }

  // get the light source of the layer
  lm_plsLight = pbsl->bsl_plsLightSource;
  const FLOAT3D &vLight = lm_plsLight->ls_penEntity->GetPlacement().pl_PositionVector;
  AnglesToDirectionVector( lm_plsLight->ls_penEntity->GetPlacement().pl_OrientationAngle,
                           lm_vLightDirection);
  // calculate intensity
  FLOAT fIntensity = 1.0f;
  if( !(lm_pbpoPolygon->bpo_ulFlags&BPOF_NOPLANEDIFFUSION)) {
    fIntensity = -((lm_pbpoPolygon->bpo_pbplPlane->bpl_plAbsolute)%lm_vLightDirection);
    fIntensity = ClampDn( fIntensity, 0.0f);
  }
  // calculate light color and ambient
  lm_colLight = lm_plsLight->GetLightColor();
  pbsl->bsl_colLastAnim = lm_colLight;
  ULONG ulIntensity = NormFloatToByte(fIntensity);
  ulIntensity = (ulIntensity<<CT_RSHIFT)|(ulIntensity<<CT_GSHIFT)|(ulIntensity<<CT_BSHIFT);
  lm_colLight = MulColors(   lm_colLight, ulIntensity);
  lm_colLight = AdjustColor( lm_colLight, _slShdHueShift, _slShdSaturation);

  // masked or non-masked?
  if( pubMask==NULL) {
    // non-masked
    AddDirectional();
  } else {
    // masked
    AddMaskDirectional( pubMask, ubMask);
  }

  // all done
  _pfWorldEditingProfile.StopTimer(CWorldEditingProfile::PTI_ADDONELAYERDIRECTIONAL);
}



// clamper helper
static INDEX GetDither(void)
{
  shd_iDithering = Clamp( shd_iDithering, 0L, 5L);
  INDEX iDither  = shd_iDithering;
  if( iDither>2) iDither++;
  return iDither;
}

template<typename T>
void SwapEndianess(T &var) {
  char *varArray = reinterpret_cast<char *>(&var);
  for (long i = 0; i < static_cast<long>(sizeof(var) / 2); i++)
    std::swap(varArray[sizeof(var) - 1 - i], varArray[i]);
}

// mix one mip-map
void CLayerMixer::MixOneMipmap(CBrushShadowMap *pbsm, INDEX iMipmap)
{
  // remember general data
  CalculateData( pbsm, iMipmap);
  const BOOL bDynamicOnly = lm_pbpoPolygon->bpo_ulFlags&BPOF_DYNAMICLIGHTSONLY;

#if WITH_BOUND_CHECK
  g_pbsm = pbsm;
  lm_pulShadowMap = (ULONG *) AllocMemory(pbsm->sm_slMemoryUsed * 2);
  memset(lm_pulShadowMap, 0, pbsm->sm_slMemoryUsed * 2);
#endif

  // fill with sector ambient
  _pfWorldEditingProfile.StartTimer(CWorldEditingProfile::PTI_AMBIENTFILL);

  // eventually add ambient component of all directional layers that might contribute
  COLOR colAmbient = 0x80808000UL; // overide ambient light color for dynamic lights only
  if( !bDynamicOnly) {
    colAmbient = AdjustColor( lm_pbpoPolygon->bpo_pbscSector->bsc_colAmbient, _slShdHueShift, _slShdSaturation);
    if( lm_pbpoPolygon->bpo_ulFlags&BPOF_HASDIRECTIONALAMBIENT) {
      {FOREACHINLIST( CBrushShadowLayer, bsl_lnInShadowMap, lm_pbsmShadowMap->bsm_lhLayers, itbsl) {
        CBrushShadowLayer &bsl = *itbsl;
        CLightSource &ls = *bsl.bsl_plsLightSource;
        ASSERT( &ls!=NULL); if( &ls==NULL) continue; // safety check
        if( !(ls.ls_ulFlags&LSF_DIRECTIONAL)) continue;  // skip non-directional layers
        COLOR col = AdjustColor( ls.GetLightAmbient(), _slShdHueShift, _slShdSaturation);
        colAmbient = AddColors( colAmbient, col);
      }}
    }
  } // set initial color
//  __asm {
//    cld
//    mov     ebx,D [this]
//    mov     ecx,D [ebx].lm_pixCanvasSizeU
//    imul    ecx,D [ebx].lm_pixCanvasSizeV
//    mov     edi,D [ebx].lm_pulShadowMap
//    mov     eax,D [colAmbient]
//    bswap   eax
//    rep     stosd
//  }
  uint32_t ecx = lm_pixCanvasSizeU * lm_pixCanvasSizeV;
  ULONG *edi = lm_pulShadowMap;
  uint32_t eax = colAmbient;
  SwapEndianess(eax);
  for (uint32_t i = 0; i < ecx; i++) {
    *edi = eax;
    edi++;
  }
  _pfWorldEditingProfile.StopTimer(CWorldEditingProfile::PTI_AMBIENTFILL);

  // find gradient layer
  CGradientParameters gpGradient;
  BOOL  bHasGradient   = FALSE;
  ULONG ulGradientType = lm_pbpoPolygon->bpo_bppProperties.bpp_ubGradientType;
  if( ulGradientType>0) {
    CEntity *pen = lm_pbpoPolygon->bpo_pbscSector->bsc_pbmBrushMip->bm_pbrBrush->br_penEntity;
    if( pen!=NULL) bHasGradient = pen->GetGradient( ulGradientType, gpGradient);
  }
  // add gradient if gradient is light
  if( bHasGradient && !gpGradient.gp_bDark) AddOneLayerGradient( gpGradient);

  // for each shadow layer
  lm_pbsmShadowMap->sm_ulFlags &= ~SMF_ANIMATINGLIGHTS;
  {FORDELETELIST( CBrushShadowLayer, bsl_lnInShadowMap, lm_pbsmShadowMap->bsm_lhLayers, itbsl)
  {
    CBrushShadowLayer &bsl = *itbsl;
    CLightSource &ls = *bsl.bsl_plsLightSource;
    ASSERT( &ls!=NULL); if( &ls==NULL) continue; // safety check

    // skip if should not be applied
    if( (bDynamicOnly && !(ls.ls_ulFlags&LSF_NONPERSISTENT)) || ls.ls_ulFlags&LSF_DYNAMIC) continue;

    // set corresponding shadowmap flag if this is an animating light
    if( ls.ls_paoLightAnimation!=NULL) lm_pbsmShadowMap->sm_ulFlags |= SMF_ANIMATINGLIGHTS;

    // if the layer is calculated
    if( bsl.bsl_pubLayer!=NULL)
    {
      UBYTE *pub;
      UBYTE ubMask;
      FindLayerMipmap( itbsl, pub, ubMask);
      // add the layer to the shadow map with masking
      if( ls.ls_ulFlags&LSF_DIRECTIONAL) {
        AddOneLayerDirectional( itbsl, pub, ubMask);
      } else {
        AddOneLayerPoint( itbsl, pub, ubMask);
      }
    }
    // if the layer is all light
    else if( !(bsl.bsl_ulFlags&BSLF_CALCULATED) || (bsl.bsl_ulFlags&BSLF_ALLLIGHT))
    {
      // add the layer to the shadow map without masking
      if( ls.ls_ulFlags&LSF_DIRECTIONAL) {
        AddOneLayerDirectional( itbsl, NULL);
      } else {
        AddOneLayerPoint( itbsl, NULL);
      }
    }
  }}

  // if gradient is dark, substract gradient
  if( bHasGradient && gpGradient.gp_bDark) AddOneLayerGradient( gpGradient);

  // do eventual filtering of shadow layer
  shd_iFiltering = Clamp( shd_iFiltering, 0L, +6L);
  if( shd_iFiltering>0) {
    FilterBitmap( shd_iFiltering, lm_pulShadowMap, lm_pulShadowMap,
                  lm_pixPolygonSizeU, lm_pixPolygonSizeV, lm_pixCanvasSizeU, lm_pixCanvasSizeV);
  }
  // do eventual dithering of shadow layer
  const INDEX iDither = GetDither();
  if( !(_pGfx->gl_ulFlags&GLF_32BITTEXTURES)) shd_bFineQuality = FALSE;
  if( iDither && !(shd_bFineQuality)) {
    DitherBitmap( iDither, lm_pulShadowMap, lm_pulShadowMap,
                  lm_pixPolygonSizeU, lm_pixPolygonSizeV, lm_pixCanvasSizeU, lm_pixCanvasSizeV);
  }

#if WITH_BOUND_CHECK
  for (int i = g_pbsm->sm_slMemoryUsed; i < g_pbsm->sm_slMemoryUsed * 2; i++) {
    if (((UBYTE *) lm_pulShadowMap)[i]) {
      FatalError("Invalid write");
    }
  }
  FreeMemory(lm_pulShadowMap);
#endif
}



// copy from static shadow map to dynamic layer
__forceinline void CLayerMixer::CopyShadowLayer(void)
{
  memcpy(lm_pulShadowMap, lm_pulStaticShadowMap, lm_pixCanvasSizeU * lm_pixCanvasSizeV * 4);
  return;
//  __asm {
//    cld
//    mov     ebx,D [this]
//    mov     ecx,D [ebx].lm_pixCanvasSizeU
//    imul    ecx,D [ebx].lm_pixCanvasSizeV
//    mov     esi,D [ebx].lm_pulStaticShadowMap
//    mov     edi,D [ebx].lm_pulShadowMap
//    rep     movsd
//  }
}


// copy from static shadow map to dynamic layer
__forceinline void CLayerMixer::FillShadowLayer( COLOR col)
{
  PIX ecx = lm_pixCanvasSizeU * lm_pixCanvasSizeV;
  ULONG *edi = lm_pulShadowMap;
  SwapEndianess(col);
  for (uint32_t i = 0; i < ecx; i++) {
    *edi++ = col;
  }

//  __asm {
//    cld
//    mov     ebx,D [this]
//    mov     ecx,D [ebx].lm_pixCanvasSizeU
//    imul    ecx,D [ebx].lm_pixCanvasSizeV
//    mov     edi,D [ebx].lm_pulShadowMap
//    mov     eax,D [col]
//    bswap   eax   // convert to R,G,B,A memory format!
//    rep     stosd
//  }
}


// mix dynamic lights
void CLayerMixer::MixOneMipmapDynamic( CBrushShadowMap *pbsm, INDEX iMipmap)
{
  // remember general data
  CalculateData( pbsm, iMipmap);
  // if static shadow map is all flat
  if( pbsm->sm_pulCachedShadowMap==&pbsm->sm_colFlat) {
    // just fill dynamic shadow map with flat color
    FillShadowLayer( pbsm->sm_colFlat);
  } // if not flat
  else {
    // copy static layer
    CopyShadowLayer();
  }

  // for each shadow layer
  {FORDELETELIST( CBrushShadowLayer, bsl_lnInShadowMap, lm_pbsmShadowMap->bsm_lhLayers, itbsl)
  { // the layer's light source must be valid
    CBrushShadowLayer &bsl = *itbsl;
    CLightSource &ls = *bsl.bsl_plsLightSource;
    ASSERT( &ls!=NULL);
    if( !(ls.ls_ulFlags&LSF_DYNAMIC)) continue;
    COLOR colLight = ls.GetLightColor() & ~CT_AMASK;
    if( IsBlack(colLight)) continue;
    // apply one layer
    colLight = AdjustColor( colLight, _slShdHueShift, _slShdSaturation);
    AddOneLayerPoint( itbsl, NULL);
  }}
}


// constructor
CLayerMixer::CLayerMixer( CBrushShadowMap *pbsm, INDEX iFirstMip, INDEX iLastMip, BOOL bDynamic)
{
  lm_bDynamic = bDynamic;
  if( bDynamic) {
    // check dynamic layers for complete blackness
    BOOL bAllBlack = TRUE;
    pbsm->sm_ulFlags &= ~SMF_DYNAMICBLACK;
    {FORDELETELIST( CBrushShadowLayer, bsl_lnInShadowMap, pbsm->bsm_lhLayers, itbsl) {
      CLightSource &ls = *itbsl->bsl_plsLightSource;
      ASSERT( &ls!=NULL);
      if( !(ls.ls_ulFlags&LSF_DYNAMIC)) continue;
      COLOR colLight = ls.GetLightColor() & ~CT_AMASK;
      itbsl->bsl_colLastAnim = colLight;
      if( !IsBlack(colLight)) bAllBlack = FALSE; // must continue because of layer info update (light anim and such stuff)
    }}
    // skip mixing if dynamic layers were all black
    if( bAllBlack) {
      pbsm->sm_ulFlags |= SMF_DYNAMICBLACK;
      return;
    }
    // need to mix in
    for( INDEX iMipmap=iFirstMip; iMipmap<=iLastMip; iMipmap++) MixOneMipmapDynamic( pbsm, iMipmap);
  }
  // mix static layers
  else {
    for( INDEX iMipmap=iFirstMip; iMipmap<=iLastMip; iMipmap++) MixOneMipmap( pbsm, iMipmap);
  }
}


// mix all layers into cached shadow map
void CBrushShadowMap::MixLayers( INDEX iFirstMip, INDEX iLastMip, BOOL bDynamic/*=FALSE*/)
{
  _sfStats.StartTimer( CStatForm::STI_SHADOWUPDATE);
  _pfWorldEditingProfile.StartTimer( CWorldEditingProfile::PTI_MIXLAYERS);
  // mix the layers with a shadow mixer
  CLayerMixer lmMixer( this, iFirstMip, iLastMip, bDynamic);
  _pfWorldEditingProfile.StopTimer( CWorldEditingProfile::PTI_MIXLAYERS);
  _sfStats.StopTimer( CStatForm::STI_SHADOWUPDATE);
}
