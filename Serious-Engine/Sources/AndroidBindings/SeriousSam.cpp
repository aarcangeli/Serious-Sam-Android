#include <Engine/StdH.h>
#include <android/log.h>
#include <Engine/Templates/DynamicContainer.h>
#include <Engine/Templates/DynamicContainer.cpp>
#include <Engine/Base/Synchronization.h>
#include <Engine/Base/FileName.h>
#include <Engine/Base/CTString.h>
#include <Engine/Base/Stream.h>
#include <Engine/Base/Unzip.h>
#include <Engine/Engine.h>
#include <dirent.h>
#include <dlfcn.h>
#include <GameMP/Game.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <cstdint>
#include <Engine/Base/Statistics_internal.h>
#include <vector>

typedef CGame *(*GAME_Create_t)(void);
void CTimer_TimerFunc_internal(void);

namespace gles_adapter {
  void gles_adp_init();

  void syncBuffers();
  extern bool enableDraws;
}

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

CGame *game;

CDrawPort *pdp = nullptr;
CViewPort *pvpViewPort;

int64_t lastTick = 0;
uint32_t tickCount = 0;
bool started = false;
bool initialized = false;
float g_uiScale = 1;

#define MAX_AXIS 10
#define AXIS_SHIFT 1
#define NS_IN_S 1000000000
float axisValue[MAX_AXIS];

extern COLOR LCDGetColor(COLOR colDefault, const char *strName) {
  return game->LCDGetColor(colDefault, strName);
}

void printGlError(const char *name) {
  EGLint err = glGetError();
  if (err) {
    WarningMessage("OpenGL Error %s: 0x%04X", name, err);
  };
}

void startSeriousSamAndroid() {
  if (started) return;
  started = true;

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
  game->Initialize(CTString("Data\\SeriousSam.gms"));
  game->LCDInit();

  _pInput->EnableInput();

  // todo: sound library
//  snd_iFormat = Clamp( snd_iFormat, (INDEX)CSoundLibrary::SF_NONE, (INDEX)CSoundLibrary::SF_44100_16);
//  _pSound->SetFormat( (enum CSoundLibrary::SoundFormat)snd_iFormat);

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
  for (int i = 0; i < MAX_AXIS; i++) {
    ctrl.ctrl_aaAxisActions[i].aa_iAxisAction = i + AXIS_SHIFT;
    ctrl.ctrl_aaAxisActions[i].aa_fSensitivity = 80;
    ctrl.ctrl_aaAxisActions[i].aa_fDeadZone = 10;
    ctrl.ctrl_aaAxisActions[i].aa_bInvert = false;
    ctrl.ctrl_aaAxisActions[i].aa_bRelativeControler = true;
    ctrl.ctrl_aaAxisActions[i].aa_bSmooth = false;
    axisValue[i] = 0;
  }

  // load
//  CTString sam_strIntroLevel = "Levels\\LevelsMP\\Intro.wld";
//  CTString sam_strIntroLevel = "Levels/LevelsMP/1_0_InTheLastEpisode.wld";
  CTString sam_strIntroLevel = "Levels/LevelsMP/1_1_Palenque.wld";

  _pShell->SetINDEX("gam_iStartDifficulty", CSessionProperties::GD_NORMAL);
  _pShell->SetINDEX("gam_iStartMode", CSessionProperties::GM_COOPERATIVE);
  game->gm_StartSplitScreenCfg = CGame::SSC_PLAY1;
  game->gm_aiStartLocalPlayers[0] = game->gm_iSinglePlayer;
  game->gm_aiStartLocalPlayers[1] = -1;
  game->gm_aiStartLocalPlayers[2] = -1;
  game->gm_aiStartLocalPlayers[3] = -1;
  game->gm_strNetworkProvider = "Local";
  game->gm_StartSplitScreenCfg = CGame::SSC_PLAY1;

  _pShell->SetINDEX("gam_iStartDifficulty", CSessionProperties::GD_NORMAL);
  _pShell->SetINDEX("gam_iStartMode", CSessionProperties::GM_FLYOVER);

  CUniversalSessionProperties sp;
  game->SetSinglePlayerSession(sp);

  game->gm_bFirstLoading = TRUE;

  if (game->NewGame(sam_strIntroLevel, sam_strIntroLevel, sp)) {
    CPrintF("Started '%s'\n", sam_strIntroLevel);
  } else {
    CPrintF("Demo '%s' NOT STARTED\n", sam_strIntroLevel);
    return;
  }

  CPrintF(TRANS("\n--- Serious Engine CPP End ---\n"));
  CTStream::DisableStreamHandling();
}

void seriousSamInit() {
  if (initialized) return;
  initialized = true;

  try {
    gles_adapter::gles_adp_init();
  } catch (const char *txt) {
    FatalError("OpenGL init error: %s", txt);
  }

  CTStream::EnableStreamHandling();

  // first time
  if (pdp != NULL && pdp->Lock()) {
    pdp->Fill(C_BLACK | CT_OPAQUE);
    pdp->Unlock();
    pvpViewPort->SwapBuffers();
    pdp->Lock();
    pdp->Fill(C_BLACK | CT_OPAQUE);
    pdp->Unlock();
    pvpViewPort->SwapBuffers();
  }

  CTStream::DisableStreamHandling();

  lastTick = getTimeNsec();
}

