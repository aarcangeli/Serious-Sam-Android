#include <AndroidBindings/seriousSamAndroid.h>
#include <Engine/Templates/DynamicContainer.h>
#include <Engine/Templates/DynamicContainer.cpp>
#include <Engine/Base/Synchronization.h>
#include <Engine/Base/FileName.h>
#include <Engine/Base/Unzip.h>
#include <dirent.h>
#include <dlfcn.h>
#include <GameMP/Game.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <cstdint>
#include <vector>

typedef CGame *(*GAME_Create_t)(void);
void CTimer_TimerFunc_internal(void);
void CPrintLog(CTString strBuffer);

namespace gles_adapter {
  void gles_adp_init();

  void syncBuffers();
  extern bool enableDraws;
}

CGame *game;

int64_t lastTick = 0;
uint32_t tickCount = 0;
bool initialized = false;
bool g_isRunningIntro = false;
bool g_runFirstLevel = false;

BOOL TryToSetDisplayMode(enum GfxAPIType eGfxAPI, INDEX iAdapter, PIX pixSizeI, PIX pixSizeJ,
                         enum DisplayDepth eColorDepth, BOOL bFullScreenMode) {
  CDisplayMode dmTmp;
  dmTmp.dm_ddDepth = eColorDepth;

  CPrintF(TRANS("  Starting display mode: %dx%dx%s (%s)\n"),
          pixSizeI, pixSizeJ, dmTmp.DepthString(),
          bFullScreenMode ? TRANS("fullscreen") : TRANS("window"));

  if (bFullScreenMode) {
    FatalError("Fullscreen???");
  } else {
    BOOL success = _pGfx->ResetDisplayMode(eGfxAPI);
    if (!success) {
      FatalError("Cannot set display mode");
    }
  }

  return true;
}

void StartNewMode(enum GfxAPIType eGfxAPI,
                  INDEX iAdapter, PIX pixSizeI,
                  PIX pixSizeJ, enum DisplayDepth eColorDepth, BOOL bFullScreenMode) {
  CPrintF(TRANS("\n* START NEW DISPLAY MODE ...\n"));

  BOOL bSuccess = TryToSetDisplayMode(eGfxAPI, iAdapter, pixSizeI, pixSizeJ, eColorDepth,
                                      bFullScreenMode);
}

extern COLOR LCDGetColor(COLOR colDefault, const char *strName) {
  return game->LCDGetColor(colDefault, strName);
}

void printGlError(const char *name) {
  EGLint err = glGetError();
  if (err) {
    WarningMessage("OpenGL Error %s: 0x%04X", name, err);
  };
}

void startGame(CTString level, bool isIntro) {
  game->gm_StartSplitScreenCfg = CGame::SSC_PLAY1;
  game->gm_aiStartLocalPlayers[0] = game->gm_iSinglePlayer;
  game->gm_aiStartLocalPlayers[1] = -1;
  game->gm_aiStartLocalPlayers[2] = -1;
  game->gm_aiStartLocalPlayers[3] = -1;
  game->gm_strNetworkProvider = "Local";
  game->gm_StartSplitScreenCfg = CGame::SSC_PLAY1;

  _pShell->SetINDEX("gam_iStartDifficulty", CSessionProperties::GD_NORMAL);

  bool runIntro = true;
  if (isIntro) {
    level = "Levels/LevelsMP/Intro.wld";
    _pShell->SetINDEX("gam_iStartMode", CSessionProperties::GM_FLYOVER);
  } else {
    _pShell->SetINDEX("gam_iStartMode", CSessionProperties::GM_COOPERATIVE);
  }

  CUniversalSessionProperties sp;
  game->SetSinglePlayerSession(sp);
  game->gm_bFirstLoading = TRUE;

  if (!game->NewGame(level, level, sp)) {
    FatalError("Cannot start '%s'\n", level);
  }

}

