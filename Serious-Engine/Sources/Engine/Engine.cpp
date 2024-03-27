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

#include <Engine/Build.h>
#include <Engine/Base/Profiling.h>
#include <Engine/Base/Input.h>
#include <Engine/Base/Protection.h>
#include <Engine/Base/Console.h>
#include <Engine/Base/Console_internal.h>
#include <Engine/Base/Statistics_Internal.h>
#include <Engine/Base/Shell.h>
#include <Engine/Base/CRC.h>
#include <Engine/Base/CRCTable.h>
#include <Engine/Base/ProgressHook.h>
#include <Engine/Sound/SoundListener.h>
#include <Engine/Sound/SoundLibrary.h>
#include <Engine/Graphics/GfxLibrary.h>
#include <Engine/Graphics/Font.h>
#include <Engine/Network/Network.h>
#include <Engine/Templates/DynamicContainer.cpp>
#include <Engine/Templates/Stock_CAnimData.h>
#include <Engine/Templates/Stock_CTextureData.h>
#include <Engine/Templates/Stock_CSoundData.h>
#include <Engine/Templates/Stock_CModelData.h>
#include <Engine/Templates/Stock_CEntityClass.h>
#include <Engine/Templates/Stock_CMesh.h>
#include <Engine/Templates/Stock_CSkeleton.h>
#include <Engine/Templates/Stock_CAnimSet.h>
#include <Engine/Templates/Stock_CShader.h>
#include <Engine/Templates/StaticArray.cpp>
#include <Engine/Base/IFeel.h>

#include <Engine/revision.h>
#include <sys/system_properties.h>
#include <sys/sysinfo.h>
#include <dirent.h>
#include <sys/stat.h>

// this version string can be referenced from outside the engine
ENGINE_API CTString _strEngineBuild  = "";
ENGINE_API ULONG _ulEngineBuildMajor = _SE_BUILD_MAJOR;
ENGINE_API ULONG _ulEngineBuildMinor = _SE_BUILD_MINOR;

ENGINE_API ULONG _ulEngineRevision = REVISION_ID;
ENGINE_API ULONG _ulEngineBuildYear = REVISION_BUILD_YEAR;
ENGINE_API ULONG _ulEngineBuildMonth = REVISION_BUILD_MONTH;
ENGINE_API ULONG _ulEngineBuildDay = REVISION_BUILD_DAY;

ENGINE_API BOOL _bDedicatedServer = FALSE;
ENGINE_API BOOL _bWorldEditorApp  = FALSE;
ENGINE_API CTString _strLogFile = "";

// global handle for application window
HWND _hwndMain = NULL;
BOOL _bFullScreen = FALSE;

// critical section for access to zlib functions
CTCriticalSection zip_csLock; 

// to keep system gamma table
static UWORD auwSystemGamma[256*3];


// OS info
static CTString sys_strOS = "";
static INDEX sys_iOSMajor = 0;
static INDEX sys_iOSMinor = 0;
static INDEX sys_iOSBuild = 0;
static CTString sys_strOSMisc = "";

// CPU info
//static CTString sys_strCPUVendor = "";
//static INDEX sys_iCPUType = 0;
//static INDEX sys_iCPUFamily = 0;
//static INDEX sys_iCPUModel = 0;
//static INDEX sys_iCPUStepping = 0;
//static BOOL  sys_bCPUHasMMX = 0;
//static BOOL  sys_bCPUHasCMOV = 0;
//static INDEX sys_iCPUMHz = 0;
//       INDEX sys_iCPUMisc = 0;

// RAM info
static INDEX sys_iRAMPhys = 0;
static INDEX sys_iRAMSwap = 0;

// HDD info
//static INDEX sys_iHDDSize = 0;
//static INDEX sys_iHDDFree = 0;
//static INDEX sys_iHDDMisc = 0;

// MOD info
static CTString sys_strModName = "";
static CTString sys_strModExt  = "";

