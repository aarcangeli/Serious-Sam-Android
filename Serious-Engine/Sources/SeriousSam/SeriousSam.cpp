/* Copyright (c) 2002-2012 Croteam Ltd.
This program is free software; you can redistribute it and/or modify
it under the terms of version 2 of the GNU General Public License as published by
the Free Software Foundation


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA. */

#include "StdH.h"
#include <AndroidBindings/bindings.h>
#include <AndroidAdapters/gles_adapter.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <Engine/CurrentVersion.h>
#include <GameMP/Game.h>
#define DECL_DLL
#include <EntitiesMP/Global.h>
#include <SeriousSam/GUI/Menus/MenuManager.h>
#include "resource.h"
#include "SplashScreen.h"
#include "MainWindow.h"
#include "GlSettings.h"
#include "LevelInfo.h"
#include "LCDDrawing.h"
#include "CmdLine.h"
#include "Credits.h"
#include "config.h"

typedef CGame *(*GAME_Create_t)(void);
void drawBannerFpsVersion(CDrawPort *pdp, int64_t deltaFrame, float fps);

// _pGame reused from GameMP module
#ifndef STATIC_LINKING
extern CGame *_pGame = NULL;
#endif

// application state variables
extern BOOL _bRunning = TRUE;
extern BOOL _bExitQuitScreen = TRUE;
extern BOOL _bQuitScreen = TRUE;
extern BOOL bMenuActive = FALSE;
extern BOOL bMenuRendering = FALSE;

extern BOOL _bDefiningKey;
static BOOL _bReconsiderInput = FALSE;
extern PIX  _pixDesktopWidth = 0;    // desktop width when started (for some tests)

static INDEX sam_iMaxFPSActive   = 500;
static INDEX sam_iMaxFPSInactive = 10;
static INDEX sam_bPauseOnMinimize = TRUE; // auto-pause when window has been minimized
extern INDEX sam_bWideScreen = FALSE;
extern FLOAT sam_fPlayerOffset = 0.0f;

// display mode settings
extern INDEX sam_bFullScreenActive = FALSE;
extern INDEX sam_iScreenSizeI = 640;  // current size of the window
extern INDEX sam_iScreenSizeJ = 480;  // current size of the window
extern INDEX sam_iDisplayDepth  = 0;  // 0==default, 1==16bit, 2==32bit
extern INDEX sam_iDisplayAdapter = 0;
extern INDEX sam_iGfxAPI = 0;         // 0==OpenGL
extern INDEX sam_bFirstStarted = FALSE;
extern FLOAT sam_tmDisplayModeReport = 5.0f;
extern INDEX sam_bShowAllLevels = FALSE;
extern INDEX sam_bMentalActivated = FALSE;

// network settings
extern CTString sam_strNetworkSettings = "";
// command line
extern CTString sam_strCommandLine = "";

// 0...app started for the first time
// 1...all ok
// 2...automatic fallback
static INDEX _iDisplayModeChangeFlag = 0;
static TIME _tmDisplayModeChanged = 100.0f; // when display mode was last changed

// rendering preferences for automatic settings
extern INDEX sam_iVideoSetup = 1;  // 0==speed, 1==normal, 2==quality, 3==custom
// automatic adjustment of audio quality
extern BOOL sam_bAutoAdjustAudio = TRUE;

extern INDEX sam_bAutoPlayDemos = TRUE;
static INDEX _bInAutoPlayLoop = TRUE;

// menu calling
extern INDEX sam_bMenu         = FALSE;
extern INDEX sam_bMenuSave     = FALSE;
extern INDEX sam_bMenuLoad     = FALSE;
extern INDEX sam_bMenuControls = FALSE;
extern INDEX sam_bMenuHiScore  = FALSE;
extern INDEX sam_bToggleConsole = FALSE;
extern INDEX sam_iStartCredits = FALSE;

// for mod re-loading
extern CTFileName _fnmModToLoad = CTString("");
extern CTString _strModServerJoin = CTString("");
extern CTString _strURLToVisit = CTString("");
extern CTFileName _modToLoadTxt = CTString("ModToLoad.txt");

// state variables fo addon execution
// 0 - nothing
// 1 - start (invoke console)
// 2 - console invoked, waiting for one redraw
extern INDEX _iAddonExecState = 0;
extern CTFileName _fnmAddonToExec = CTString("");

// logo textures
static CTextureObject  _toLogoCT;
static CTextureObject  _toLogoODI;
static CTextureObject  _toLogoEAX;
extern CTextureObject *_ptoLogoCT  = NULL;
extern CTextureObject *_ptoLogoODI = NULL;
extern CTextureObject *_ptoLogoEAX = NULL;

extern CTString sam_strVersion = SSA_VERSION;
extern CTString sam_strModName = TRANS("-   A N D R O I D   P O R T   ( U N O F F I C I A L )   -");
extern CTString sam_strBackLink = TRANS("https://github.com/aarcangeli/Serious-Sam-Android");

extern CTFileName sam_strFirstLevel = CTString("Levels\\LevelsMP\\1_0_InTheLastEpisode.wld");
extern CTFileName sam_strIntroLevel = CTString("Levels\\LevelsMP\\Intro.wld");
extern CTString sam_strGameName = "serioussamse";

extern CTString sam_strTechTestLevel = "Levels\\LevelsMP\\Technology\\TechTest.wld";
extern CTString sam_strTrainingLevel = "Levels\\KarnakDemo.wld";

ENGINE_API extern INDEX snd_iFormat;


// main window canvas
CDrawPort *pdp;
CDrawPort *pdpNormal;
CDrawPort *pdpWideScreen;
CViewPort *pvpViewPort;

void StartNextDemo(void);
BOOL _bWindowChanging = FALSE;
void CloseMainWindow() {
  // nope
}
void OpenMainWindowFullScreen(PIX pixSizeI, PIX pixSizeJ) {
  // nope
}
void OpenMainWindowNormal(PIX pixSizeI, PIX pixSizeJ) {
  // nope
}
void MainWindow_End() {
  // nope
}

bool ignoreTouchInputs = false;

void TouchDown(void* pArgs) {
  INDEX x = NEXTARGUMENT(INDEX);
  INDEX y = NEXTARGUMENT(INDEX);

  _pGame->isTouchingScreen = true;

  if (g_cb.gameState == GS_LOADING) {
    ignoreTouchInputs = true;
    return;
  }
  ignoreTouchInputs = false;

  if (bMenuActive) {
    MenuOnMouseMove(x, y);
    return;
  }

  if (_pGame->gm_csComputerState != CS_OFF && _pGame->gm_csComputerState != CS_ONINBACKGROUND) {
    _pGame->ComputerMouseMove(x, y);
    MSG msg;
    msg.message = WM_LBUTTONDOWN;
    _pGame->ComputerKeyDown(msg);
    return;
  }
}

