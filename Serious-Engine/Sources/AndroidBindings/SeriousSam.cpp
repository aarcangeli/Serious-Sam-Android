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

typedef CGame *(*GAME_Create_t)(void);

void StartNewMode() {
  CPrintF(TRANS("\n* START NEW DISPLAY MODE ...\n"));
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

  CGame *game = GAME_Create();
  game->Initialize(CTString("Data\\SeriousSam.gms"));
  game->LCDInit();

  // todo: sound library

  CPrintF("Level list:\n");
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

  if (game->NewGame( sam_strIntroLevel, sam_strIntroLevel, sp)) {
    CPrintF("Started '%s'\n", sam_strIntroLevel);
  } else {
    CPrintF("Demo '%s' NOT STARTED\n", sam_strIntroLevel);
    return;
  }

  CPrintF(TRANS("\n--- Serious Engine CPP End ---\n"));
}