// enables paranoia checks for allocation array
BOOL _bAllocationArrayParanoiaCheck = FALSE;

//static void DetectCPU(void)
//{
//  char strVendor[12+1];
//  strVendor[12] = 0;
//  ULONG ulTFMS;
//  ULONG ulFeatures;
//
//  // test MMX presence and update flag
//  __asm {
//    mov     eax,0           ;// request for basic id
//    cpuid
//    mov     dword ptr [strVendor+0], ebx
//    mov     dword ptr [strVendor+4], edx
//    mov     dword ptr [strVendor+8], ecx
//    mov     eax,1           ;// request for TFMS feature flags
//    cpuid
//    mov     dword ptr [ulTFMS], eax ;// remember type, family, model and stepping
//    mov     dword ptr [ulFeatures], edx
//  }
//
//  INDEX iType     = (ulTFMS>>12)&0x3;
//  INDEX iFamily   = (ulTFMS>> 8)&0xF;
//  INDEX iModel    = (ulTFMS>> 4)&0xF;
//  INDEX iStepping = (ulTFMS>> 0)&0xF;
//
//
//  CPrintF(TRANS("  Vendor: %s\n"), strVendor);
//  CPrintF(TRANS("  Type: %d, Family: %d, Model: %d, Stepping: %d\n"),
//    iType, iFamily, iModel, iStepping);
//
//  BOOL bMMX  = ulFeatures & (1<<23);
//  BOOL bCMOV = ulFeatures & (1<<15);
//
//  CTString strYes = TRANS("Yes");
//  CTString strNo = TRANS("No");
//
//  CPrintF(TRANS("  MMX : %s\n"), bMMX ?strYes:strNo);
//  CPrintF(TRANS("  CMOV: %s\n"), bCMOV?strYes:strNo);
//  CPrintF(TRANS("  Clock: %.0fMHz\n"), _pTimer->tm_llCPUSpeedHZ/1E6);
//
//  sys_strCPUVendor = strVendor;
//  sys_iCPUType = iType;
//  sys_iCPUFamily =  iFamily;
//  sys_iCPUModel = iModel;
//  sys_iCPUStepping = iStepping;
//  sys_bCPUHasMMX = bMMX!=0;
//  sys_bCPUHasCMOV = bCMOV!=0;
//  sys_iCPUMHz = INDEX(_pTimer->tm_llCPUSpeedHZ/1E6);
//
//  if( !bMMX) FatalError( TRANS("MMX support required but not present!"));
//}

//static void DetectCPUWrapper(void)
//{
//  __try {
//    DetectCPU();
//  } __except(EXCEPTION_EXECUTE_HANDLER) {
//    CPrintF( TRANS("Cannot detect CPU: exception raised.\n"));
//  }
//}

// reverses string
void StrRev( char *str) {
  char ctmp;
  char *pch0 = str;
  char *pch1 = str+strlen(str)-1;
  while( pch1>pch0) {
    ctmp  = *pch0;
    *pch0 = *pch1;
    *pch1 = ctmp;
    pch0++;
    pch1--;
  }
}

//static char strExePath[MAX_PATH] = "";
//static char strDirPath[MAX_PATH] = "";

