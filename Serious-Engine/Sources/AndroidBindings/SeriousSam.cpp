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

typedef CGame *(*GAME_Create_t)(void);
void CTimer_TimerFunc_internal(void);

namespace gles_adapter {
  void gles_adp_init();

  void syncBuffers();
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

  // todo: sound library
//  snd_iFormat = Clamp( snd_iFormat, (INDEX)CSoundLibrary::SF_NONE, (INDEX)CSoundLibrary::SF_44100_16);
//  _pSound->SetFormat( (enum CSoundLibrary::SoundFormat)snd_iFormat);

  CPrintF("Level list:\n"); // TODO: GetLevelInfo
  CDynamicStackArray <CTFileName> afnmDir;
  MakeDirList(afnmDir, CTString("Levels\\"), "*.wld", DLI_RECURSIVE | DLI_SEARCHCD);
  for (INDEX i = 0; i < afnmDir.Count(); i++) {
    CTFileName fnm = afnmDir[i];
    CPrintF("  level: '%s'\n", fnm);
  }
  CPrintF("\n");

  CPrintF("Demos:\n");
  CDynamicStackArray <CTFileName> demoDir;
  MakeDirList(demoDir, CTString("Demos\\"), "Demos/Auto-*.dem", DLI_RECURSIVE);
  for (INDEX i = 0; i < demoDir.Count(); i++) {
    CTFileName fnm = demoDir[i];
    CPrintF("  level: '%s'\n", fnm);
  }
  CPrintF("\n");

  StartNewMode(GAT_OGL, 0, 640, 480, DD_DEFAULT, false);

  // load
  CTString sam_strIntroLevel = "Levels\\LevelsMP\\Intro.wld";

  game->gm_aiStartLocalPlayers[0] = 0;
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
  game->gm_csConsoleState = CS_TURNINGON;

  if (game->NewGame(sam_strIntroLevel, sam_strIntroLevel, sp)) {
    CPrintF("Started '%s'\n", sam_strIntroLevel);
  } else {
    CPrintF("Demo '%s' NOT STARTED\n", sam_strIntroLevel);
    return;
  }

  CPrintF(TRANS("\n--- Serious Engine CPP End ---\n"));
}

void seriousSamInit() {
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
}

void seriousSamResize(uint32_t width, uint32_t height) {
  // todo: resize pdp
  glViewport(0, 0, width, height);

  if (!pdp) {
    _pGfx->CreateWindowCanvas(width, height, &pvpViewPort, &pdp);
  }
}

void seriousSamDoGame() {
//  glClear(GL_COLOR_BUFFER_BIT);
//
//  float positions[] = {
//    0, 0, 0,
//    0, 1, 0,
//    1, 0, 0,
//  };
//
////  glBindBuffer(GL_ARRAY_BUFFER, 2);
////  glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
////  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
//  glEnableClientState(GL_VERTEX_ARRAY);
//  glVertexPointer(3, GL_FLOAT, 0, positions);
////  glBindBuffer(GL_ARRAY_BUFFER, 0);
//  glColor4f(0, 1, 1, 1);
//
//  printGlError("glVertexAttribPointer");
////  glEnableVertexAttribArray(1);
////  gles_adapter::syncBuffers();
//  glDrawArrays(GL_TRIANGLES, 0, 3);
//  printGlError("glDrawArrays");
//  return;

//  CTStream::EnableStreamHandling();
//  if (pdp != NULL && pdp->Lock()) {
//    pdp->Fill(C_GREEN | CT_OPAQUE);
//    pdp->Unlock();
//    pvpViewPort->SwapBuffers();
//    pdp->Lock();
//    pdp->Fill(C_GREEN | CT_OPAQUE);
//    pdp->Unlock();
//    pvpViewPort->SwapBuffers();
//  }
//  CTStream::DisableStreamHandling();
//
//  return;
  CTimer_TimerFunc_internal();

  CTStream::EnableStreamHandling();

  const char *string = (const char *) pglGetString(GL_EXTENSIONS);
  game->GameMainLoop();

  // todo: draw screen

  if (pdp != NULL && pdp->Lock()) {
    InfoMessage("Frame Start");
    pdp->Fill(C_BLACK | CT_OPAQUE);

    // handle pretouching of textures and shadowmaps
    pdp->Unlock();

//    ULONG ulFlags = (game->gm_csConsoleState != CS_OFF || bMenuActive) ? 0 : GRV_SHOWEXTRAS;
    ULONG ulFlags = 0;
//    pdp->Fill(LCDGetColor(C_dGREEN | CT_OPAQUE, "bcg fill"));

    game->GameRedrawView(pdp, ulFlags);

    pdp->Lock();
//    game->ComputerRender(pdp);
//    game->ConsoleRender(pdp);
    pdp->Unlock();

//    pdp->Fill(LCDGetColor(C_dGREEN | CT_OPAQUE, "bcg fill"));
//    pdp->Unlock();
  }

  CTStream::DisableStreamHandling();
}