void TouchMove(void* pArgs) {
  INDEX x = NEXTARGUMENT(INDEX);
  INDEX y = NEXTARGUMENT(INDEX);

  if (ignoreTouchInputs) {
    return;
  }

  if (bMenuActive) {
    MenuOnMouseMove(x, y);
    return;
  }

  if (_pGame->gm_csComputerState != CS_OFF && _pGame->gm_csComputerState != CS_ONINBACKGROUND) {
    _pGame->ComputerMouseMove(x, y);
    return;
  }
}

void TouchUp(void* pArgs) {
  INDEX x = NEXTARGUMENT(INDEX);
  INDEX y = NEXTARGUMENT(INDEX);

  _pGame->isTouchingScreen = false;

  if (ignoreTouchInputs) {
    return;
  }

  if (_gmRunningGameMode == GM_DEMO || _gmRunningGameMode == GM_INTRO) {
    if (!bMenuActive && !bMenuRendering && _pGame->gm_csConsoleState == CS_OFF) {
      _pGame->StopGame();
      _gmRunningGameMode = GM_NONE;
      StartMenus();
      return;
    }
  }

  if (!_bRunning) {
    // stop menu
    _bExitQuitScreen = true;
    return;
  }

  if (bMenuActive) {
    MenuOnMouseMove(x, y);
    MenuOnKeyDown(VK_LBUTTON);
    return;
  }

  if (_pGame->gm_csComputerState != CS_OFF && _pGame->gm_csComputerState != CS_ONINBACKGROUND) {
    _pGame->ComputerMouseMove(x, y);
    MSG msg;
    msg.message = WM_LBUTTONUP;
    _pGame->ComputerKeyDown(msg);
    return;
  }
}

void SaveOptions(void* pArgs) {
  CTFileName fnmPersistentSymbols = CTString("Scripts\\PersistentSymbols.ini");
  _pShell->StorePersistentSymbols(fnmPersistentSymbols);
}

void ExitConfirm(void);
void GoMenuBack() {
  if (bMenuActive) {
    if (&_pGUIM->gmMainMenu == pgmCurrentMenu) {
      ExitConfirm();
    } else {
      MenuOnKeyDown(VK_ESCAPE);
    }
  }
}

void ToggleConsole() {
  if (_pGame->gm_csConsoleState == CS_OFF || _pGame->gm_csConsoleState == CS_TURNINGOFF) {
    _pGame->gm_csConsoleState = CS_TURNINGON;
    if (bMenuActive) {
      StopMenus(FALSE);
    }
  } else if (_pGame->gm_csConsoleState == CS_ON || _pGame->gm_csConsoleState == CS_TURNINGON) {
    _pGame->gm_csConsoleState = CS_TURNINGOFF;
  }
}

void HideConsole() {
  if (_pGame->gm_csConsoleState == CS_ON || _pGame->gm_csConsoleState == CS_TURNINGON) {
    _pGame->gm_csConsoleState = CS_TURNINGOFF;
  }
}

void HideComputer() {
  if (_pGame->gm_csComputerState == CS_ON || _pGame->gm_csComputerState == CS_TURNINGON) {
    _pGame->gm_csComputerState = CS_TURNINGOFF;
  }
}

void ViewportResized() {
  _tmDisplayModeChanged = _pTimer->GetRealTimeTick();
}

void MenuEvent(void* pArgs) {
  INDEX vk = NEXTARGUMENT(INDEX);
  MenuOnKeyDown(vk);

  if (_pGame->gm_csConsoleState != CS_OFF && _pGame->gm_csConsoleState != CS_ONINBACKGROUND) {
    MSG msg;
    msg.message = WM_KEYDOWN;
    msg.wParam = vk;
    _pGame->ConsoleKeyDown(msg);
    return;
  }
}

void MenuChar(void* pArgs) {
  INDEX keyCode = NEXTARGUMENT(INDEX);
  MSG msg;
  msg.message = WM_CHAR;
  msg.wParam = keyCode;

  if (bMenuActive && !_pInput->IsInputEnabled()) {
    MenuOnChar(msg);
  }

  _pGame->ConsoleChar(msg);
}

static void PlayDemo(void* pArgs)
{
  CTString strDemoFilename = *NEXTARGUMENT(CTString*);
  _gmMenuGameMode = GM_DEMO;
  CTFileName fnDemo = "demos\\" + strDemoFilename + ".dem";
  extern BOOL LSLoadDemo(const CTFileName &fnm);
  LSLoadDemo(fnDemo);
}

static void ApplyRenderingPreferences(void)
{
  ApplyGLSettings(TRUE);
}

extern void ApplyVideoMode(void)
{
  StartNewMode( (GfxAPIType)sam_iGfxAPI, sam_iDisplayAdapter, sam_iScreenSizeI, sam_iScreenSizeJ,
                (enum DisplayDepth)sam_iDisplayDepth, sam_bFullScreenActive);
}

static void QuitGame(void)
{
  _bRunning = FALSE;
  _bQuitScreen = FALSE;
}

void End(void);

// automaticaly manage input enable/disable toggling
static BOOL _bInputEnabled = FALSE;
void UpdateInputEnabledState(void)
{
  // do nothing if window is invalid
  if( _hwndMain==NULL) return;

#if 0
  // input should be enabled if application is active
  // and no menu is active and no console is active
  BOOL bShouldBeEnabled = (!IsIconic(_hwndMain) && !bMenuActive && _pGame->gm_csConsoleState==CS_OFF
                       && (_pGame->gm_csComputerState==CS_OFF || _pGame->gm_csComputerState==CS_ONINBACKGROUND))
                       || _bDefiningKey;

  // if should be turned off
  if( (!bShouldBeEnabled && _bInputEnabled) || _bReconsiderInput) {
    // disable it and remember new state
    _pInput->DisableInput();
    _bInputEnabled = FALSE;
  }
  // if should be turned on
  if( bShouldBeEnabled && !_bInputEnabled) {
    // enable it and remember new state
    _pInput->EnableInput(_hwndMain);
    _bInputEnabled = TRUE;
  }
  _bReconsiderInput = FALSE;
#endif
}


// automaticaly manage pause toggling
void UpdatePauseState(void)
{
  BOOL bShouldPause = (_gmRunningGameMode==GM_SINGLE_PLAYER) && (bMenuActive || 
                       _pGame->gm_csConsoleState ==CS_ON || _pGame->gm_csConsoleState ==CS_TURNINGON || _pGame->gm_csConsoleState ==CS_TURNINGOFF ||
                       _pGame->gm_csComputerState==CS_ON || _pGame->gm_csComputerState==CS_TURNINGON || _pGame->gm_csComputerState==CS_TURNINGOFF);
  _pNetwork->SetLocalPause(bShouldPause);
}


