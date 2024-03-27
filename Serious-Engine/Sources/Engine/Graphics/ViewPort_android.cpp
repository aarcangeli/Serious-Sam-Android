#include <Engine/StdH.h>
#include <Engine/Graphics/ViewPort.h>
#include <Engine/Graphics/GfxProfile.h>
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Base/Statistics_Internal.h>
#include <Engine/Base/ErrorReporting.h>
#include <vector>
#include <Engine/Base/Shell.h>

extern INDEX ogl_bExclusive;

bool CViewPort::hasCViewPort = false;

CViewPort::CViewPort()
  : vp_Raster(0, 0, 0){
  vp_Raster.ra_pvpViewPort = this;

  // ensure that just one viewport is created
  if (hasCViewPort) FatalError("CViewPort is singleton");
  hasCViewPort = true;
}

CViewPort::~CViewPort(void) {}

static BOOL _bClassRegistered = FALSE;

int findConfigAttrib(EGLDisplay display, EGLConfig &config, int attribute, int defaultValue) {
  int mValue;
  if (eglGetConfigAttrib(display, config, attribute, &mValue)) {
    return mValue;
  }
  return defaultValue;
}

void CViewPort::Initialize(ANativeWindow *window) {
  const EGLint attribs[] = {
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_DEPTH_SIZE, 16,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
    EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_NONE
  };

  if (!eglInitialized) {
    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
      FatalError("eglGetDisplay() returned error 0x%04X", eglGetError());
    }

    if (!eglInitialize(display, 0, 0)) {
      FatalError("eglInitialize() returned error 0x%04X", eglGetError());
    }

    // get all configurations
    EGLint numConfigs;
    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
      FatalError("eglChooseConfig() returned error 0x%04X", eglGetError());
    }
    if (numConfigs <= 0) {
      FatalError("no configuration found");
    }

    if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
      FatalError("eglGetConfigAttrib() returned error 0x%04X", eglGetError());
    }

    ANativeWindow_setBuffersGeometry(window, 0, 0, format);

    const EGLint context_attrib_list[] = {
      // request a context using Open GL ES 2.0
      EGL_CONTEXT_CLIENT_VERSION, 3,
      EGL_NONE
    };
    if (!(context = eglCreateContext(display, config, 0, context_attrib_list))) {
      FatalError("eglCreateContext() returned error 0x%04X", eglGetError());
    }

    eglInitialized = true;
  }

  if (surface) {
    if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
      WarningMessage("eglMakeCurrent(null) returned error 0x%04X", eglGetError());
    }
    if (!eglDestroySurface(display, surface)) {
      WarningMessage("eglCreateContext() returned error 0x%04X", eglGetError());
    }
  }

  if (!(surface = eglCreateWindowSurface(display, config, window, 0))) {
    FatalError("eglCreateWindowSurface() returned error 0x%04X", eglGetError());
  }

  if (!eglMakeCurrent(display, surface, surface, context)) {
    FatalError("eglMakeCurrent() returned error 0x%04X", eglGetError());
  }

  Resize();
}

void CViewPort::OpenCanvas(void) {
}

void CViewPort::CloseCanvas(BOOL bRelease) {
}

void CViewPort::Resize(void) {
  EGLint width, height;
  if (!eglQuerySurface(display, surface, EGL_WIDTH, &width) ||
      !eglQuerySurface(display, surface, EGL_HEIGHT, &height)) {
    FatalError("eglQuerySurface() returned error 0x%04X", eglGetError());
  }

  if (width != vp_Raster.ra_Width || height != vp_Raster.ra_Height) {
    vp_Raster.Resize(width, height);
    _pShell->Execute("ViewportResized();");
  }
}

void CViewPort::SwapBuffers(void) {

  // ask the current driver to swap buffers
  _sfStats.StartTimer(CStatForm::STI_SWAPBUFFERS);
  _pfGfxProfile.StartTimer( CGfxProfile::PTI_SWAPBUFFERS);
  _pfGfxProfile.IncrementAveragingCounter(1);

  _pGfx->SwapBuffers(this);

  _pfGfxProfile.StopTimer( CGfxProfile::PTI_SWAPBUFFERS);
  _sfStats.StopTimer(CStatForm::STI_SWAPBUFFERS);
}