static void AnalyzeApplicationPath(void)
{
  FatalError("unused");
//  strcpy(strDirPath, "D:\\");
//  strcpy(strExePath, "D:\\TestExe.xbe");
//  char strTmpPath[MAX_PATH] = "";
//  // get full path to the exe
//  GetModuleFileNameA( NULL, strExePath, sizeof(strExePath)-1);
//  // copy that to the path
//  strncpy(strTmpPath, strExePath, sizeof(strTmpPath)-1);
//  strDirPath[sizeof(strTmpPath)-1] = 0;
//  // remove name from application path
//  StrRev(strTmpPath);
//  // find last backslash
//  char *pstr = strchr( strTmpPath, '\\');
//  if( pstr==NULL) {
//    // not found - path is just "\"
//    strcpy( strTmpPath, "\\");
//    pstr = strTmpPath;
//  }
//  // remove 'debug' from app path if needed
//  if( strnicmp( pstr, "\\gubed", 6)==0) pstr += 6;
//  if( pstr[0] = '\\') pstr++;
//  char *pstrFin = strchr( pstr, '\\');
//  if( pstrFin==NULL) {
//    strcpy( pstr, "\\");
//    pstrFin = pstr;
//  }
//  // copy that to the path
//  StrRev(pstrFin);
//  strncpy( strDirPath, pstrFin, sizeof(strDirPath)-1);
//  strDirPath[sizeof(strDirPath)-1] = 0;
}


