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

#ifndef SE_INCL_VIEWPORT_H
#define SE_INCL_VIEWPORT_H
#ifdef PRAGMA_ONCE
  #pragma once
#endif

#include <Engine/Graphics/Raster.h>
#include <android/native_window.h>
#include <EGL/egl.h>

/*
 *  ViewPort
 */

// base abstract class for viewport
class ENGINE_API CViewPort {
public:
// implementation
  HWND vp_hWnd;                 // canvas (child) window
  HWND vp_hWndParent;           // window of the viewport
  CRaster vp_Raster;            // the used Raster
  INDEX vp_ctDisplayChanges;    // number of display driver

  // open/close canvas window
  void OpenCanvas(void);
  void CloseCanvas(BOOL bRelease=FALSE);

  // initialize for specific window
  void Initialize(ANativeWindow *window);

// interface
  /* Constructor for given window. */
  CViewPort();
	/* Destructor. */
  ~CViewPort(void);

	/* Display the back buffer on screen. */
  void SwapBuffers(void);
  // change size of this viewport, it's raster and all it's drawports to fit it window
  void Resize(void);

  // EGL
  bool eglInitialized = false;
  EGLDisplay display = 0;
  EGLSurface surface = 0;
  EGLContext context = 0;
  EGLint format;
  EGLConfig config;

  static bool hasCViewPort;
};


#endif  /* include-once check. */

