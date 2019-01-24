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

void startSeriousSamAndroid() {
  CTStream::EnableStreamHandling();
  SE_InitEngine("");
  SE_LoadDefaultFonts();

  // translations
  InitTranslation();
  try {
    AddTranslationTablesDir_t(CTString("Data\\Translations\\"), CTString("*.txt"));
    FinishTranslationTable();
  } catch (char *strError) {
    FatalError("%s", strError);
  }

  CPrintF(TRANS("\n--- Serious Engine CPP End ---\n"));
}