// startup engine 
ENGINE_API void SE_InitEngine(CTString strGameID)
{
  #pragma message(">> Remove this from SE_InitEngine : _bWorldEditorApp")
  if(strGameID=="SeriousEditor") {
    _bWorldEditorApp = TRUE;
  }

  // setted by bindings.cpp
//  AnalyzeApplicationPath();
//  _fnmApplicationPath = CTString(strDirPath);
//  _fnmApplicationExe = CTString(strExePath);
//  try {
//    _fnmApplicationExe.RemoveApplicationPath_t();
//  } catch ( const char *strError) {
//    (void) strError;
//    ASSERT(FALSE);
//  }

  _pConsole = new CConsole;
  if (_strLogFile=="") {
//    _strLogFile = CTFileName(CTString(strExePath)).FileName();
    _strLogFile = CTFileName(_fnmApplicationExe).FileName();
  }

  CTFileName fnmLogsDir = _fnmApplicationPath + "Logs/";
  mkdir(fnmLogsDir.str_String, ACCESSPERMS);
  _pConsole->Initialize(fnmLogsDir + _strLogFile+".log", 90, 512);

  _pAnimStock        = new CStock_CAnimData;
  _pTextureStock     = new CStock_CTextureData;
  _pSoundStock       = new CStock_CSoundData;
  _pModelStock       = new CStock_CModelData;
  _pEntityClassStock = new CStock_CEntityClass;
  _pMeshStock        = new CStock_CMesh;
  _pSkeletonStock    = new CStock_CSkeleton;
  _pAnimSetStock     = new CStock_CAnimSet;
  _pShaderStock      = new CStock_CShader;

  _pTimer = new CTimer;
  _pGfx   = new CGfxLibrary;
  _pSound = new CSoundLibrary;

  // [SSE] Light Dedicated Server
  if (!_bDedicatedServer) {
    _pInput = new CInput;
  }

  _pNetwork = new CNetworkLibrary;

  CRCT_Init();

  _strEngineBuild.PrintF( TRANS("SeriousEngine Build: %d.%d"), _SE_BUILD_MAJOR, _SE_BUILD_MINOR);

  // print basic engine info
  CPrintF(TRANS("--- Serious Engine E Startup ---\n"));
  CPrintF("  %s\n", _strEngineBuild);
  CPrintF("  Revision:   %d\n", _ulEngineRevision);
  CPrintF("  Build Date: %d/%d/%d\n\n", _ulEngineBuildYear, _ulEngineBuildMonth, _ulEngineBuildDay);
  
  // print info on the started application
  CPrintF(TRANS("Executable: %s\n"), _fnmApplicationExe);
  CPrintF(TRANS("Assumed engine directory: %s\n"), _fnmApplicationPath);

  CPrintF("\n");

  // report os info
  CPrintF(TRANS("Examining underlying OS...\n"));
//  OSVERSIONINFOA osv;
//  memset(&osv, 0, sizeof(osv));
//  osv.dwOSVersionInfoSize = sizeof(osv);
//  if (GetVersionExA(&osv)) {
//    switch (osv.dwPlatformId) {
//    case VER_PLATFORM_WIN32s:         sys_strOS = "Win32s";  break;
//    case VER_PLATFORM_WIN32_WINDOWS:  sys_strOS = "Win9x"; break;
//    case VER_PLATFORM_WIN32_NT:       sys_strOS = "WinNT"; break;
//    default: sys_strOS = "Unknown\n"; break;
//    }
//
//    sys_iOSMajor = osv.dwMajorVersion;
//    sys_iOSMinor = osv.dwMinorVersion;
//    sys_iOSBuild = osv.dwBuildNumber & 0xFFFF;
//    sys_strOSMisc = osv.szCSDVersion;
//
//    CPrintF(TRANS("  Type: %s\n"), (const char*)sys_strOS);
//    CPrintF(TRANS("  Version: %d.%d, build %d\n"),
//      osv.dwMajorVersion, osv.dwMinorVersion, osv.dwBuildNumber & 0xFFFF);
//    CPrintF(TRANS("  Misc: %s\n"), osv.szCSDVersion);
//  } else {
//    CPrintF(TRANS("Error getting OS info: %s\n"), GetWindowsError(GetLastError()).str_String );
//  }
  char buffer[PROP_VALUE_MAX+1];
  if (__system_property_get("ro.product.model", buffer)) {
    CPrintF("  Running on Android [%s]\n", buffer);
  } else {
    CPrintF("  Running on Android\n");
  }
  if (__system_property_get("ro.build.version.sdk", buffer)) {
    CPrintF("  API level %s\n", buffer);
  }
  if (__system_property_get("ro.product.cpu.abi", buffer)) {
    CPrintF("  Abi: %s", buffer);
  }
  if (__system_property_get("ro.product.cpu.abilist", buffer)) {
    CPrintF(" [avaiable: %s]", buffer);
  }
  CPrintF("\n");
  CPrintF("\n");

  // init main shell
  _pShell = new CShell;
  _pShell->Initialize();

  // report CPU
//  CPrintF(TRANS("Detecting CPU...\n"));
//  DetectCPUWrapper();
//  CPrintF("\n");

  // report memory info
  extern void ReportGlobalMemoryStatus(void);
  ReportGlobalMemoryStatus();

//  MEMORYSTATUS ms;
//  GlobalMemoryStatus(&ms);

  #define MB (1024*1024)
//  sys_iRAMPhys = ms.dwTotalPhys    /MB;
//  sys_iRAMSwap = ms.dwTotalPageFile/MB;
  struct sysinfo info;
  sysinfo(&info);
  sys_iRAMPhys = info.totalram;
  sys_iRAMSwap = info.totalswap;

  // initialize zip semaphore
  zip_csLock.cs_iIndex = -1;  // not checked for locking order


  // get info on the first disk in system
//  DWORD dwSerial;
//  DWORD dwFreeClusters;
//  DWORD dwClusters;
//  DWORD dwSectors;
//  DWORD dwBytes;

//  char strDrive[] = "C:\\";
//  strDrive[0] = strExePath[0];
//
//  GetVolumeInformationA(strDrive, NULL, 0, &dwSerial, NULL, NULL, NULL, 0);
//  GetDiskFreeSpaceA(strDrive, &dwSectors, &dwBytes, &dwFreeClusters, &dwClusters);
//  sys_iHDDSize = __int64(dwSectors)*dwBytes*dwClusters/MB;
//  sys_iHDDFree = __int64(dwSectors)*dwBytes*dwFreeClusters/MB;
//  sys_iHDDMisc = dwSerial;

  // add console variables
  extern INDEX con_bNoWarnings;
  extern INDEX wld_bFastObjectOptimization;
  extern INDEX fil_bPreferZips;
  extern FLOAT mth_fCSGEpsilon;
  _pShell->DeclareSymbol("user INDEX con_bNoWarnings;",(void*) &con_bNoWarnings);
  _pShell->DeclareSymbol("user INDEX wld_bFastObjectOptimization;",(void*) &wld_bFastObjectOptimization);
  _pShell->DeclareSymbol("user FLOAT mth_fCSGEpsilon;",(void*) &mth_fCSGEpsilon);
  _pShell->DeclareSymbol("persistent user INDEX fil_bPreferZips;",(void*) &fil_bPreferZips);
  // OS info
  _pShell->DeclareSymbol("user const CTString sys_strOS    ;",(void*) &sys_strOS);
  _pShell->DeclareSymbol("user const INDEX sys_iOSMajor    ;",(void*) &sys_iOSMajor);
  _pShell->DeclareSymbol("user const INDEX sys_iOSMinor    ;",(void*) &sys_iOSMinor);
  _pShell->DeclareSymbol("user const INDEX sys_iOSBuild    ;",(void*) &sys_iOSBuild);
  _pShell->DeclareSymbol("user const CTString sys_strOSMisc;",(void*) &sys_strOSMisc);
  // CPU info
//  _pShell->DeclareSymbol("user const CTString sys_strCPUVendor;", &sys_strCPUVendor);
//  _pShell->DeclareSymbol("user const INDEX sys_iCPUType       ;", &sys_iCPUType    );
//  _pShell->DeclareSymbol("user const INDEX sys_iCPUFamily     ;", &sys_iCPUFamily  );
//  _pShell->DeclareSymbol("user const INDEX sys_iCPUModel      ;", &sys_iCPUModel   );
//  _pShell->DeclareSymbol("user const INDEX sys_iCPUStepping   ;", &sys_iCPUStepping);
//  _pShell->DeclareSymbol("user const INDEX sys_bCPUHasMMX     ;", &sys_bCPUHasMMX  );
//  _pShell->DeclareSymbol("user const INDEX sys_bCPUHasCMOV    ;", &sys_bCPUHasCMOV );
//  _pShell->DeclareSymbol("user const INDEX sys_iCPUMHz        ;", &sys_iCPUMHz     );
//  _pShell->DeclareSymbol("     const INDEX sys_iCPUMisc       ;", &sys_iCPUMisc    );
  // RAM info
  _pShell->DeclareSymbol("user const INDEX sys_iRAMPhys;",(void*) &sys_iRAMPhys);
  _pShell->DeclareSymbol("user const INDEX sys_iRAMSwap;",(void*)&sys_iRAMSwap);
//  _pShell->DeclareSymbol("user const INDEX sys_iHDDSize;", &sys_iHDDSize);
//  _pShell->DeclareSymbol("user const INDEX sys_iHDDFree;", &sys_iHDDFree);
//  _pShell->DeclareSymbol("     const INDEX sys_iHDDMisc;", &sys_iHDDMisc);
  // MOD info
  _pShell->DeclareSymbol("user const CTString sys_strModName;", (void*)&sys_strModName);
  _pShell->DeclareSymbol("user const CTString sys_strModExt;", (void*) &sys_strModExt);

  // Stock clearing
  extern void FreeUnusedStock(void);
  _pShell->DeclareSymbol("user void FreeUnusedStock(void);", (void*) &FreeUnusedStock);
  
  // Timer tick quantum
  _pShell->DeclareSymbol("user const FLOAT fTickQuantum;", (FLOAT*)&_pTimer->TickQuantum);

  // init MODs and stuff ...
  extern void InitStreams(void);
  InitStreams();
  // keep mod name in sys cvar
  sys_strModName = _strModName;
  sys_strModExt  = _strModExt;

// checking of crc
#if 0
  ULONG ulCRCActual = -2;
  SLONG ulCRCExpected = -1;
  try {
    // get the checksum of engine
    #ifndef NDEBUG
      #define SELFFILE "Bin\\Debug\\EngineD.dll"
      #define SELFCRCFILE "Bin\\Debug\\EngineD.crc"
    #else
      #define SELFFILE "Bin\\Engine.dll"
      #define SELFCRCFILE "Bin\\Engine.crc"
    #endif
    ulCRCActual = GetFileCRC32_t(CTString(SELFFILE));
    // load expected checksum from the file on disk
    ulCRCExpected = 0;
    LoadIntVar(CTString(SELFCRCFILE), ulCRCExpected);
  } catch ( const char *strError) {
    CPrintF("%s\n", strError);
  }
  // if not same
  if (ulCRCActual!=ulCRCExpected) {
    // don't run
    //FatalError(TRANS("Engine CRC is invalid.\nExpected %08x, but found %08x.\n"), ulCRCExpected, ulCRCActual);
  }
#endif

  // [SSE] Light Dedicated Server
  if (!_bDedicatedServer) {
    _pInput->Initialize();
  }

  _pGfx->Init();
  _pSound->Init();

  if (strGameID!="") {
    _pNetwork->Init(strGameID);
    // just make classes declare their shell variables
    try {
      CEntityClass* pec = _pEntityClassStock->Obtain_t(CTString("Classes\\Player.ecl"));
      ASSERT(pec != NULL);
      _pEntityClassStock->Release(pec);  // this must not be a dependency!
    // if couldn't find player class
    } catch ( const char *strError) {
      FatalError(TRANS("Cannot initialize classes:\n%s"), strError);
    }
  } else {
    _pNetwork = NULL;
  }

  // mark that default fonts aren't loaded (yet)
  _pfdDisplayFont = NULL;
  _pfdConsoleFont = NULL;

  // readout system gamma table
//  HDC  hdc = GetDC(NULL);
//  BOOL bOK = GetDeviceGammaRamp( hdc, &auwSystemGamma[0]);
//  _pGfx->gl_ulFlags |= GLF_ADJUSTABLEGAMMA;
//  if( !bOK) {
//    _pGfx->gl_ulFlags &= ~GLF_ADJUSTABLEGAMMA;
//    CPrintF( TRANS("\nWARNING: Gamma, brightness and contrast are not adjustable!\n\n"));
//  } // done
//  ReleaseDC( NULL, hdc);
  _pGfx->gl_ulFlags &= ~GLF_ADJUSTABLEGAMMA;

  // init IFeel
  HWND hwnd = NULL;//GetDesktopWindow();
  HINSTANCE hInstance = GetModuleHandle(NULL);
  if(IFeel_InitDevice(hInstance,hwnd))
  {
    CTString strDefaultProject = "Data\\Default.ifr";
    // get project file name for this device
    CTString strIFeel = IFeel_GetProjectFileName();
    // if no file name is returned use default file
    if(strIFeel.Length()==0) strIFeel = strDefaultProject;
    if(!IFeel_LoadFile(strIFeel))
    {
      if(strIFeel!=strDefaultProject)
      {
        IFeel_LoadFile(strDefaultProject);
      }
    }
    CPrintF("\n");
  }
  
}


