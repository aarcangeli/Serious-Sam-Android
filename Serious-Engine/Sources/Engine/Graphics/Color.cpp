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

#include <Engine/Graphics/Color.h>
#include <Engine/Math/Functions.h>
#include <Engine/Base/ErrorReporting.h>
#include <cstdint>

// asm shortcuts
#define O offset
#define Q qword ptr
#define D dword ptr
#define W  word ptr
#define B  byte ptr


extern const UBYTE *pubClipByte;


// convert HSV components to CroTeam COLOR format
COLOR HSVToColor( UBYTE const ubH, UBYTE const ubS, UBYTE const ubV)
{
  if( ubS>1) {
    SLONG xH   = (SLONG)ubH *1536; // ->FIXINT/(256/6)
    INDEX iHlo = xH & 0xFFFF; 
    SLONG slP  = ((SLONG)ubV * (256-  (SLONG)ubS)) >>8;
    SLONG slQ  = ((SLONG)ubV * (256-(((SLONG)ubS*iHlo)>>16))) >>8;
    SLONG slT  = ((SLONG)ubV * (256-(((SLONG)ubS*(65536-iHlo))>>16))) >>8;
    switch( xH>>16) {
      case 0:  return RGBToColor(ubV,slT,slP);
      case 1:  return RGBToColor(slQ,ubV,slP);
      case 2:  return RGBToColor(slP,ubV,slT);
      case 3:  return RGBToColor(slP,slQ,ubV);
      case 4:  return RGBToColor(slT,slP,ubV);
      case 5:  return RGBToColor(ubV,slP,slQ);
      default: ASSERTALWAYS("WHAT???"); return C_BLACK;
    }
  } else return RGBToColor(ubV,ubV,ubV);
}


// convert CroTeam COLOR format to HSV components
void ColorToHSV( COLOR const colSrc, UBYTE &ubH, UBYTE &ubS, UBYTE &ubV)
{
  UBYTE ubR, ubG, ubB;
  ColorToRGB( colSrc, ubR,ubG,ubB);
  ubH = 0;
  ubS = 0;
  ubV = Max( Max(ubR,ubG),ubB);
  if( ubV>1) {
    SLONG slD = ubV - Min( Min(ubR,ubG),ubB);
    if( slD<1) return;
    ubS = (slD*255) /ubV; 
         if( ubR==ubV) ubH =   0+ (((SLONG)ubG-ubB)*85) / (slD*2);
    else if( ubG==ubV) ubH =  85+ (((SLONG)ubB-ubR)*85) / (slD*2);
    else               ubH = 170+ (((SLONG)ubR-ubG)*85) / (slD*2);
  }
}



// color checking routines

#define  GRAY_TRESHOLD  4
#define WHITE_TRESHOLD (255-GRAY_TRESHOLD)

BOOL IsGray( COLOR const col)
{
  UBYTE ubR,ubG,ubB;
  ColorToRGB( col, ubR,ubG,ubB);
  INDEX iMaxDelta = Max( Max(ubR,ubG),ubB) - Min( Min(ubR,ubG),ubB);
  if( iMaxDelta<GRAY_TRESHOLD) return TRUE;
  return FALSE;
}

BOOL IsBlack( COLOR const col)
{
  UBYTE ubR,ubG,ubB;
  ColorToRGB( col, ubR,ubG,ubB);
  if( ubR<GRAY_TRESHOLD && ubG<GRAY_TRESHOLD && ubB<GRAY_TRESHOLD) return TRUE;
  return FALSE;
}

BOOL IsWhite( COLOR const col)
{
  UBYTE ubR,ubG,ubB;
  ColorToRGB( col, ubR,ubG,ubB);
  if( ubR>WHITE_TRESHOLD && ubG>WHITE_TRESHOLD && ubB>WHITE_TRESHOLD) return TRUE;
  return FALSE;
}

BOOL IsBigger( COLOR const col1, COLOR const col2)
{
  UBYTE ubR1,ubG1,ubB1;
  UBYTE ubR2,ubG2,ubB2;
  ColorToRGB( col1, ubR1,ubG1,ubB1);
  ColorToRGB( col2, ubR2,ubG2,ubB2);
  SLONG slGray1 = (((SLONG)ubR1+ubG1+ubB1)*21846) >>16;
  SLONG slGray2 = (((SLONG)ubR2+ubG2+ubB2)*21846) >>16;
  return (slGray1>slGray2);
}


