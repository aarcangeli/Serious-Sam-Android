#include <Engine/StdH.h>


#include <Engine/Graphics/ViewPort.h>

#include <Engine/Graphics/GfxProfile.h>
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Base/Statistics_internal.h>

extern INDEX ogl_bExclusive;

CViewPort::CViewPort(PIX pixWidth, PIX pixHeight, HWND hWnd) :
  vp_Raster(pixWidth, pixHeight, 0) {
  vp_Raster.ra_pvpViewPort = this;
}

CViewPort::~CViewPort(void) {}

static BOOL _bClassRegistered = FALSE;

void CViewPort::OpenCanvas(void) {}

void CViewPort::CloseCanvas(BOOL bRelease/*=FALSE*/) {}

void CViewPort::Resize(void) {}

void CViewPort::SwapBuffers(void) {}
