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

#ifndef SE_INCL_VERTEX_H
#define SE_INCL_VERTEX_H
#ifdef PRAGMA_ONCE
  #pragma once
#endif


struct GFXVertex3
{
  FLOAT x,y,z;
};


struct GFXNormal3
{
  FLOAT nx,ny,nz;
};


struct GFXTexCoord
{
  union {
    struct { FLOAT u,v; } uv;
    struct { FLOAT s,t; } st;
  } gfxtc;
};


struct GFXTexCoord4
{
  FLOAT s,t,r,q;
};


struct GFXColor
{
  union {
    struct { UBYTE r,g,b,a; } ub;
    struct { ULONG abgr;    } ul;  // reverse order - use ByteSwap()!
  } gfxcol;

  GFXColor() {};

  GFXColor( COLOR col) {
    // little endian to big endian
    ULONG temp = 0;
    temp |= (col & 0xff) << 24;
    temp |= (col >> 8 & 0xff) << 16;
    temp |= (col >> 16 & 0xff) << 8;
    temp |= (col >> 24 & 0xff);
    gfxcol.ul.abgr = temp;
  }

  __forceinline void Set( COLOR col) {
    ULONG temp = 0;
    temp |= (col & 0xff) << 24;
    temp |= (col >> 8 & 0xff) << 16;
    temp |= (col >> 16 & 0xff) << 8;
    temp |= (col >> 24 & 0xff);
    gfxcol.ul.abgr = temp;
  }

  void MultiplyRGBA( const GFXColor &col1, const GFXColor &col2) {
    gfxcol.ub.r = (ULONG(col1.gfxcol.ub.r)*col2.gfxcol.ub.r)>>8;
    gfxcol.ub.g = (ULONG(col1.gfxcol.ub.g)*col2.gfxcol.ub.g)>>8;
    gfxcol.ub.b = (ULONG(col1.gfxcol.ub.b)*col2.gfxcol.ub.b)>>8;
    gfxcol.ub.a = (ULONG(col1.gfxcol.ub.a)*col2.gfxcol.ub.a)>>8;
  }

  void MultiplyRGB( const GFXColor &col1, const GFXColor &col2) {
    gfxcol.ub.r = (ULONG(col1.gfxcol.ub.r)*col2.gfxcol.ub.r)>>8;
    gfxcol.ub.g = (ULONG(col1.gfxcol.ub.g)*col2.gfxcol.ub.g)>>8;
    gfxcol.ub.b = (ULONG(col1.gfxcol.ub.b)*col2.gfxcol.ub.b)>>8;
  }

  void MultiplyRGBCopyA1( const GFXColor &col1, const GFXColor &col2) {
    gfxcol.ub.r = (ULONG(col1.gfxcol.ub.r)*col2.gfxcol.ub.r)>>8;
    gfxcol.ub.g = (ULONG(col1.gfxcol.ub.g)*col2.gfxcol.ub.g)>>8;
    gfxcol.ub.b = (ULONG(col1.gfxcol.ub.b)*col2.gfxcol.ub.b)>>8;
    gfxcol.ub.a = col1.gfxcol.ub.a;
  }

  void AttenuateRGB( ULONG ulA) {
    gfxcol.ub.r = (ULONG(gfxcol.ub.r)*ulA)>>8;
    gfxcol.ub.g = (ULONG(gfxcol.ub.g)*ulA)>>8;
    gfxcol.ub.b = (ULONG(gfxcol.ub.b)*ulA)>>8;
  }

  void AttenuateA( ULONG ulA) {
    gfxcol.ub.a = (ULONG(gfxcol.ub.a)*ulA)>>8;
  }
};


#define GFXVertex GFXVertex4
struct GFXVertex4
{
  GFXVertex4()
  {
  }
  FLOAT x,y,z;
  union {
    struct { struct GFXColor col; };
    struct { SLONG shade; };
  };
};


#define GFXNormal GFXNormal4
struct GFXNormal4
{
  FLOAT nx,ny,nz;
  ULONG ul;
};



#endif  /* include-once check. */

