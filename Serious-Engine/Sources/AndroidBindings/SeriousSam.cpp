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

void startSeriousSamAndroid() {
  CTStream::EnableStreamHandling();
  SE_InitEngine("");
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
    FatalError("Cannot load GameMP");
  }
  CPrintF(TRANS("libGameMP.so loaded\n"));

  GAME_Create_t GAME_Create = (GAME_Create_t) dlsym(libGameMP, "GAME_Create");
  if (!GAME_Create) {
    FatalError("Cannot find GAME_Create");
  }
  CPrintF(TRANS("GAME_Create found\n"));

  CGame *game = GAME_Create();

  game->Initialize(CTString("Data\\SeriousSam.gms"));

  CPrintF(TRANS("\n--- Serious Engine CPP End ---\n"));
}