// limit current frame rate if neeeded
void LimitFrameRate(void)
{
  // measure passed time for each loop
  static CTimerValue tvLast(-1.0f);
  CTimerValue tvNow   = _pTimer->GetHighPrecisionTimer();
  TIME tmCurrentDelta = (tvNow-tvLast).GetSeconds();

  // limit maximum frame rate
  sam_iMaxFPSActive   = ClampDn( (INDEX)sam_iMaxFPSActive,   1L);
  sam_iMaxFPSInactive = ClampDn( (INDEX)sam_iMaxFPSInactive, 1L);
  INDEX iMaxFPS = sam_iMaxFPSActive;
  if(_pGame->gm_CurrentSplitScreenCfg==CGame::SSC_DEDICATED) {
    iMaxFPS = ClampDn(iMaxFPS, 60L); // never go very slow if dedicated server
  }
  TIME tmWantedDelta = 1.0f / iMaxFPS;
  if( tmCurrentDelta<tmWantedDelta) Sleep( (tmWantedDelta-tmCurrentDelta)*1000.0f);
  
  // remember new time
  tvLast = _pTimer->GetHighPrecisionTimer();
}

// load first demo
void StartNextDemo(void)
{
  if (!sam_bAutoPlayDemos || !_bInAutoPlayLoop) {
    _bInAutoPlayLoop = FALSE;
    return;
  }

  // skip if no demos
  if(_lhAutoDemos.IsEmpty()) {
    _bInAutoPlayLoop = FALSE;
    return;
  }

  // get first demo level and cycle the list
  CLevelInfo *pli = LIST_HEAD(_lhAutoDemos, CLevelInfo, li_lnNode);
  pli->li_lnNode.Remove();
  _lhAutoDemos.AddTail(pli->li_lnNode);

  // if intro
  if (pli->li_fnLevel==sam_strIntroLevel) {
    // start intro
    _gmRunningGameMode = GM_NONE;
    _pGame->gm_aiStartLocalPlayers[0] = 0;
    _pGame->gm_aiStartLocalPlayers[1] = -1;
    _pGame->gm_aiStartLocalPlayers[2] = -1;
    _pGame->gm_aiStartLocalPlayers[3] = -1;
    _pGame->gm_strNetworkProvider = "Local";
    _pGame->gm_StartSplitScreenCfg = CGame::SSC_PLAY1;

    _pShell->SetINDEX("gam_iStartDifficulty", CSessionProperties::GD_NORMAL);
    _pShell->SetINDEX("gam_iStartMode", CSessionProperties::GM_FLYOVER);

    CUniversalSessionProperties sp;
    _pGame->SetSinglePlayerSession(sp);

    _pGame->gm_bFirstLoading = TRUE;

    if (_pGame->NewGame( sam_strIntroLevel, sam_strIntroLevel, sp)) {
      _gmRunningGameMode = GM_INTRO;
    }
  // if not intro
  } else {
    // start the demo
    _pGame->gm_StartSplitScreenCfg = CGame::SSC_OBSERVER;
    _pGame->gm_aiStartLocalPlayers[0] = -1;
    _pGame->gm_aiStartLocalPlayers[1] = -1;
    _pGame->gm_aiStartLocalPlayers[2] = -1;
    _pGame->gm_aiStartLocalPlayers[3] = -1;
    // play the demo
    _pGame->gm_strNetworkProvider = "Local";
    _gmRunningGameMode = GM_NONE;
    if( _pGame->StartDemoPlay( pli->li_fnLevel)) {
      _gmRunningGameMode = GM_DEMO;
      CON_DiscardLastLineTimes();
    }
  }

  if (_gmRunningGameMode==GM_NONE) {
    _bInAutoPlayLoop = FALSE;
  }
}

BOOL _bCDPathFound = FALSE;

BOOL FileExistsOnHD(const CTString &strFile)
{
  FILE *f = fopen(_fnmApplicationPath+strFile, "rb");
  if (f!=NULL) {
    fclose(f);
    return TRUE;
  } else {
    return FALSE;
  }
}

void TrimString(char *str)
{
  int i = strlen(str);
  if (str[i-1]=='\n' || str[i-1]=='\r') {
    str[i-1]=0;
  }
}


void LoadAndForceTexture(CTextureObject &to, CTextureObject *&pto, const CTFileName &fnm)
{
  try {
    to.SetData_t(fnm);
    CTextureData *ptd = (CTextureData*)to.GetData();
    ptd->Force( TEX_CONSTANT);
    ptd = ptd->td_ptdBaseTexture;
    if( ptd!=NULL) ptd->Force( TEX_CONSTANT);
    pto = &to;
  } catch ( const char *pchrError) {
    (void*)pchrError;
    pto = NULL;
  }
}

#ifdef STATIC_LINKING
extern "C" CGame *GAME_Create(void);
#endif

void InitializeGame(void)
{
  try {

#ifndef STATIC_LINKING
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
#endif

    _pGame = GAME_Create();

  } catch ( const char *strError) {
    FatalError("%s", strError);
  }
  // init game - this will load persistent symbols
  _pGame->Initialize(CTString("Data\\SeriousSam.gms"));
}

