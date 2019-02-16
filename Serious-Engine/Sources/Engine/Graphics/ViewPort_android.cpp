#include <Engine/StdH.h>
#include <Engine/Graphics/ViewPort.h>
#include <Engine/Graphics/GfxProfile.h>
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Base/Statistics_internal.h>
#include <Engine/Base/ErrorReporting.h>
#include <vector>

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

  if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
    FatalError("eglGetDisplay() returned error %d", eglGetError());
  }

  if (!eglInitialize(display, 0, 0)) {
    FatalError("eglInitialize() returned error %d", eglGetError());
  }

  std::vector<EGLConfig> configs;

  // get configuration count
  EGLint numConfigs;
  if (!eglChooseConfig(display, attribs, nullptr, 0, &numConfigs)) {
    FatalError("eglChooseConfig() returned error %d", eglGetError());
  }
  if (numConfigs <= 0) {
    FatalError("no configuration found");
  }

  // get all configurations
  configs.resize(numConfigs);
  if (!eglChooseConfig(display, attribs, configs.data(), numConfigs, &numConfigs)) {
    FatalError("eglChooseConfig() returned error %d", eglGetError());
  }

  int asd = EGL_OPENGL_ES2_BIT;
  // iterate and choose
  for (EGLConfig &config : configs) {
    int d = findConfigAttrib(display, config, EGL_DEPTH_SIZE, 0);
    int s = findConfigAttrib(display, config, EGL_STENCIL_SIZE, 0);
    int r = findConfigAttrib(display, config, EGL_RED_SIZE, 0);
    int g = findConfigAttrib(display, config, EGL_GREEN_SIZE, 0);
    int b = findConfigAttrib(display, config, EGL_BLUE_SIZE, 0);
    int a = findConfigAttrib(display, config, EGL_ALPHA_SIZE, 0);
    int es = findConfigAttrib(display, config, EGL_RENDERABLE_TYPE, 0);
    int conf = findConfigAttrib(display, config, EGL_CONFORMANT, 0);
    int surface = findConfigAttrib(display, config, EGL_SURFACE_TYPE, 0);

    int t = 0;
  }

  EGLConfig config = configs[0];
  if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
    FatalError("eglGetConfigAttrib() returned error %d", eglGetError());
  }

  ANativeWindow_setBuffersGeometry(window, 0, 0, format);

  if (!(surface = eglCreateWindowSurface(display, config, window, 0))) {
    FatalError("eglCreateWindowSurface() returned error %d", eglGetError());
  }

  const EGLint context_attrib_list[] = {
    // request a context using Open GL ES 2.0
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };
  if (!(context = eglCreateContext(display, config, 0, context_attrib_list))) {
    FatalError("eglCreateContext() returned error %d", eglGetError());
  }

  if (!eglMakeCurrent(display, surface, surface, context)) {
    FatalError("eglMakeCurrent() returned error %d", eglGetError());
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
    FatalError("eglQuerySurface() returned error %d", eglGetError());
  }

  vp_Raster.Resize(width, height);
}

void CViewPort::SwapBuffers(void) {

  // ask the current driver to swap buffers
  _sfStats.StartTimer(CStatForm::STI_SWAPBUFFERS);
  _pfGfxProfile.StartTimer( CGfxProfile::PTI_SWAPBUFFERS);
  _pfGfxProfile.IncrementAveragingCounter(1);

  if (!eglSwapBuffers(display, surface)) {
    WarningMessage("eglSwapBuffers() returned error %d", eglGetError());
  }

  _pfGfxProfile.StopTimer( CGfxProfile::PTI_SWAPBUFFERS);
  _sfStats.StopTimer(CStatForm::STI_SWAPBUFFERS);
}