void setControls(PlayerControls &ctrls) {
  static bool bStartLast = false;

  if (initialized) {

    if (ctrls.bUse) {
      if (game->gm_csComputerState == CS_ON || game->gm_csComputerState == CS_TURNINGON) {
        game->gm_csComputerState = CS_TURNINGOFF;
        ctrls.bUse = false;
      }
    }

    if (ctrls.bUse) {
      if (game->gm_csConsoleState == CS_ON || game->gm_csComputerState == CS_TURNINGON ||
          game->gm_csComputerState == CS_TALK) {
        game->gm_csConsoleState = CS_TURNINGOFF;
        ctrls.bUse = false;
      }
    }

    if (ctrls.bStart && ctrls.bStart != bStartLast) {
      if (game->gm_csConsoleState == CS_OFF || game->gm_csConsoleState == CS_TURNINGOFF) {
        game->gm_csConsoleState = CS_TURNINGON;
      } else {
        game->gm_csConsoleState = CS_TURNINGOFF;
      }
    }

    if ((ctrls.bMoveUp || ctrls.bUse) && g_isRunningIntro) {
      g_runFirstLevel = true;
      g_isRunningIntro = false;
    }

    bStartLast = ctrls.bStart;

    if (g_printProfiling) {
      g_printProfiling = false;
      _pShell->Execute("RecordProfile();");
    }

    memcpy(game->gm_lpLocalPlayers[0].lp_ubPlayerControlsState, &ctrls, sizeof(ctrls));
  }
}

void startSeriousPrestart() {
  CTStream::EnableStreamHandling();
  SE_InitEngine("SeriousSam");
  SE_LoadDefaultFonts();

//  // translations
//  InitTranslation();
//  try {
//    AddTranslationTablesDir_t(CTString("Data\\Translations\\"), CTString("*.txt"));
//    FinishTranslationTable();
//  } catch (char *strError) {
//    FatalError("%s", strError);
//  }

  void *libGameMP = dlopen("libGameMP.so", RTLD_NOW);
  if (!libGameMP) {
    FatalError("  Cannot load GameMP");
  }
  CPrintF("  libGameMP.so loaded\n");

  GAME_Create_t GAME_Create = (GAME_Create_t) dlsym(libGameMP, "GAME_Create");
  if (!GAME_Create) {
    FatalError("  Cannot find GAME_Create");
  }
  CPrintF("  GAME_Create found\n");
  CPrintF("\n");

  _pNetwork->md_strGameID = "SeriousSam";

  game = GAME_Create();
  _pTimer->tm_bPaused = !g_gameRunning;
}

void startSeriousSamAndroid(CDrawPort *pdp) {
  game->EnableLoadingHook(pdp);
  game->Initialize(CTString("Data\\SeriousSam.gms"));
  game->LCDInit();

  _pInput->EnableInput();

  CPrintF("Level list:\n"); // TODO: GetLevelInfo
  CDynamicStackArray<CTFileName> afnmDir;
  MakeDirList(afnmDir, CTString("Levels\\"), "*.wld", DLI_RECURSIVE | DLI_SEARCHCD);
  for (INDEX i = 0; i < afnmDir.Count(); i++) {
    CTFileName fnm = afnmDir[i];
    CPrintF("  level: '%s'\n", fnm);
  }
  CPrintF("\n");

  CPrintF("Demos:\n");
  CDynamicStackArray<CTFileName> demoDir;
  MakeDirList(demoDir, CTString("Demos\\"), "Demos/Auto-*.dem", DLI_RECURSIVE);
  for (INDEX i = 0; i < demoDir.Count(); i++) {
    CTFileName fnm = demoDir[i];
    CPrintF("  level: '%s'\n", fnm);
  }
  CPrintF("\n");

  StartNewMode(GAT_OGL, 0, 640, 480, DD_DEFAULT, false);

  // override key settings
  CControls &ctrl = game->gm_actrlControls[0];
  for (int i = 0; i < 10; i++) {
    ctrl.ctrl_aaAxisActions[i].aa_fSensitivity = 80;
    ctrl.ctrl_aaAxisActions[i].aa_fDeadZone = 10;
    ctrl.ctrl_aaAxisActions[i].aa_bInvert = false;
    ctrl.ctrl_aaAxisActions[i].aa_bRelativeControler = true;
    ctrl.ctrl_aaAxisActions[i].aa_bSmooth = false;
  }

  // "Levels/LevelsMP/Intro.wld";
  // "Levels/LevelsMP/1_0_InTheLastEpisode.wld";
  // "Levels/LevelsMP/1_1_Palenque.wld";

  startGame("Levels/LevelsMP/Intro.wld", true);
  g_isRunningIntro = true;

  CTStream::DisableStreamHandling();

  CPrintLog(TRANS("\n--- Serious Engine CPP End ---\n"));
}