BOOL Init()
{

  // this is not used in android
  _pixDesktopWidth = 0;

  // initialize engine
  SE_InitEngine(sam_strGameName);

  SE_LoadDefaultFonts();

  // load all translation tables
  InitTranslation();
  try {
    AddTranslationTablesDir_t(CTString("Data\\Translations\\"), CTString("*.txt"));
    FinishTranslationTable();
  } catch ( const char *strError) {
    FatalError("%s", strError);
  }

  // always disable all warnings when in serious sam
  _pShell->Execute( "con_bNoWarnings=1;");

  // declare shell symbols
  _pShell->DeclareSymbol("user void PlayDemo(CTString);", (void *) &PlayDemo);
  _pShell->DeclareSymbol("persistent INDEX sam_bFullScreen;", (void *)   &sam_bFullScreenActive);
  _pShell->DeclareSymbol("persistent INDEX sam_iScreenSizeI;", (void *)  &sam_iScreenSizeI);
  _pShell->DeclareSymbol("persistent INDEX sam_iScreenSizeJ;", (void *)  &sam_iScreenSizeJ);
  _pShell->DeclareSymbol("persistent INDEX sam_iDisplayDepth;", (void *) &sam_iDisplayDepth);
  _pShell->DeclareSymbol("persistent INDEX sam_iDisplayAdapter;", (void *) &sam_iDisplayAdapter);
  _pShell->DeclareSymbol("persistent INDEX sam_iGfxAPI;", (void *)         &sam_iGfxAPI);
  _pShell->DeclareSymbol("persistent INDEX sam_bFirstStarted;", (void *) &sam_bFirstStarted);
  _pShell->DeclareSymbol("persistent INDEX sam_bAutoAdjustAudio;", (void *) &sam_bAutoAdjustAudio);
  _pShell->DeclareSymbol("persistent user INDEX sam_bWideScreen;", (void *) &sam_bWideScreen);
  _pShell->DeclareSymbol("persistent user FLOAT sam_fPlayerOffset;", (void *)  &sam_fPlayerOffset);
  _pShell->DeclareSymbol("persistent user INDEX sam_bAutoPlayDemos;", (void *) &sam_bAutoPlayDemos);
  _pShell->DeclareSymbol("persistent user INDEX sam_iMaxFPSActive;", (void *)    &sam_iMaxFPSActive);
  _pShell->DeclareSymbol("persistent user INDEX sam_iMaxFPSInactive;", (void *)  &sam_iMaxFPSInactive);
  _pShell->DeclareSymbol("persistent user INDEX sam_bPauseOnMinimize;", (void *) &sam_bPauseOnMinimize);
  _pShell->DeclareSymbol("persistent user FLOAT sam_tmDisplayModeReport;", (void *)   &sam_tmDisplayModeReport);
  _pShell->DeclareSymbol("persistent user CTString sam_strNetworkSettings;", (void *) &sam_strNetworkSettings);
  _pShell->DeclareSymbol("persistent user CTString sam_strIntroLevel;", (void *)      &sam_strIntroLevel);
  _pShell->DeclareSymbol("persistent user CTString sam_strGameName;", (void *)      &sam_strGameName);
  _pShell->DeclareSymbol("user CTString sam_strVersion;", (void *)    &sam_strVersion);
  _pShell->DeclareSymbol("user CTString sam_strFirstLevel;", (void *) &sam_strFirstLevel);
  _pShell->DeclareSymbol("user CTString sam_strModName;", (void *) &sam_strModName);
  _pShell->DeclareSymbol("user CTString sam_strBackLink;", (void *) &sam_strBackLink);
  _pShell->DeclareSymbol("persistent INDEX sam_bShowAllLevels;", (void *) &sam_bShowAllLevels);
  _pShell->DeclareSymbol("persistent INDEX sam_bMentalActivated;", (void *) &sam_bMentalActivated);

  _pShell->DeclareSymbol("user CTString sam_strTechTestLevel;", (void *) &sam_strTechTestLevel);
  _pShell->DeclareSymbol("user CTString sam_strTrainingLevel;", (void *) &sam_strTrainingLevel);

  _pShell->DeclareSymbol("user void Quit(void);", (void *) &QuitGame);

  _pShell->DeclareSymbol("persistent user INDEX sam_iVideoSetup;", (void *)     &sam_iVideoSetup);
  _pShell->DeclareSymbol("user void ApplyRenderingPreferences(void);", (void *) &ApplyRenderingPreferences);
  _pShell->DeclareSymbol("user void ApplyVideoMode(void);", (void *)            &ApplyVideoMode);

  _pShell->DeclareSymbol("user INDEX sam_bMenu;", (void *)         &sam_bMenu);
  _pShell->DeclareSymbol("user INDEX sam_bMenuSave;", (void *)     &sam_bMenuSave);
  _pShell->DeclareSymbol("user INDEX sam_bMenuLoad;", (void *)     &sam_bMenuLoad);
  _pShell->DeclareSymbol("user INDEX sam_bMenuControls;", (void *) &sam_bMenuControls);
  _pShell->DeclareSymbol("user INDEX sam_bMenuHiScore;", (void *)  &sam_bMenuHiScore);
  _pShell->DeclareSymbol("user INDEX sam_bToggleConsole;",&sam_bToggleConsole);
  _pShell->DeclareSymbol("INDEX sam_iStartCredits;", (void *) &sam_iStartCredits);

  _pShell->DeclareSymbol("user void TouchMove(INDEX, INDEX);", (void *) &TouchMove);
  _pShell->DeclareSymbol("user void TouchDown(INDEX, INDEX);", (void *) &TouchDown);
  _pShell->DeclareSymbol("user void TouchUp(INDEX, INDEX);", (void *) &TouchUp);
  _pShell->DeclareSymbol("user void SaveOptions();", (void *) &SaveOptions);
  _pShell->DeclareSymbol("user void MenuEvent(INDEX);", (void *) &MenuEvent);
  _pShell->DeclareSymbol("user void MenuChar(INDEX);", (void *) &MenuChar);
  _pShell->DeclareSymbol("user void GoMenuBack();", (void *) &GoMenuBack);
  _pShell->DeclareSymbol("user void ToggleConsole();", (void *) &ToggleConsole);
  _pShell->DeclareSymbol("user void HideConsole();", (void *) &HideConsole);
  _pShell->DeclareSymbol("user void HideComputer();", (void *) &HideComputer);
  _pShell->DeclareSymbol("user void ViewportResized();", (void *) &ViewportResized);
  _pShell->DeclareSymbol("INDEX input_iIsShiftPressed;", (void *) &g_cb.isShiftPressed);
  _pShell->DeclareSymbol("FLOAT input_uiScale;", (void *) &g_cb.globalScale);

  InitializeGame();
  _pNetwork->md_strGameID = sam_strGameName;

  LCDInit();

  if( sam_bFirstStarted) {
    InfoMessage("%s", TRANS(
      "SeriousSam is starting for the first time.\n"
      "If you experience any problems, please consult\n"
      "ReadMe file for troubleshooting information."));
  }

  // initialize sound library
#if 0
  snd_iFormat = Clamp( snd_iFormat, (INDEX)CSoundLibrary::SF_NONE, (INDEX)CSoundLibrary::SF_44100_16);
  _pSound->SetFormat( (enum CSoundLibrary::SoundFormat)snd_iFormat);
#endif

  if (sam_bAutoAdjustAudio) {
    _pShell->Execute("include \"Scripts\\Addons\\SFX-AutoAdjust.ini\"");
  }

  // execute script given on command line
#if 0
  if (cmd_strScript!="") {
    CPrintF("Command line script: '%s'\n", cmd_strScript);
    CTString strCmd;
    strCmd.PrintF("include \"%s\"", cmd_strScript);
    _pShell->Execute(strCmd);
  }
#endif

  // load logo textures
  LoadAndForceTexture(_toLogoCT,   _ptoLogoCT,   CTFILENAME("Textures\\Logo\\LogoCT.tex"));
  LoadAndForceTexture(_toLogoODI,  _ptoLogoODI,  CTFILENAME("Textures\\Logo\\GodGamesLogo.tex"));
  LoadAndForceTexture(_toLogoEAX,  _ptoLogoEAX,  CTFILENAME("Textures\\Logo\\LogoEAX.tex"));

  // !! NOTE !! Re-enable these to allow mod support.
  LoadStringVar(CTString("Data\\Var\\Sam_Version.var"), sam_strVersion);
  LoadStringVar(CTString("Data\\Var\\ModName.var"), sam_strModName);
  CPrintF(TRANS("Serious Sam version: %s\n"), sam_strVersion);
  CPrintF(TRANS("Active mod: %s\n"), sam_strModName);
  InitializeMenus();      
  
  // if there is a mod
  if (_fnmMod!="") {
    // execute the mod startup script
    _pShell->Execute(CTString("include \"Scripts\\Mod_startup.ini\";"));
  }

  // init gl settings module
  InitGLSettings();

  // init level-info subsystem
  LoadLevelsList();
  LoadDemosList();

  // apply application mode
  StartNewMode( (GfxAPIType)sam_iGfxAPI, sam_iDisplayAdapter, sam_iScreenSizeI, sam_iScreenSizeJ,
                (enum DisplayDepth)sam_iDisplayDepth, sam_bFullScreenActive);

  gles_adapter::gles_adp_init();

  // set default mode reporting
  if( sam_bFirstStarted) {
    _iDisplayModeChangeFlag = 0;
    sam_bFirstStarted = FALSE;
  }

  if (cmd_strPassword!="") {
    _pShell->SetString("net_strConnectPassword", cmd_strPassword);
  }

#if 0
  // if connecting to server from command line
  if (cmd_strServer!="") {
    CTString strPort = "";
    if (cmd_iPort>0) {
      _pShell->SetINDEX("net_iPort", cmd_iPort);
      strPort.PrintF(":%d", cmd_iPort);
    }
    CPrintF(TRANS("Command line connection: '%s%s'\n"), cmd_strServer, strPort);
    // go to join menu
    _pGame->gam_strJoinAddress = cmd_strServer;
    if (cmd_bQuickJoin) {
      extern void JoinNetworkGame(void);
      JoinNetworkGame();
    } else {
      StartMenus("join");
    }
  // if starting world from command line
  } else if (cmd_strWorld!="") {
    CPrintF(TRANS("Command line world: '%s'\n"), cmd_strWorld);
    // try to start the game with that level
    try {
      if (cmd_iGoToMarker>=0) {
        CPrintF(TRANS("Command line marker: %d\n"), cmd_iGoToMarker);
        CTString strCommand;
        strCommand.PrintF("cht_iGoToMarker = %d;", cmd_iGoToMarker);
        _pShell->Execute(strCommand);
      }
      _pGame->gam_strCustomLevel = cmd_strWorld;
      if (cmd_bServer) {
        extern void StartNetworkGame(void);
        StartNetworkGame();
      } else {
        extern void StartSinglePlayerGame(void);
        StartSinglePlayerGame();
      }
    } catch ( const char *strError) {
      CPrintF(TRANS("Cannot start '%s': '%s'\n"), cmd_strWorld, strError);
    }
  // if no relevant starting at command line
  } else {
    StartNextDemo();
  }
#endif

  return TRUE;
}


