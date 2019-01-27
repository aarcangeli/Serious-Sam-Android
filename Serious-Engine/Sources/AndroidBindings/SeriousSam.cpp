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

typedef CGame *(*GAME_Create_t)(void);

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

CDrawPort *pdp;
CViewPort *pvpViewPort;

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

  if (game->NewGame(sam_strIntroLevel, sam_strIntroLevel, sp)) {
    CPrintF("Started '%s'\n", sam_strIntroLevel);
  } else {
    CPrintF("Demo '%s' NOT STARTED\n", sam_strIntroLevel);
    return;
  }

  const char *string = (const char *) pglGetString(GL_EXTENSIONS);
  _pGfx->CreateWindowCanvas(0, &pvpViewPort, &pdp);

  CPrintF(TRANS("\n--- Serious Engine CPP End ---\n"));
}

void seriousSamInit() {
  // todo: init opengl
  // first time
//  if (pdp != NULL && pdp->Lock()) {
//    pdp->Fill(C_GREEN | CT_OPAQUE);
//    pdp->Unlock();
//    pvpViewPort->SwapBuffers();
//    pdp->Lock();
//    pdp->Fill(C_GREEN | CT_OPAQUE);
//    pdp->Unlock();
//    pvpViewPort->SwapBuffers();
//  }
}

void seriousSamResize(uint32_t width, uint32_t height) {
  pglViewport(0, 0, width, height);
}

void seriousSamDoGame() {
  CTStream::EnableStreamHandling();

  const char *string = (const char *) pglGetString(GL_EXTENSIONS);
  game->GameMainLoop();

  // todo: draw screen

  static float red = 0;
  red += 0.01;
  if (red > 1) red -= 1;
  pglClearColor(red, 0, 0, 1);
  pglClear(GL_COLOR_BUFFER_BIT);

  CTStream::DisableStreamHandling();
}