// has color same hue and saturation (with little tolerance) ?
BOOL CompareChroma( COLOR col1, COLOR col2)
{ 
  // make color1 bigger
  if( IsBigger(col2,col1)) Swap(col1,col2);

  // find biggest component
  SLONG slR1=0, slG1=0, slB1=0;
  SLONG slR2=0, slG2=0, slB2=0;
  ColorToRGB( col1, (UBYTE&)slR1, (UBYTE&)slG1, (UBYTE&)slB1);
  ColorToRGB( col2, (UBYTE&)slR2, (UBYTE&)slG2, (UBYTE&)slB2);
  SLONG slMax1 = Max(Max(slR1,slG1),slB1);
  SLONG slMax2 = Max(Max(slR2,slG2),slB2);
  // trivial?
  if( slMax1<GRAY_TRESHOLD || slMax2<GRAY_TRESHOLD) return TRUE;

  // find expected color
  SLONG slR,slG,slB, slDiv;
  if( slR1==slMax1) {
    slDiv = 65536 / slR1;
    slR =  slR2;
    slG = (slR2*slG1*slDiv)>>16;
    slB = (slR2*slB1*slDiv)>>16;
  } else if( slG1==slMax1) {
    slDiv = 65536 / slG1;
    slR = (slG2*slR1*slDiv)>>16;
    slG =  slG2;
    slB = (slG2*slB1*slDiv)>>16;
  } else {
    slDiv = 65536 / slB1;
    slR = (slB2*slR1*slDiv)>>16;
    slG = (slB2*slG1*slDiv)>>16;
    slB =  slB2;
  }

  // check expected color
  if( Abs(slR-slR2) > (GRAY_TRESHOLD/2)) return FALSE;
  if( Abs(slG-slG2) > (GRAY_TRESHOLD/2)) return FALSE;
  if( Abs(slB-slB2) > (GRAY_TRESHOLD/2)) return FALSE;
  return TRUE;
}


// find corresponding desaturated color (it's not same as gray!)
COLOR DesaturateColor( COLOR const col)
{
  UBYTE ubR,ubG,ubB,ubA, ubMax;
  ColorToRGBA( col, ubR,ubG,ubB,ubA);
  ubMax = Max(Max(ubR,ubG),ubB);
  return RGBAToColor( ubMax,ubMax,ubMax,ubA);
}


// adjust color saturation and/or hue
COLOR AdjustColor( COLOR const col, SLONG const slHueShift, SLONG const slSaturation)
{
  // nothing?
  if( slHueShift==0 && slSaturation==256) return col;
  // saturation?
  COLOR colRes = col;
  UBYTE ubA = (col&CT_AMASK)>>CT_ASHIFT;
  if( slSaturation!=256)
  { // calculate gray factor
    UBYTE ubR,ubG,ubB;
    ColorToRGB( col, ubR,ubG,ubB);
    SLONG slGray = (ubR*72 + ubG*152 + ubB*32)>>8;
    // saturate color components
    SLONG slR = slGray + (((ubR-slGray)*slSaturation)>>8);
    SLONG slG = slGray + (((ubG-slGray)*slSaturation)>>8);
    SLONG slB = slGray + (((ubB-slGray)*slSaturation)>>8);
    // clamp color components
    colRes = RGBToColor( pubClipByte[slR], pubClipByte[slG], pubClipByte[slB]);
  }
  // hue?
  if( slHueShift==0) return colRes|ubA;
  UBYTE ubH,ubS,ubV;
  ColorToHSV( colRes, ubH,ubS,ubV);
  ubH += slHueShift;
  return HSVAToColor( ubH,ubS,ubV,ubA);
}


// adjust color gamma correction
COLOR AdjustGamma( COLOR const col, FLOAT const fGamma)
{
  if( fGamma==1.0f || fGamma<0.2f) return col;
  const FLOAT f1oGamma = 1.0f / fGamma;
  const FLOAT f1o255   = 1.0f / 255.0f;
  UBYTE ubR,ubG,ubB,ubA;
  ColorToRGBA( col, ubR,ubG,ubB,ubA);
  ubR = ClampUp( NormFloatToByte(pow(ubR*f1o255,f1oGamma)), (ULONG) 255);
  ubG = ClampUp( NormFloatToByte(pow(ubG*f1o255,f1oGamma)), (ULONG) 255);
  ubB = ClampUp( NormFloatToByte(pow(ubB*f1o255,f1oGamma)), (ULONG) 255);
  return RGBAToColor( ubR,ubG,ubB,ubA);
}