void End(void)
{
  _pGame->DisableLoadingHook();
  // cleanup level-info subsystem
  ClearLevelsList();
  ClearDemosList();

  // destroy the main window and its canvas
  if (pvpViewPort!=NULL) {
    _pGfx->DestroyWindowCanvas( pvpViewPort);
    pvpViewPort = NULL;
    pdpNormal   = NULL;
  }

  CloseMainWindow();
  MainWindow_End();
  DestroyMenus();
  _pGame->End();
  LCDEnd();
  // unlock the directory
  SE_EndEngine();
}


// print display mode info if needed
void PrintDisplayModeInfo(void)
{
  // skip if timed out
  if( _pTimer->GetRealTimeTick() > (_tmDisplayModeChanged+sam_tmDisplayModeReport)) return;

  // cache some general vars
  SLONG slDPWidth  = pdp->GetWidth();
  SLONG slDPHeight = pdp->GetHeight();
  if( pdp->IsDualHead()) slDPWidth/=2;

  CDisplayMode dm;
  dm.dm_pixSizeI = slDPWidth;
  dm.dm_pixSizeJ = slDPHeight;
  // determine proper text scale for statistics display
  FLOAT fTextScale = (FLOAT)slDPWidth/640.0f;

  // get resolution
  CTString strRes;
  extern CTString _strPreferencesDescription;
  strRes.PrintF( "%dx%dx%s", slDPWidth, slDPHeight, _pGfx->gl_dmCurrentDisplayMode.DepthString());
  if( dm.IsDualHead())   strRes += TRANS(" DualMonitor");
  if( dm.IsWideScreen()) strRes += TRANS(" WideScreen");
       if( _pGfx->gl_eCurrentAPI==GAT_OGL) strRes += " (OpenGL)";
#ifdef SE1_D3D
  else if( _pGfx->gl_eCurrentAPI==GAT_D3D) strRes += " (Direct3D)";
#endif // SE1_D3D

  CTString strDescr;
  strDescr.PrintF("\n%s (%s)\n", _strPreferencesDescription, RenderingPreferencesDescription(sam_iVideoSetup));
  strRes+=strDescr;
  // tell if application is started for the first time, or failed to set mode
  if( _iDisplayModeChangeFlag==0) {
    strRes += TRANS("Display mode set by default!");
  } else if( _iDisplayModeChangeFlag==2) {
    strRes += TRANS("Last mode set failed!");
  }

  // print it all
  pdp->SetFont( _pfdDisplayFont);
  pdp->SetTextScaling( fTextScale);
  pdp->SetTextAspect( 1.0f);
  pdp->PutText( strRes, slDPWidth*0.05f, slDPHeight*0.85f, LCDGetColor(C_GREEN|255, "display mode"));
}