// shutdown entire engine
ENGINE_API void SE_EndEngine(void)
{
  // restore system gamma table (if needed)
  if( _pGfx->gl_ulFlags&GLF_ADJUSTABLEGAMMA) {
//    HDC  hdc = GetDC(NULL);
//    BOOL bOK = SetDeviceGammaRamp( hdc, &auwSystemGamma[0]);
//    //ASSERT(bOK);
//    ReleaseDC( NULL, hdc);
  }

  // free stocks
  delete _pEntityClassStock;  _pEntityClassStock = NULL;
  delete _pModelStock;        _pModelStock       = NULL; 
  delete _pSoundStock;        _pSoundStock       = NULL; 
  delete _pTextureStock;      _pTextureStock     = NULL; 
  delete _pAnimStock;         _pAnimStock        = NULL; 
  delete _pMeshStock;         _pMeshStock        = NULL; 
  delete _pSkeletonStock;     _pSkeletonStock    = NULL; 
  delete _pAnimSetStock;      _pAnimSetStock     = NULL; 
  delete _pShaderStock;       _pShaderStock      = NULL; 

  // free all memory used by the crc cache
  CRCT_Clear();

  // shutdown
  if ( _pNetwork != NULL) { delete _pNetwork;  _pNetwork = NULL; }
  
  // _pInput can be NULL while running DedicatedServer.
  if (   _pInput != NULL) { delete _pInput;    _pInput   = NULL; }
  delete _pSound;    _pSound   = NULL;  
  delete _pGfx;      _pGfx     = NULL;    
  delete _pTimer;    _pTimer   = NULL;  
  delete _pShell;    _pShell   = NULL;  
  delete _pConsole;  _pConsole = NULL;
  extern void EndStreams(void);
  EndStreams();

  // shutdown profilers
  _sfStats.Clear();
  _pfGfxProfile           .pf_apcCounters.Clear();
  _pfGfxProfile           .pf_aptTimers  .Clear();
  _pfModelProfile         .pf_apcCounters.Clear();
  _pfModelProfile         .pf_aptTimers  .Clear();
  _pfSoundProfile         .pf_apcCounters.Clear();
  _pfSoundProfile         .pf_aptTimers  .Clear();
  _pfNetworkProfile       .pf_apcCounters.Clear();
  _pfNetworkProfile       .pf_aptTimers  .Clear();
  _pfRenderProfile        .pf_apcCounters.Clear();
  _pfRenderProfile        .pf_aptTimers  .Clear();
  _pfWorldEditingProfile  .pf_apcCounters.Clear();
  _pfWorldEditingProfile  .pf_aptTimers  .Clear();
  _pfPhysicsProfile       .pf_apcCounters.Clear();
  _pfPhysicsProfile       .pf_aptTimers  .Clear();

  // remove default fonts if needed
  if( _pfdDisplayFont != NULL) { delete _pfdDisplayFont;  _pfdDisplayFont=NULL; }
  if( _pfdConsoleFont != NULL) { delete _pfdConsoleFont;  _pfdConsoleFont=NULL; } 

  // deinit IFeel
  IFeel_DeleteDevice();
}