// color lerping functions

void LerpColor( COLOR col0, COLOR col1, FLOAT fRatio, UBYTE &ubR, UBYTE &ubG, UBYTE &ubB)
{
  UBYTE ubR0, ubG0, ubB0;
  UBYTE ubR1, ubG1, ubB1;
  ColorToRGB( col0, ubR0, ubG0, ubB0);
  ColorToRGB( col1, ubR1, ubG1, ubB1);
  ubR = Lerp( ubR0, ubR1, fRatio);
  ubG = Lerp( ubG0, ubG1, fRatio);
  ubB = Lerp( ubB0, ubB1, fRatio);
}


COLOR LerpColor( COLOR col0, COLOR col1, FLOAT fRatio)
{
  UBYTE ubR0, ubG0, ubB0, ubA0;
  UBYTE ubR1, ubG1, ubB1, ubA1;
  ColorToRGBA( col0, ubR0, ubG0, ubB0, ubA0);
  ColorToRGBA( col1, ubR1, ubG1, ubB1, ubA1);
  ubR0 = Lerp( ubR0, ubR1, fRatio);
  ubG0 = Lerp( ubG0, ubG1, fRatio);
  ubB0 = Lerp( ubB0, ubB1, fRatio);
  ubA0 = Lerp( ubA0, ubA1, fRatio);
  return RGBAToColor( ubR0, ubG0, ubB0, ubA0);
}



// fast color multiply function - RES = 1ST * 2ND /255
COLOR MulColors( COLOR col1, COLOR col2) 
{
  if (col1 == 0xFFFFFFFF) return col2;
  if (col2 == 0xFFFFFFFF) return col1;
  if (col1 == 0 || col2 == 0) return 0;

  union
  {
    COLOR col;
    UBYTE bytes[4];
  } conv1;

  union
  {
    COLOR col;
    UBYTE bytes[4];
  } conv2;

  conv1.col = col1;
  conv2.col = col2;
  conv1.bytes[0] = (UBYTE)((((DWORD) conv1.bytes[0]) * ((DWORD) conv2.bytes[0])) / 255);
  conv1.bytes[1] = (UBYTE)((((DWORD) conv1.bytes[1]) * ((DWORD) conv2.bytes[1])) / 255);
  conv1.bytes[2] = (UBYTE)((((DWORD) conv1.bytes[2]) * ((DWORD) conv2.bytes[2])) / 255);
  conv1.bytes[3] = (UBYTE)((((DWORD) conv1.bytes[3]) * ((DWORD) conv2.bytes[3])) / 255);

  return(conv1.col);

}


uint32_t clampNumber(uint32_t num) {
  return num <= 255 ? num : 255;
}

// fast color additon function - RES = clamp (1ST + 2ND)
COLOR AddColors( COLOR col1, COLOR col2) 
{
  if (col1 == 0) return col2;
  if (col2 == 0) return col1;
  if (col1 == 0xFFFFFFFF || col2 == 0xFFFFFFFF) return 0xFFFFFFFF;

  union
  {
    COLOR col;
    UBYTE bytes[4];
  } conv1;

  union
  {
    COLOR col;
    UBYTE bytes[4];
  } conv2;

  conv1.col = col1;
  conv2.col = col2;

  conv1.bytes[0] = Min(((WORD)conv1.bytes[0]) + ((WORD)conv2.bytes[0]), 255);
  conv1.bytes[1] = Min(((WORD)conv1.bytes[1]) + ((WORD)conv2.bytes[1]), 255);
  conv1.bytes[2] = Min(((WORD)conv1.bytes[2]) + ((WORD)conv2.bytes[2]), 255);
  conv1.bytes[3] = Min(((WORD)conv1.bytes[3]) + ((WORD)conv2.bytes[3]), 255);

  return conv1.col;

}



// multiple conversion from OpenGL color to DirectX color
void abgr2argb( ULONG *pulSrc, ULONG *pulDst, INDEX ct)
{
  for (int i = 0; i<ct; i++)
  {
    pulDst[i] = abgr2argb(pulSrc[i]);
  }

}