// do the main game loop and render screen
void DoGame(void)
{
  // set flag if not in game
  if( !_pGame->gm_bGameOn) _gmRunningGameMode = GM_NONE;

  if( _gmRunningGameMode==GM_DEMO  && _pNetwork->IsDemoPlayFinished()
      ||_gmRunningGameMode==GM_INTRO && _pNetwork->IsGameFinished()) {
    _pGame->StopGame();
    _gmRunningGameMode = GM_NONE;

    // load next demo
    StartNextDemo();
    if (!_bInAutoPlayLoop) {
      // start menu
      StartMenus();
    }
  }

  // update fps
  int64_t start = getTimeNsec();
  static int64_t lastFrameDraw = start;
  int64_t deltaFrame = start - lastFrameDraw;
  lastFrameDraw = start;

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


  // do the main game loop
  if( _gmRunningGameMode != GM_NONE) {
    _pGame->GameMainLoop();
    // if game is not started
  } else {
    // just handle broadcast messages
    _pNetwork->GameInactive();
  }

  if (sam_iStartCredits>0) {
    Credits_On(sam_iStartCredits);
    sam_iStartCredits = 0;
  }
  if (sam_iStartCredits<0) {
    Credits_Off();
    sam_iStartCredits = 0;
  }
  if( _gmRunningGameMode==GM_NONE) {
    Credits_Off();
    sam_iStartCredits = 0;
  }

  if (sam_bToggleConsole) {
    sam_bToggleConsole = false;
    ToggleConsole();
  }

  if (pdp!=NULL && pdp->Lock()) {
    pdp->Fill(C_BLACK | CT_OPAQUE);

    if (_gmRunningGameMode != GM_NONE && !bMenuActive) {
      pdp->Unlock();
      _pGame->GameRedrawView( pdp, (_pGame->gm_csConsoleState!=CS_OFF || bMenuActive)?0:GRV_SHOWEXTRAS);
      pdp->Lock();
      _pGame->ComputerRender(pdp);
      pdp->Unlock();
      CDrawPort dpScroller(pdp, TRUE);
      dpScroller.Lock();
      if (Credits_Render(&dpScroller)==0) {
        Credits_Off();
      }
      dpScroller.Unlock();
      pdp->Lock();
    } else {
      pdp->Fill( LCDGetColor(C_dGREEN|CT_OPAQUE, "bcg fill"));
    }

    // do menu
    if( bMenuRendering) {
      // clear z-buffer
      pdp->FillZBuffer( ZBUF_BACK);
      // remember if we should render menus next tick
      bMenuRendering = DoMenu(pdp);
    }

    // print display mode info if needed
    PrintDisplayModeInfo();

    // render console
    _pGame->ConsoleRender(pdp);

    // draw fps and frame time
    if (!bMenuActive && _pGame->gm_csConsoleState == CS_OFF && _pGame->gm_csComputerState == CS_OFF) {
      drawBannerFpsVersion(pdp, deltaFrame, fps);
    }

    if (_gmRunningGameMode == GM_INTRO) {
      g_cb.setSeriousState(GS_INTRO);
    } else if (_gmRunningGameMode == GM_DEMO) {
      g_cb.setSeriousState(GS_DEMO);
    } else if (_pGame->gm_csConsoleState != CS_OFF) {
      g_cb.setSeriousState(GS_CONSOLE);
    } else if (_pGame->gm_csComputerState != CS_OFF) {
      g_cb.setSeriousState(GS_COMPUTER);
    } else if (bMenuActive) {
      g_cb.setSeriousState(GS_MENU);
    } else {
      g_cb.setSeriousState(GS_NORMAL);
    }

    // done with all
    pdp->Unlock();

    // clear upper and lower parts of screen if in wide screen mode
    if( pdp==pdpWideScreen && pdpNormal->Lock()) {
      const PIX pixWidth  = pdpWideScreen->GetWidth();
      const PIX pixHeight = (pdpNormal->GetHeight() - pdpWideScreen->GetHeight()) /2;
      const PIX pixJOfs   = pixHeight + pdpWideScreen->GetHeight()-1;
      pdpNormal->Fill( 0, 0,       pixWidth, pixHeight, C_BLACK|CT_OPAQUE);
      pdpNormal->Fill( 0, pixJOfs, pixWidth, pixHeight, C_BLACK|CT_OPAQUE);
      pdpNormal->Unlock();
    }

    // show
    pvpViewPort->SwapBuffers();

  }

}

CTextureObject _toStarField;
static FLOAT _fLastVolume = 1.0f;
void RenderStarfield(CDrawPort *pdp, FLOAT fStrength)
{
  CTextureData *ptd = (CTextureData *)_toStarField.GetData();
  // skip if no texture
  if(ptd==NULL) return;

  PIX pixSizeI = pdp->GetWidth();
  PIX pixSizeJ = pdp->GetHeight();
  FLOAT fStretch = pixSizeI/640.0f;
  fStretch*=FLOAT(ptd->GetPixWidth())/ptd->GetWidth();

  PIXaabbox2D boxScreen(PIX2D(0,0), PIX2D(pixSizeI, pixSizeJ));
  MEXaabbox2D boxTexture(MEX2D(0, 0), MEX2D(pixSizeI/fStretch, pixSizeJ/fStretch));
  pdp->PutTexture(&_toStarField, boxScreen, boxTexture, LerpColor(C_BLACK, C_WHITE, fStrength)|CT_OPAQUE);
}


FLOAT RenderQuitScreen(CDrawPort *pdp, CViewPort *pvp)
{
  CDrawPort dpQuit(pdp, TRUE);
  CDrawPort dpWide;
  dpQuit.MakeWideScreen(&dpWide);
  // redraw the view
  if (!dpWide.Lock()) {
    return 0;
  }

  dpWide.Fill(C_BLACK|CT_OPAQUE);
  RenderStarfield(&dpWide, _fLastVolume);
  
  FLOAT fVolume = Credits_Render(&dpWide);
  _fLastVolume = fVolume;

  dpWide.Unlock();
  pvp->SwapBuffers();

  return fVolume;
}
void QuitScreenLoop(void)
{
  g_cb.setSeriousState(GS_QUIT_SCREEN);
  Credits_On(3);
  CSoundObject soMusic;
  try {
    _toStarField.SetData_t(CTFILENAME("Textures\\Background\\Night01\\Stars01.tex"));
    soMusic.Play_t(CTFILENAME("Music\\Credits.mp3"), SOF_NONGAME|SOF_MUSIC|SOF_LOOP);
  } catch ( const char *strError) {
    CPrintF("%s\n", strError);
  }
  _bExitQuitScreen = false;
  // while it is still running
  while (!_bExitQuitScreen) {
    FLOAT fVolume = RenderQuitScreen(pdp, pvpViewPort);
    if (fVolume<=0) {
      return;
    }
    // assure we can listen to non-3d sounds
    soMusic.SetVolume(fVolume, fVolume);
    _pSound->UpdateSounds();

    g_cb.syncSeriousThreads();
    //Sleep(5);
  }
}

// try to start a new display mode
BOOL TryToSetDisplayMode( enum GfxAPIType eGfxAPI, INDEX iAdapter, PIX pixSizeI, PIX pixSizeJ,
                          enum DisplayDepth eColorDepth, BOOL bFullScreenMode)
{
  CDisplayMode dmTmp;
  dmTmp.dm_ddDepth = eColorDepth;
  CPrintF( TRANS("  Starting display mode: %dx%dx%s (%s)\n"),
           pixSizeI, pixSizeJ, dmTmp.DepthString(),
           bFullScreenMode ? TRANS("fullscreen") : TRANS("window"));