// prepare and load some default fonts
ENGINE_API void SE_LoadDefaultFonts(void)
{
  _pfdDisplayFont = new CFontData;
  _pfdConsoleFont = new CFontData;

  // try to load default fonts
  try {
    _pfdDisplayFont->Load_t( CTFILENAME( "Fonts\\Display3-narrow.fnt"));
    _pfdConsoleFont->Load_t( CTFILENAME( "Fonts\\Console1.fnt"));
  }
  catch ( const char *strError) {
    FatalError( TRANS("Error loading font: %s."), strError);
  }
  // change fonts' default spacing a bit
  _pfdDisplayFont->SetCharSpacing( 0);
  _pfdDisplayFont->SetLineSpacing(+1);
  _pfdConsoleFont->SetCharSpacing(-1);
  _pfdConsoleFont->SetLineSpacing(+1);
  _pfdConsoleFont->SetFixedWidth();
}


// updates main windows' handles for windowed mode and fullscreen
ENGINE_API void SE_UpdateWindowHandle( HWND hwndMain)
{
  ASSERT( hwndMain!=NULL);
  _hwndMain = hwndMain;
  _bFullScreen = _pGfx!=NULL && (_pGfx->gl_ulFlags&GLF_FULLSCREEN);
}


static BOOL TouchBlock(UBYTE *pubMemoryBlock, INDEX ctBlockSize)
{
//  // cannot pretouch block that are smaller than 64KB :(
//  ctBlockSize -= 16*0x1000;
//  if( ctBlockSize<4) return FALSE;
//
//  __try {
//    // 4 times should be just enough
//    for( INDEX i=0; i<4; i++) {
//      // must do it in asm - don't know what VC will try to optimize
//      __asm {
//        // The 16-page skip is to keep Win 95 from thinking we're trying to page ourselves in
//        // (we are doing that, of course, but there's no reason we shouldn't) - THANX JOHN! :)
//        mov   esi,dword ptr [pubMemoryBlock]
//        mov   ecx,dword ptr [ctBlockSize]
//        shr   ecx,2
//touchLoop:
//        mov   eax,dword ptr [esi]
//        mov   ebx,dword ptr [esi+16*0x1000]
//        add   eax,ebx     // BLA, BLA, TROOCH, TRUCH
//        add   esi,4
//        dec   ecx
//        jnz   touchLoop
//      }
//    }
//  }
//  __except(EXCEPTION_EXECUTE_HANDLER) {
//    return FALSE;
//  }
  return TRUE;
}