void seriousSamDoGame(CDrawPort *pdp) {
  gles_adapter::enableDraws = true;

  if (!initialized) {
    // initialize gles adapter
    try {
      gles_adapter::gles_adp_init();
    } catch (const char *txt) {
      FatalError("OpenGL init error: %s", txt);
    }

    // start engine
    startSeriousSamAndroid(pdp);

    initialized = true;
  }

  gles_adapter::enableDraws = true;

  int64_t start = getTimeNsec();
  static int64_t lastFrameDraw = start;
  int64_t deltaFrame = start - lastFrameDraw;
  lastFrameDraw = start;

  static int64_t seriousSamDoGameTime = 0;

  const int64_t UPDATE_TIME = 2000000000; // 2s
  static int64_t lastFpsNow = start;
  static int64_t times = 0;
  static float fps;
  if (start - lastFpsNow > UPDATE_TIME) {
    fps = (float) times / (start - lastFpsNow) * 1000 * 1000 * 1000;
    lastFpsNow = start;
    times = 0;
  }
  times++;

  CTStream::EnableStreamHandling();

  if (g_runFirstLevel) {
    startGame("Levels/LevelsMP/1_1_Palenque.wld", false);
    g_runFirstLevel = false;
  }

  game->GameMainLoop();

  if (pdp != NULL && pdp->Lock()) {
//    InfoMessage("Frame Start");
    pdp->Fill(C_BLACK | CT_OPAQUE);

    // handle pretouching of textures and shadowmaps
    pdp->Unlock();

//    ULONG ulFlags = (game->gm_csConsoleState != CS_OFF || bMenuActive) ? 0 : GRV_SHOWEXTRAS;
    ULONG ulFlags = 0;
//    pdp->Fill(LCDGetColor(C_dGREEN | CT_OPAQUE, "bcg fill"));

    game->GameRedrawView(pdp, GRV_SHOWEXTRAS);

    // draw computer if needed
    pdp->Lock();
    game->ComputerRender(pdp);
    game->ConsoleRender(pdp);

    // draw fps and frame time
    SLONG slDPWidth = pdp->GetWidth();
    SLONG slDPHeight = pdp->GetHeight();
    pdp->SetFont(_pfdDisplayFont);
    pdp->SetTextScaling(1);
    pdp->SetTextAspect(1.0f);
    CTString str = CTString(0, "V1.01 fps: %.2f; frame: %.2f ms", fps, deltaFrame / 1000000.f);
    pdp->PutText(str, 0, 0, LCDGetColor(C_GREEN | 255, "display mode"));

    pdp->Unlock();

    // Update pause state
    _pNetwork->SetLocalPause(game->gm_csConsoleState == CS_ON ||
                             game->gm_csConsoleState == CS_TURNINGON ||
                             game->gm_csConsoleState == CS_TURNINGOFF ||
                             game->gm_csComputerState == CS_ON ||
                             game->gm_csComputerState == CS_TURNINGON ||
                             game->gm_csComputerState == CS_TURNINGOFF);

    pdp->dp_Raster->ra_pvpViewPort->SwapBuffers();
  }

  CTStream::DisableStreamHandling();

  seriousSamDoGameTime = getTimeNsec() - start;
}