  // mark to start ignoring window size/position messages until settled down
  _bWindowChanging = TRUE;
  
  // destroy canvas if existing
  _pGame->DisableLoadingHook();
#if 0
  if( pvpViewPort!=NULL) {
    _pGfx->DestroyWindowCanvas( pvpViewPort);
    pvpViewPort = NULL;
    pdpNormal = NULL;
  }
#endif

  // close the application window
  CloseMainWindow();

  // try to set new display mode
  BOOL bSuccess;
  if( bFullScreenMode) {
#ifdef SE1_D3D
    if( eGfxAPI==GAT_D3D) OpenMainWindowFullScreen( pixSizeI, pixSizeJ);
#endif // SE1_D3D
    bSuccess = _pGfx->SetDisplayMode( eGfxAPI, iAdapter, pixSizeI, pixSizeJ, eColorDepth);
    if( bSuccess && eGfxAPI==GAT_OGL) OpenMainWindowFullScreen( pixSizeI, pixSizeJ);
  } else {
#ifdef SE1_D3D
    if( eGfxAPI==GAT_D3D) OpenMainWindowNormal( pixSizeI, pixSizeJ);
#endif // SE1_D3D
    bSuccess = _pGfx->ResetDisplayMode( eGfxAPI);
    if( bSuccess && eGfxAPI==GAT_OGL) OpenMainWindowNormal( pixSizeI, pixSizeJ);
#ifdef SE1_D3D
    if( bSuccess && eGfxAPI==GAT_D3D) ResetMainWindowNormal();
#endif // SE1_D3D
  }

  // if new mode was set
  if( bSuccess) {
    // create canvas
#if 0
    ASSERT( pvpViewPort==NULL);
    ASSERT( pdpNormal==NULL);
    _pGfx->CreateWindowCanvas( _hwndMain, &pvpViewPort, &pdpNormal);
#else
    pvpViewPort = g_cb.getViewPort();
    pdpNormal = &pvpViewPort->vp_Raster.ra_MainDrawPort;
#endif

    // erase context of both buffers (for the sake of wide-screen)
    pdp = pdpNormal;
    if( pdp!=NULL && pdp->Lock()) {
      pdp->Fill(C_BLACK|CT_OPAQUE);
      pdp->Unlock();
      pvpViewPort->SwapBuffers();
      pdp->Lock();
      pdp->Fill(C_BLACK|CT_OPAQUE);
      pdp->Unlock();
      pvpViewPort->SwapBuffers();
    }

    // lets try some wide screen screaming :)
    const PIX pixYBegAdj = pdp->GetHeight() * 21/24;
    const PIX pixYEndAdj = pdp->GetHeight() * 3/24;
    const PIX pixXEnd    = pdp->GetWidth();
    pdpWideScreen = new CDrawPort( pdp, PIXaabbox2D( PIX2D(0,pixYBegAdj), PIX2D(pixXEnd, pixYEndAdj)));
    pdpWideScreen->dp_fWideAdjustment = 9.0f / 12.0f;
    if( sam_bWideScreen) pdp = pdpWideScreen;

    // initial screen fill and swap, just to get context running
    BOOL bSuccess = FALSE;
    if( pdp!=NULL && pdp->Lock()) {
      pdp->Fill( LCDGetColor( C_dGREEN|CT_OPAQUE, "bcg fill"));
      pdp->Unlock();
      pvpViewPort->SwapBuffers();
      bSuccess = TRUE;
    }
    _pGame->EnableLoadingHook(pdp);

    // if the mode is not working, or is not accelerated
    if( !bSuccess)
    { // report error
      CPrintF( TRANS("This mode does not support hardware acceleration.\n"));
      // destroy canvas if existing
#if 0
      if( pvpViewPort!=NULL) {
        _pGame->DisableLoadingHook();
        _pGfx->DestroyWindowCanvas( pvpViewPort);
        pvpViewPort = NULL;
        pdpNormal = NULL;
      }
#endif
      // close the application window
      CloseMainWindow();
      // report failure
      return FALSE;
    }

    // remember new settings
    sam_bFullScreenActive = bFullScreenMode;
    sam_iScreenSizeI = pixSizeI;
    sam_iScreenSizeJ = pixSizeJ;
    sam_iDisplayDepth = eColorDepth;
    sam_iDisplayAdapter = iAdapter;
    sam_iGfxAPI = eGfxAPI;

    // report success
    return TRUE;
  // if couldn't set new mode
  } else {
    // close the application window
    CloseMainWindow();
    // report failure
    return FALSE;
  }
}


// list of possible display modes for recovery 
const INDEX aDefaultModes[][3] =
{ // color, API, adapter
  { DD_DEFAULT, GAT_OGL, 0},
  { DD_16BIT,   GAT_OGL, 0},
  { DD_16BIT,   GAT_OGL, 1}, // 3dfx Voodoo2
#ifdef SE1_D3D
  { DD_DEFAULT, GAT_D3D, 0},
  { DD_16BIT,   GAT_D3D, 0},
  { DD_16BIT,   GAT_D3D, 1},
#endif // SE1_D3D
};
const INDEX ctDefaultModes = ARRAYCOUNT(aDefaultModes);

// start new display mode
void StartNewMode( enum GfxAPIType eGfxAPI, INDEX iAdapter, PIX pixSizeI, PIX pixSizeJ,
                   enum DisplayDepth eColorDepth, BOOL bFullScreenMode)
{
  CPrintF( TRANS("\n* START NEW DISPLAY MODE ...\n"));

  // try to set the mode
  BOOL bSuccess = TryToSetDisplayMode( eGfxAPI, iAdapter, pixSizeI, pixSizeJ, eColorDepth, bFullScreenMode);

  // if failed
  if( !bSuccess)
  {

#if 0
    // report failure and reset to default resolution
    _iDisplayModeChangeFlag = 2;  // failure
    CPrintF( TRANS("Requested display mode could not be set!\n"));
    pixSizeI = 640;
    pixSizeJ = 480;
    bFullScreenMode = TRUE;
    // try to revert to one of recovery modes
    for( INDEX iMode=0; iMode<ctDefaultModes; iMode++) {
      eColorDepth = (DisplayDepth)aDefaultModes[iMode][0];
      eGfxAPI     = (GfxAPIType)  aDefaultModes[iMode][1];
      iAdapter    =               aDefaultModes[iMode][2];
      CPrintF(TRANS("\nTrying recovery mode %d...\n"), iMode);
      bSuccess = TryToSetDisplayMode( eGfxAPI, iAdapter, pixSizeI, pixSizeJ, eColorDepth, bFullScreenMode);
      if( bSuccess) break;
    }
#endif

    // if all failed
    if( !bSuccess) {
      FatalError(TRANS(
        "Cannot set display mode!\n"
        "Serious Sam was unable to find display mode with hardware acceleration.\n"
        "Make sure you install proper drivers for your video card as recommended\n"
        "in documentation and set your desktop to 16 bit (65536 colors).\n"
        "Please see ReadMe file for troubleshooting information.\n"));
    }

  // if succeeded
  } else {
    _iDisplayModeChangeFlag = 1;  // all ok
  }