// pretouch all memory commited by process
BOOL _bNeedPretouch = FALSE;
ENGINE_API extern void SE_PretouchIfNeeded(void)
{
  _bNeedPretouch = FALSE;
}




#if 0

      // printout block info
      CPrintF("--------\n");
      CTString strTmp1, strTmp2;
      CPrintF("Base/Alloc Address: 0x%8X / 0x%8X - Size: %d KB\n", mbi.BaseAddress, mbi.AllocationBase, mbi.RegionSize/1024);
      switch( mbi.Protect) {
      case PAGE_READONLY:          strTmp1 = "PAGE_READONLY";          break;
      case PAGE_READWRITE:         strTmp1 = "PAGE_READWRITE";         break;
      case PAGE_WRITECOPY:         strTmp1 = "PAGE_WRITECOPY";         break;
      case PAGE_EXECUTE:           strTmp1 = "PAGE_EXECUTE";           break;
      case PAGE_EXECUTE_READ:      strTmp1 = "PAGE_EXECUTE_READ";      break;
      case PAGE_EXECUTE_READWRITE: strTmp1 = "PAGE_EXECUTE_READWRITE"; break;
      case PAGE_GUARD:             strTmp1 = "PAGE_GUARD";             break;
      case PAGE_NOACCESS:          strTmp1 = "PAGE_NOACCESS";          break;
      case PAGE_NOCACHE:           strTmp1 = "PAGE_NOCACHE";           break;
      }
      switch( mbi.AllocationProtect) {
      case PAGE_READONLY:          strTmp2 = "PAGE_READONLY";          break;
      case PAGE_READWRITE:         strTmp2 = "PAGE_READWRITE";         break;
      case PAGE_WRITECOPY:         strTmp2 = "PAGE_WRITECOPY";         break;
      case PAGE_EXECUTE:           strTmp2 = "PAGE_EXECUTE";           break;
      case PAGE_EXECUTE_READ:      strTmp2 = "PAGE_EXECUTE_READ";      break;
      case PAGE_EXECUTE_READWRITE: strTmp2 = "PAGE_EXECUTE_READWRITE"; break;
      case PAGE_GUARD:             strTmp2 = "PAGE_GUARD";             break;
      case PAGE_NOACCESS:          strTmp2 = "PAGE_NOACCESS";          break;
      case PAGE_NOCACHE:           strTmp2 = "PAGE_NOCACHE";           break;
      }
      CPrintF("Current/Alloc protect: %s / %s\n", strTmp1, strTmp2);
      switch( mbi.State) {
      case MEM_COMMIT:  strTmp1 = "MEM_COMMIT";  break;
      case MEM_FREE:    strTmp1 = "MEM_FREE";    break;
      case MEM_RESERVE: strTmp1 = "MEM_RESERVE"; break;
      }
      switch( mbi.Type) {
      case MEM_IMAGE:   strTmp2 = "MEM_IMAGE";   break;
      case MEM_MAPPED:  strTmp2 = "MEM_MAPPED";  break;
      case MEM_PRIVATE: strTmp2 = "MEM_PRIVATE"; break;
      }
      CPrintF("State/Type: %s / %s\n", strTmp1, strTmp2);

#endif