void seriousSamResize(uint32_t width, uint32_t height) {
  // todo: resize pdp
  glViewport(0, 0, width, height);

  if (!pdp) {
    _pGfx->CreateWindowCanvas(width, height, &pvpViewPort, &pdp);
  }
}

void toggleConsoleState() {
  if (game->gm_csComputerState == CS_ON || game->gm_csComputerState == CS_TURNINGON) {
    game->gm_csComputerState = CS_TURNINGOFF;
    return;
  }
  if (game->gm_csConsoleState == CS_ON || game->gm_csComputerState == CS_TURNINGON || game->gm_csComputerState == CS_TALK) {
    game->gm_csConsoleState = CS_TURNINGOFF;
    return;
  }
}

// automaticaly manage pause toggling
void UpdatePauseState(void)
{
  BOOL bShouldPause = game->gm_csConsoleState == CS_ON ||
                      game->gm_csConsoleState == CS_TURNINGON ||
                      game->gm_csConsoleState == CS_TURNINGOFF ||
                      game->gm_csComputerState == CS_ON ||
                      game->gm_csComputerState == CS_TURNINGON ||
                      game->gm_csComputerState == CS_TURNINGOFF;
  _pNetwork->SetLocalPause(bShouldPause);
}

void setAxisValue(uint32_t key, float value) {
  ASSERT(key >= 0 && key < 10);
  axisValue[key] = value;
}

void processInputs() {
  for (int i = 0; i < 10; i++) {
    _pInput->inp_caiAllAxisInfo[i + AXIS_SHIFT].cai_fReading = axisValue[i];
  }
}

void setUIScale(float scale) {
  g_uiScale = scale;
}

void printProfilingData() {
  CTString totalString;
  CTString it;

  totalString += " ------------------------------------------------------------\n";
  totalString += " ---------------------- PROFILING DATA ----------------------\n";
  totalString += " ------------------------------------------------------------\n";

  _pfRenderProfile.Report(it);
  totalString += "RenderProfile:\n" + it + "\n";

  _pfSoundProfile.Report(it);
  totalString += "SoundProfile:\n" + it + "\n";

  _pfModelProfile.Report(it);
  totalString += "ModelProfile:\n" + it + "\n";

  _pfGfxProfile.Report(it);
  totalString += "GfxProfile:\n" + it + "\n";

  _pfPhysicsProfile.Report(it);
  totalString += "PhysicsProfile:\n" + it + "\n";

  _pfNetworkProfile.Report(it);
  totalString += "NetworkProfile:\n" + it + "\n";

  totalString += "\n";

  CPrintF(totalString);

  _pfRenderProfile.Reset();
  _pfSoundProfile.Reset();
  _pfModelProfile.Reset();
  _pfGfxProfile.Reset();
  _pfPhysicsProfile.Reset();
  _pfNetworkProfile.Reset();
}

void seriousSamDoGame() {
  gles_adapter::enableDraws = true;

  int64_t start = getTimeNsec();
  static int64_t lastFrameDraw = start;
  int64_t deltaFrame = start - lastFrameDraw;
  lastFrameDraw = start;
  int64_t nanoPerTick = (int64_t)(CTimer::TickQuantum * NS_IN_S);
  for (int i = 0; i < 4; i++) {
    if (start > lastTick + nanoPerTick) {
      CTimer_TimerFunc_internal();
      lastTick += nanoPerTick;
      tickCount++;
    } else {
      break;
    }
  }
  if (start > lastTick + nanoPerTick) {
    lastTick = start;
  }

  static int64_t seriousSamDoGameTime = 0;

  const int64_t UPDATE_TIME = 2000000000; // 2s
  static int64_t lastFpsNow = start;
  static int64_t times = 0;
  static float fps;
  if (start - lastFpsNow > UPDATE_TIME) {
    fps = (float) times / (start - lastFpsNow) * 1000 * 1000 * 1000;
    lastFpsNow += UPDATE_TIME;
    times = 0;
  }
  times++;

  CTStream::EnableStreamHandling();

  game->GameMainLoop();

  // todo: draw screen

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

    SLONG slDPWidth = pdp->GetWidth();
    SLONG slDPHeight = pdp->GetHeight();
    pdp->SetFont(_pfdDisplayFont);
    pdp->SetTextScaling(1);
    pdp->SetTextAspect(1.0f);
    CTString str = CTString(0, "fps: %.2f; frame: %.2f ms", fps, deltaFrame / 1000000.f);
    pdp->PutText(str, slDPWidth * 0.05f, slDPHeight * 0.15f, LCDGetColor(C_GREEN | 255, "display mode"));

    pdp->Unlock();

    UpdatePauseState();

//    pdp->Fill(LCDGetColor(C_dGREEN | CT_OPAQUE, "bcg fill"));
//    pdp->Unlock();
  }

  CTStream::DisableStreamHandling();

  // manual call since we never call SwapBuffers
  _pfGfxProfile.IncrementAveragingCounter(1);

  seriousSamDoGameTime = getTimeNsec() - start;
}