  // apply 3D-acc settings
  ApplyGLSettings(FALSE);

  // remember time of mode setting
  _tmDisplayModeChanged = _pTimer->GetRealTimeTick();
}

// set controls from java (mutex locked)
void setControls(PlayerControls &ctrls) {
  static BOOL bStartLast = false;

  if (_pGame) {

    if (ctrls.bUse) {
      if (_pGame->gm_csComputerState == CS_ON || _pGame->gm_csComputerState == CS_TURNINGON) {
        _pGame->gm_csComputerState = CS_TURNINGOFF;
        ctrls.bUse = false;
      }
    }

    if (ctrls.bUse) {
      if (_pGame->gm_csConsoleState == CS_ON || _pGame->gm_csComputerState == CS_TURNINGON ||
          _pGame->gm_csComputerState == CS_TALK) {
        _pGame->gm_csConsoleState = CS_TURNINGOFF;
        ctrls.bUse = false;
      }
    }

    if (ctrls.bStart && ctrls.bStart != bStartLast) {
      if (_pGame->gm_csConsoleState == CS_OFF || _pGame->gm_csConsoleState == CS_TURNINGOFF) {
        _pGame->gm_csConsoleState = CS_TURNINGON;
      } else {
        _pGame->gm_csConsoleState = CS_TURNINGOFF;
      }
    }

    bStartLast = ctrls.bStart;

    PlayerControls &playerCtrl = *(PlayerControls *) _pGame->gm_lpLocalPlayers[0].lp_ubPlayerControlsState;
    playerCtrl = ctrls;

    for (int i = 0; i < 10; i++) {
      playerCtrl.axisValue[i] += ctrls.shiftAxisValue[i];
      ctrls.shiftAxisValue[i] = 0;
    }
  }
}

void seriousSamInitialize() {

  CTStream::EnableStreamHandling();

  if (FileExists(_modToLoadTxt)) {
    CTFileStream stream;
    stream.Open_t(_modToLoadTxt);
    CTString strMod;
    stream.GetLine_t(strMod);
    stream.Close();
    RemoveFile(_modToLoadTxt);
    if (strMod.Length() && strMod != "SeriousSam") {
      _fnmMod = "Mods\\" + strMod + "\\";
    }
  }

  Init();

  // override key settings
  CControls &ctrl = _pGame->gm_actrlControls[0];
  for (int i = 0; i < 8; i++) {
    ctrl.ctrl_aaAxisActions[i].aa_fSensitivity = 80;
    ctrl.ctrl_aaAxisActions[i].aa_fDeadZone = 0;
    ctrl.ctrl_aaAxisActions[i].aa_bInvert = false;
    ctrl.ctrl_aaAxisActions[i].aa_bRelativeControler = true;
    ctrl.ctrl_aaAxisActions[i].aa_bSmooth = false;
  }

  _pInput->EnableInput();

  _bRunning    = TRUE;
  _bQuitScreen = TRUE;
  _pGame->gm_csConsoleState  = CS_OFF;
  _pGame->gm_csComputerState = CS_OFF;

}

void seriousSubMain() {

  seriousSamInitialize();

  StartNextDemo();

  while(_bRunning && !_fnmModToLoad.Length()) {
    g_cb.syncSeriousThreads();

    // when all messages are removed, window has surely changed
    _bWindowChanging = FALSE;

    // automaticaly manage input enable/disable toggling
    UpdateInputEnabledState();
    // automaticaly manage pause toggling
    UpdatePauseState();
    // notify game whether menu is active
    _pGame->gm_bMenuOn = bMenuActive;

    BOOL bMenuForced = (_gmRunningGameMode==GM_NONE &&
                        (_pGame->gm_csConsoleState==CS_OFF || _pGame->gm_csConsoleState==CS_TURNINGOFF));

    if( !bMenuActive) {
      if( bMenuForced ) {
        // if console is active
        if( _pGame->gm_csConsoleState==CS_ON || _pGame->gm_csConsoleState==CS_TURNINGON) {
          // deactivate it
          _pGame->gm_csConsoleState = CS_TURNINGOFF;
          _iAddonExecState = 0;
        }
        // start menu
        StartMenus();
      }
    }

    if (sam_bMenu) {
      sam_bMenu = FALSE;
      StartMenus();
    }
    if (sam_bMenuSave) {
      sam_bMenuSave = FALSE;
      StartMenus("save");
    }
    if (sam_bMenuLoad) {
      sam_bMenuLoad = FALSE;
      StartMenus("load");
    }
    if (sam_bMenuControls) {
      sam_bMenuControls = FALSE;
      StartMenus("controls");
    }
    if (sam_bMenuHiScore) {
      sam_bMenuHiScore = FALSE;
      StartMenus("hiscore");
    }


    // do the main game loop and render screen
    DoGame();
  }
  _bRunning = false;

  _pInput->DisableInput();
  _pGame->StopGame();

  if (_fnmModToLoad.Length()) {
    CTFileStream stream;
    stream.Create_t(_modToLoadTxt);
    stream.PutString_t(_fnmModToLoad.FileName());
    stream.Close();
    _bQuitScreen = false;
    g_cb.restart();
    Sleep(60 * 1000); // wait restart
  }

  if (_bQuitScreen) QuitScreenLoop();

}

void drawBannerFpsVersion(CDrawPort *pdp, int64_t deltaFrame, float fps) {
  static int textWidthMax = 0;
  static float lastGlobalScale = g_cb.globalScale;
  if (lastGlobalScale != g_cb.globalScale) {
    lastGlobalScale = g_cb.globalScale;
    textWidthMax = 0;
  }
  SLONG slDPWidth = pdp->GetWidth();
  SLONG slDPHeight = pdp->GetHeight();
  pdp->SetFont(_pfdDisplayFont);
  pdp->SetTextScaling(g_cb.globalScale);
  pdp->SetTextAspect(1.0f);
  CTString str = CTString(0, SSA_VERSION " fps: %.2f; frame: %.2f ms", fps, deltaFrame / 1000000.f);
  ULONG textWidth = pdp->GetTextWidth(str);
  if (textWidth > textWidthMax) {
    textWidthMax = textWidth;
  }
  pdp->PutText(str, slDPWidth - textWidthMax, (PIX)(g_cb.globalScale * 30), LCDGetColor(C_GREEN | 255, "display mode"));
}
