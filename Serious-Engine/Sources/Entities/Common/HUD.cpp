#include "Entities/StdH/StdH.h"

#include <Engine/Graphics/DrawPort.h>
#include <Entities/Player.h>
#include <Entities/PlayerWeapons.h>
#include <Entities/MusicHolder.h>
#include <Entities/EnemyBase.h>
#include <Entities/EnemyCounter.h>
#include <AndroidAdapters/binding-callbacks.h>


// armor & health constants 
// NOTE: these _do not_ reflect easy/tourist maxvalue adjustments. that is by design!
#define TOP_ARMOR  100
#define TOP_HEALTH 100


// cheats
extern INDEX cht_bEnable;
extern INDEX cht_bGod;
extern INDEX cht_bFly;
extern INDEX cht_bGhost;
extern INDEX cht_bInvisible;
extern FLOAT cht_fTranslationMultiplier;

// interface control
extern INDEX hud_bShowInfo;
extern INDEX hud_bShowLatency;
extern INDEX hud_bShowMessages;
extern INDEX hud_iShowPlayers;
extern INDEX hud_iSortPlayers;
extern FLOAT hud_fOpacity;
extern FLOAT hud_fScaling;
extern FLOAT hud_tmWeaponsOnScreen;


// player statistics sorting keys
enum SortKeys {
  PSK_NAME    = 1,
  PSK_HEALTH  = 2,
  PSK_SCORE   = 3,
  PSK_MANA    = 4, 
  PSK_FRAGS   = 5,
  PSK_DEATHS  = 6,
};

// where is the bar lowest value
enum BarOrientations {
  BO_LEFT  = 1,
  BO_RIGHT = 2, 
  BO_UP    = 3,
  BO_DOWN  = 4,
};



// maximal mana for master status
#define MANA_MASTER 10000

// drawing variables
static const CPlayer *_penPlayer;
static CPlayerWeapons *_penWeapons;
static CDrawPort *_pDP;
static PIX   _pixDPWidth, _pixDPHeight;
static FLOAT _fResolutionScaling;
static FLOAT _fCustomScaling;
static ULONG _ulAlphaHUD;
static COLOR _colHUD;
static TIME  _tmNow = -1.0f;
static CFontData _fdNumbersFont;

// array for pointers of all players
CPlayer *_apenPlayers[NET_MAXGAMEPLAYERS] = {0};

// status bar textures
static CTextureObject _toHealth;
static CTextureObject _toArmor;
static CTextureObject _toOxygen;
static CTextureObject _toScore;
static CTextureObject _toHiScore;
static CTextureObject _toMessage;
static CTextureObject _toMana;
static CTextureObject _toFrags;
static CTextureObject _toDeaths;
// ammo textures                    
static CTextureObject _toAShells;
static CTextureObject _toABullets;
static CTextureObject _toARockets;
static CTextureObject _toAGrenades;
static CTextureObject _toANapalm;
static CTextureObject _toAElectricity;
static CTextureObject _toAIronBall;
// weapon textures
static CTextureObject _toWKnife;
static CTextureObject _toWColt;
static CTextureObject _toWSingleShotgun;
static CTextureObject _toWDoubleShotgun;
static CTextureObject _toWTommygun;
static CTextureObject _toWMinigun;
static CTextureObject _toWRocketLauncher;
static CTextureObject _toWGrenadeLauncher;
static CTextureObject _toWPipeBomb;
static CTextureObject _toWFlamer;
static CTextureObject _toWGhostBuster;
static CTextureObject _toWLaser;
static CTextureObject _toWIronCannon;
// tile texture (one has corners, edges and center)
static CTextureObject _toTile;


// all info about color transitions
struct ColorTransitionTable {
  COLOR ctt_colFine;      // color for values over 1.0
  COLOR ctt_colHigh;      // color for values from 1.0 to 'fMedium'
  COLOR ctt_colMedium;    // color for values from 'fMedium' to 'fLow'
  COLOR ctt_colLow;       // color for values under fLow
  FLOAT ctt_fMediumHigh;  // when to switch to high color   (normalized float!)
  FLOAT ctt_fLowMedium;   // when to switch to medium color (normalized float!)
  BOOL  ctt_bSmooth;      // should colors have smooth transition
};
static struct ColorTransitionTable _cttHUD;


// ammo's info structure
struct AmmoInfo {
  CTextureObject    *ai_ptoAmmo;
  struct WeaponInfo *ai_pwiWeapon1;
  struct WeaponInfo *ai_pwiWeapon2;
  INDEX ai_iAmmoAmmount;
  INDEX ai_iMaxAmmoAmmount;
  INDEX ai_iLastAmmoAmmount;
  TIME  ai_tmAmmoChanged;
  BOOL  ai_bHasWeapon;
};

// weapons' info structure
struct WeaponInfo {
  enum WeaponType  wi_wtWeapon;
  CTextureObject  *wi_ptoWeapon;
  struct AmmoInfo *wi_paiAmmo;
  BOOL wi_bHasWeapon;
};

extern struct WeaponInfo _awiWeapons[18];
static struct AmmoInfo _aaiAmmo[8] = {
  { &_toAShells,      &_awiWeapons[4],  &_awiWeapons[5],  0, 0, 0, -9, FALSE },
  { &_toABullets,     &_awiWeapons[6],  &_awiWeapons[7],  0, 0, 0, -9, FALSE },
  { &_toARockets,     &_awiWeapons[8],  NULL,             0, 0, 0, -9, FALSE },
  { &_toAGrenades,    &_awiWeapons[9],  &_awiWeapons[10], 0, 0, 0, -9, FALSE },
  { &_toANapalm,      &_awiWeapons[12], NULL,             0, 0, 0, -9, FALSE },
  { &_toAElectricity, &_awiWeapons[14], &_awiWeapons[15], 0, 0, 0, -9, FALSE },
  { &_toAIronBall,    &_awiWeapons[16], NULL,             0, 0, 0, -9, FALSE },
  { &_toAIronBall,    &_awiWeapons[17], NULL,             0, 0, 0, -9, FALSE },
};

struct WeaponInfo _awiWeapons[18] = {
  { WEAPON_NONE,            NULL,                 NULL,         FALSE },   //  0
  { WEAPON_KNIFE,           &_toWKnife,           NULL,         FALSE },   //  1
  { WEAPON_COLT,            &_toWColt,            NULL,         FALSE },   //  2
  { WEAPON_DOUBLECOLT,      &_toWColt,            NULL,         FALSE },   //  3
  { WEAPON_SINGLESHOTGUN,   &_toWSingleShotgun,   &_aaiAmmo[0], FALSE },   //  4
  { WEAPON_DOUBLESHOTGUN,   &_toWDoubleShotgun,   &_aaiAmmo[0], FALSE },   //  5
  { WEAPON_TOMMYGUN,        &_toWTommygun,        &_aaiAmmo[1], FALSE },   //  6
  { WEAPON_MINIGUN,         &_toWMinigun,         &_aaiAmmo[1], FALSE },   //  7
  { WEAPON_ROCKETLAUNCHER,  &_toWRocketLauncher,  &_aaiAmmo[2], FALSE },   //  8
  { WEAPON_GRENADELAUNCHER, &_toWGrenadeLauncher, &_aaiAmmo[3], FALSE },   //  9
  { WEAPON_NONE,            NULL,                 NULL,         FALSE }, //{ WEAPON_PIPEBOMB,        &_toWPipeBomb,        &_aaiAmmo[3], FALSE },   // 10
  { WEAPON_NONE,            NULL,                 NULL,         FALSE },   // 11
  { WEAPON_NONE,            NULL,                 NULL,         FALSE }, //{ WEAPON_FLAMER,          &_toWFlamer,          &_aaiAmmo[4], FALSE },   // 12
  { WEAPON_NONE,            NULL,                 NULL,         FALSE },   // 13
  { WEAPON_LASER,           &_toWLaser,           &_aaiAmmo[5], FALSE },   // 14
  { WEAPON_NONE,            NULL,                 NULL,         FALSE }, //{ WEAPON_GHOSTBUSTER,     &_toWGhostBuster,     &_aaiAmmo[5], FALSE },   // 15
  { WEAPON_IRONCANNON,      &_toWIronCannon,      &_aaiAmmo[6], FALSE },   // 16
  { WEAPON_NONE,            NULL,                 NULL,         FALSE }, //{ WEAPON_NUKECANNON,      &_toWNukeCannon,      &_aaiAmmo[7], FALSE },   // 17
};


// compare functions for qsort()
static int qsort_CompareNames( const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer**)ppPEN0;
  CPlayer &en1 = **(CPlayer**)ppPEN1;
  CTString strName0 = en0.GetPlayerName();
  CTString strName1 = en1.GetPlayerName();
  return strnicmp( strName0, strName1, 8);
}

static int qsort_CompareScores( const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer**)ppPEN0;
  CPlayer &en1 = **(CPlayer**)ppPEN1;
  SLONG sl0 = en0.m_psGameStats.ps_iScore;
  SLONG sl1 = en1.m_psGameStats.ps_iScore;
  if(      sl0<sl1) return +1;
  else if( sl0>sl1) return -1;
  else              return  0;
}

static int qsort_CompareHealth( const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer**)ppPEN0;
  CPlayer &en1 = **(CPlayer**)ppPEN1;
  SLONG sl0 = (SLONG)ceil(en0.GetHealth());
  SLONG sl1 = (SLONG)ceil(en1.GetHealth());
  if(      sl0<sl1) return +1;
  else if( sl0>sl1) return -1;
  else              return  0;
}

static int qsort_CompareManas( const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer**)ppPEN0;
  CPlayer &en1 = **(CPlayer**)ppPEN1;
  SLONG sl0 = en0.m_iMana;
  SLONG sl1 = en1.m_iMana;
  if(      sl0<sl1) return +1;
  else if( sl0>sl1) return -1;
  else              return  0;
}

static int qsort_CompareFrags( const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer**)ppPEN0;
  CPlayer &en1 = **(CPlayer**)ppPEN1;
  SLONG sl0 = en0.m_psGameStats.ps_iKills;
  SLONG sl1 = en1.m_psGameStats.ps_iKills;
  if(      sl0<sl1) return +1;
  else if( sl0>sl1) return -1;
  else              return  0;
}

static int qsort_CompareDeaths( const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer**)ppPEN0;
  CPlayer &en1 = **(CPlayer**)ppPEN1;
  SLONG sl0 = en0.m_psGameStats.ps_iDeaths;
  SLONG sl1 = en1.m_psGameStats.ps_iDeaths;
  if(      sl0<sl1) return +1;
  else if( sl0>sl1) return -1;
  else              return  0;
}

#if 0 // DG: unused
static int qsort_CompareLatencies( const void *ppPEN0, const void *ppPEN1) {
  CPlayer &en0 = **(CPlayer**)ppPEN0;
  CPlayer &en1 = **(CPlayer**)ppPEN1;
  SLONG sl0 = (SLONG)ceil(en0.m_tmLatency);
  SLONG sl1 = (SLONG)ceil(en1.m_tmLatency);
  if(      sl0<sl1) return +1;
  else if( sl0>sl1) return -1;
  else              return  0;
}
#endif // 0 (unused)

// prepare color transitions
static void PrepareColorTransitions( COLOR colFine, COLOR colHigh, COLOR colMedium, COLOR colLow,
                                     FLOAT fMediumHigh, FLOAT fLowMedium, BOOL bSmooth)
{
  _cttHUD.ctt_colFine     = colFine;
  _cttHUD.ctt_colHigh     = colHigh;   
  _cttHUD.ctt_colMedium   = colMedium;
  _cttHUD.ctt_colLow      = colLow;
  _cttHUD.ctt_fMediumHigh = fMediumHigh;
  _cttHUD.ctt_fLowMedium  = fLowMedium;
  _cttHUD.ctt_bSmooth     = bSmooth;
}



// calculates shake ammount and color value depanding on value change
#define SHAKE_TIME (2.0f)
static COLOR AddShaker( PIX const pixAmmount, INDEX const iCurrentValue, INDEX &iLastValue,
                        TIME &tmChanged, FLOAT &fMoverX, FLOAT &fMoverY)
{
  // update shaking if needed
  fMoverX = fMoverY = 0.0f;
  const TIME tmNow = _pTimer->GetLerpedCurrentTick();
  if( iCurrentValue != iLastValue) {
    iLastValue = iCurrentValue;
    tmChanged  = tmNow;
  } else {
    // in case of loading (timer got reseted)
    tmChanged = ClampUp( tmChanged, tmNow);
  }
  
  // no shaker?
  const TIME tmDelta = tmNow - tmChanged;
  if( tmDelta > SHAKE_TIME) return NONE;
  ASSERT( tmDelta>=0);
  // shake, baby shake!
  const FLOAT fAmmount    = _fResolutionScaling * _fCustomScaling * pixAmmount;
  const FLOAT fMultiplier = (SHAKE_TIME-tmDelta)/SHAKE_TIME *fAmmount;
  const INDEX iRandomizer = (INDEX)(tmNow*511.0f)*fAmmount*iCurrentValue;
  const FLOAT fNormRnd1   = (FLOAT)((iRandomizer ^ (iRandomizer>>9)) & 1023) * 0.0009775f;  // 1/1023 - normalized
  const FLOAT fNormRnd2   = (FLOAT)((iRandomizer ^ (iRandomizer>>7)) & 1023) * 0.0009775f;  // 1/1023 - normalized
  fMoverX = (fNormRnd1 -0.5f) * fMultiplier;
  fMoverY = (fNormRnd2 -0.5f) * fMultiplier;
  // clamp to adjusted ammount (pixels relative to resolution and HUD scale
  fMoverX = Clamp( fMoverX, -fAmmount, fAmmount);
  fMoverY = Clamp( fMoverY, -fAmmount, fAmmount);
  if( tmDelta < SHAKE_TIME/3) return C_WHITE;
  else return NONE;
//return FloatToInt(tmDelta*4) & 1 ? C_WHITE : NONE;
}


// get current color from local color transitions table
static COLOR GetCurrentColor( FLOAT fNormalizedValue)
{
  // if value is in 'low' zone just return plain 'low' alert color
  if( fNormalizedValue < _cttHUD.ctt_fLowMedium) return( _cttHUD.ctt_colLow & 0xFFFFFF00);
  // if value is in out of 'extreme' zone just return 'extreme' color
  if( fNormalizedValue > 1.0f) return( _cttHUD.ctt_colFine & 0xFFFFFF00);
 
  COLOR col;
  // should blend colors?
  if( _cttHUD.ctt_bSmooth)
  { // lets do some interpolations
    FLOAT fd, f1, f2;
    COLOR col1, col2;
    UBYTE ubH,ubS,ubV, ubH2,ubS2,ubV2;
    // determine two colors for interpolation
    if( fNormalizedValue > _cttHUD.ctt_fMediumHigh) {
      f1   = 1.0f;
      f2   = _cttHUD.ctt_fMediumHigh;
      col1 = _cttHUD.ctt_colHigh;
      col2 = _cttHUD.ctt_colMedium;
    } else { // fNormalizedValue > _cttHUD.ctt_fLowMedium == TRUE !
      f1   = _cttHUD.ctt_fMediumHigh;
      f2   = _cttHUD.ctt_fLowMedium;
      col1 = _cttHUD.ctt_colMedium;
      col2 = _cttHUD.ctt_colLow;
    }
    // determine interpolation strength
    fd = (fNormalizedValue-f2) / (f1-f2);
    // convert colors to HSV
    ColorToHSV( col1, ubH,  ubS,  ubV);
    ColorToHSV( col2, ubH2, ubS2, ubV2);
    // interpolate H, S and V components
    ubH = (UBYTE)(ubH*fd + ubH2*(1.0f-fd));
    ubS = (UBYTE)(ubS*fd + ubS2*(1.0f-fd));
    ubV = (UBYTE)(ubV*fd + ubV2*(1.0f-fd));
    // convert HSV back to COLOR
    col = HSVToColor( ubH, ubS, ubV);
  }
  else
  { // simple color picker
    col = _cttHUD.ctt_colMedium;
    if( fNormalizedValue > _cttHUD.ctt_fMediumHigh) col = _cttHUD.ctt_colHigh;
  }
  // all done
  return( col & 0xFFFFFF00);
}



// fill array with players' statistics (returns current number of players in game)
extern INDEX SetAllPlayersStats( INDEX iSortKey)
{
  // determine maximum number of players for this session
  INDEX iPlayers    = 0;
  INDEX iMaxPlayers = _penPlayer->GetMaxPlayers();
  CPlayer *penCurrent;
  // loop thru potentional players 
  for( INDEX i=0; i<iMaxPlayers; i++)
  { // ignore non-existent players
    penCurrent = (CPlayer*)&*_penPlayer->GetPlayerEntity(i);
    if( penCurrent==NULL) continue;
    // fill in player parameters
    _apenPlayers[iPlayers] = penCurrent;
    // advance to next real player
    iPlayers++;
  }
  // sort statistics by some key if needed
  switch( iSortKey) {
  case PSK_NAME:    qsort( _apenPlayers, iPlayers, sizeof(CPlayer*), qsort_CompareNames);   break;
  case PSK_SCORE:   qsort( _apenPlayers, iPlayers, sizeof(CPlayer*), qsort_CompareScores);  break;
  case PSK_HEALTH:  qsort( _apenPlayers, iPlayers, sizeof(CPlayer*), qsort_CompareHealth);  break;
  case PSK_MANA:    qsort( _apenPlayers, iPlayers, sizeof(CPlayer*), qsort_CompareManas);   break;
  case PSK_FRAGS:   qsort( _apenPlayers, iPlayers, sizeof(CPlayer*), qsort_CompareFrags);   break;
  case PSK_DEATHS:  qsort( _apenPlayers, iPlayers, sizeof(CPlayer*), qsort_CompareDeaths);  break;
  default:  break;  // invalid or NONE key specified so do nothing
  }
  // all done
  return iPlayers;
}



// ----------------------- drawing functions

// draw border with filter
static void HUD_DrawBorder( FLOAT fCenterX, FLOAT fCenterY, FLOAT fSizeX, FLOAT fSizeY, COLOR colTiles)
{
  // determine location
  const FLOAT fCenterI  = fCenterX*_pixDPWidth  / 640.0f;
  const FLOAT fCenterJ  = fCenterY*_pixDPHeight / (480.0f * _pDP->dp_fWideAdjustment);
  const FLOAT fSizeI    = _fResolutionScaling*fSizeX;
  const FLOAT fSizeJ    = _fResolutionScaling*fSizeY;
  const FLOAT fTileSize = 8*_fResolutionScaling*_fCustomScaling;
  // determine exact positions
  const FLOAT fLeft  = fCenterI  - fSizeI/2 -1; 
  const FLOAT fRight = fCenterI  + fSizeI/2 +1; 
  const FLOAT fUp    = fCenterJ  - fSizeJ/2 -1; 
  const FLOAT fDown  = fCenterJ  + fSizeJ/2 +1;
  const FLOAT fLeftEnd  = fLeft  + fTileSize;
  const FLOAT fRightBeg = fRight - fTileSize; 
  const FLOAT fUpEnd    = fUp    + fTileSize; 
  const FLOAT fDownBeg  = fDown  - fTileSize; 
  // prepare texture                 
  colTiles |= _ulAlphaHUD;
  // put corners
  _pDP->InitTexture( &_toTile, TRUE); // clamping on!
  _pDP->AddTexture( fLeft, fUp,   fLeftEnd, fUpEnd,   colTiles);
  _pDP->AddTexture( fRight,fUp,   fRightBeg,fUpEnd,   colTiles);
  _pDP->AddTexture( fRight,fDown, fRightBeg,fDownBeg, colTiles);
  _pDP->AddTexture( fLeft, fDown, fLeftEnd, fDownBeg, colTiles);
  // put edges
  _pDP->AddTexture( fLeftEnd,fUp,    fRightBeg,fUpEnd,   0.4f,0.0f, 0.6f,1.0f, colTiles);
  _pDP->AddTexture( fLeftEnd,fDown,  fRightBeg,fDownBeg, 0.4f,0.0f, 0.6f,1.0f, colTiles);
  _pDP->AddTexture( fLeft,   fUpEnd, fLeftEnd, fDownBeg, 0.0f,0.4f, 1.0f,0.6f, colTiles);
  _pDP->AddTexture( fRight,  fUpEnd, fRightBeg,fDownBeg, 0.0f,0.4f, 1.0f,0.6f, colTiles);
  // put center
  _pDP->AddTexture( fLeftEnd, fUpEnd, fRightBeg, fDownBeg, 0.4f,0.4f, 0.6f,0.6f, colTiles);
  _pDP->FlushRenderingQueue();
}


// draw icon texture (if color = NONE, use colortransitions structure)
static void HUD_DrawIcon( FLOAT fCenterX, FLOAT fCenterY, CTextureObject &toIcon,
                          COLOR colDefault, FLOAT fNormValue, BOOL bBlink)
{
  // determine color
  COLOR col = colDefault;
  if( col==NONE) col = GetCurrentColor( fNormValue);
  // determine blinking state
  if( bBlink && fNormValue<=(_cttHUD.ctt_fLowMedium/2)) {
    // activate blinking only if value is <= half the low edge
    INDEX iCurrentTime = (INDEX)(_tmNow*4);
    if( iCurrentTime&1) col = C_vdGRAY;
  }
  // determine location
  const FLOAT fCenterI = fCenterX*_pixDPWidth  / 640.0f;
  const FLOAT fCenterJ = fCenterY*_pixDPHeight / (480.0f * _pDP->dp_fWideAdjustment);
  // determine dimensions
  CTextureData *ptd = (CTextureData*)toIcon.GetData();
  const FLOAT fHalfSizeI = _fResolutionScaling*_fCustomScaling * ptd->GetPixWidth()  *0.5f;
  const FLOAT fHalfSizeJ = _fResolutionScaling*_fCustomScaling * ptd->GetPixHeight() *0.5f;
  // done
  _pDP->InitTexture( &toIcon);
  _pDP->AddTexture( fCenterI-fHalfSizeI, fCenterJ-fHalfSizeJ,
                    fCenterI+fHalfSizeI, fCenterJ+fHalfSizeJ, col|_ulAlphaHUD);
  _pDP->FlushRenderingQueue();
}


// draw text (or numbers, whatever)
static void HUD_DrawText( FLOAT fCenterX, FLOAT fCenterY, const CTString &strText,
                          COLOR colDefault, FLOAT fNormValue)
{
  // determine color
  COLOR col = colDefault;
  if( col==NONE) col = GetCurrentColor( fNormValue);
  // determine location
  PIX pixCenterI = (PIX)(fCenterX*_pixDPWidth  / 640.0f);
  PIX pixCenterJ = (PIX)(fCenterY*_pixDPHeight / (480.0f * _pDP->dp_fWideAdjustment));
  // done
  _pDP->SetTextScaling( _fResolutionScaling*_fCustomScaling);
  _pDP->PutTextCXY( strText, pixCenterI, pixCenterJ, col|_ulAlphaHUD);
}


// draw bar
static void HUD_DrawBar( FLOAT fCenterX, FLOAT fCenterY, PIX pixSizeX, PIX pixSizeY,
                         enum BarOrientations eBarOrientation, COLOR colDefault, FLOAT fNormValue)
{
  // determine color
  COLOR col = colDefault;
  if( col==NONE) col = GetCurrentColor( fNormValue);
  // determine location and size
  PIX pixCenterI = (PIX)(fCenterX*_pixDPWidth  / 640.0f);
  PIX pixCenterJ = (PIX)(fCenterY*_pixDPHeight / (480.0f * _pDP->dp_fWideAdjustment));
  PIX pixSizeI   = (PIX)(_fResolutionScaling*pixSizeX);
  PIX pixSizeJ   = (PIX)(_fResolutionScaling*pixSizeY);
  // fill bar background area
  PIX pixLeft  = pixCenterI-pixSizeI/2;
  PIX pixUpper = pixCenterJ-pixSizeJ/2;
  // determine bar position and inner size
  switch( eBarOrientation) {
  case BO_UP:
    pixSizeJ *= fNormValue;
    break;
  case BO_DOWN:
    pixUpper  = pixUpper + (PIX)ceil(pixSizeJ * (1.0f-fNormValue));
    pixSizeJ *= fNormValue;
    break;
  case BO_LEFT:
    pixSizeI *= fNormValue;
    break;
  case BO_RIGHT:
    pixLeft   = pixLeft + (PIX)ceil(pixSizeI * (1.0f-fNormValue));
    pixSizeI *= fNormValue;
    break;
  }
  // done
  _pDP->Fill( pixLeft, pixUpper, pixSizeI, pixSizeJ, col|_ulAlphaHUD);
}


// helper functions

// fill weapon and ammo table with current state
static void FillWeaponAmmoTables(void)
{
  // ammo quantities
  _aaiAmmo[0].ai_iAmmoAmmount    = _penWeapons->m_iShells;
  _aaiAmmo[0].ai_iMaxAmmoAmmount = _penWeapons->m_iMaxShells;
  _aaiAmmo[1].ai_iAmmoAmmount    = _penWeapons->m_iBullets;
  _aaiAmmo[1].ai_iMaxAmmoAmmount = _penWeapons->m_iMaxBullets;
  _aaiAmmo[2].ai_iAmmoAmmount    = _penWeapons->m_iRockets;
  _aaiAmmo[2].ai_iMaxAmmoAmmount = _penWeapons->m_iMaxRockets;
  _aaiAmmo[3].ai_iAmmoAmmount    = _penWeapons->m_iGrenades;
  _aaiAmmo[3].ai_iMaxAmmoAmmount = _penWeapons->m_iMaxGrenades;
  _aaiAmmo[4].ai_iAmmoAmmount    = 0;//_penWeapons->m_iNapalm;
  _aaiAmmo[4].ai_iMaxAmmoAmmount = 0;//_penWeapons->m_iMaxNapalm;
  _aaiAmmo[5].ai_iAmmoAmmount    = _penWeapons->m_iElectricity;
  _aaiAmmo[5].ai_iMaxAmmoAmmount = _penWeapons->m_iMaxElectricity;
  _aaiAmmo[6].ai_iAmmoAmmount    = _penWeapons->m_iIronBalls;
  _aaiAmmo[6].ai_iMaxAmmoAmmount = _penWeapons->m_iMaxIronBalls;
  _aaiAmmo[7].ai_iAmmoAmmount    = 0;//_penWeapons->m_iNukeBalls;
  _aaiAmmo[7].ai_iMaxAmmoAmmount = 0;//_penWeapons->m_iMaxNukeBalls;

  // prepare ammo table for weapon possesion
  INDEX i, iAvailableWeapons = _penWeapons->m_iAvailableWeapons;
  for( i=0; i<8; i++) _aaiAmmo[i].ai_bHasWeapon = FALSE;
  // weapon possesion
  for( i=WEAPON_NONE+1; i<WEAPON_LAST; i++)
  {
    if ( _awiWeapons[i].wi_wtWeapon == WEAPON_NONE) {
      continue;
    }

    // regular weapons
    _awiWeapons[i].wi_bHasWeapon = (iAvailableWeapons&(1<<(_awiWeapons[i].wi_wtWeapon-1)));

    if (_awiWeapons[i].wi_paiAmmo != NULL) {
      _awiWeapons[i].wi_paiAmmo->ai_bHasWeapon |= _awiWeapons[i].wi_bHasWeapon;
    }
  }
}



// main

// render interface (frontend) to drawport
// (units are in pixels for 640x480 resolution - for other res HUD will be scalled automatically)
extern void DrawHUD( const CPlayer *penPlayerCurrent, CDrawPort *pdpCurrent, BOOL bSnooping)
{
  // no player - no info, sorry
  if( penPlayerCurrent==NULL || (penPlayerCurrent->GetFlags()&ENF_DELETED)) return;

  // find last values in case of predictor
  CPlayer *penLast = (CPlayer*)penPlayerCurrent;
  if( penPlayerCurrent->IsPredictor()) penLast = (CPlayer*)(((CPlayer*)penPlayerCurrent)->GetPredicted());
  ASSERT( penLast!=NULL);
  if( penLast==NULL) return; // !!!! just in case

  // cache local variables
  hud_fOpacity = Clamp( hud_fOpacity, 0.1f, 1.0f);
  hud_fScaling = Clamp( hud_fScaling, 0.5f, 1.2f);
  _penPlayer  = penPlayerCurrent;
  _penWeapons = (CPlayerWeapons*)&*_penPlayer->m_penWeapons;
  _pDP        = pdpCurrent;
  _pixDPWidth   = _pDP->GetWidth();
  _pixDPHeight  = _pDP->GetHeight();
  _fCustomScaling     = hud_fScaling;
  _fResolutionScaling = (FLOAT)_pixDPWidth /640.0f;
  _colHUD     = C_GREEN;
  _ulAlphaHUD = NormFloatToByte(hud_fOpacity);
  _tmNow = _pTimer->CurrentTick();

  // set HUD colorization;
  COLOR colMax = _colHUD;
  COLOR colTop = _colHUD;
  COLOR colMid = _colHUD;

  // adjust borders color in case of spying mode
  COLOR colBorder = _colHUD; 
  if( bSnooping) {
    UBYTE ubR,ubG,ubB;
    ColorToRGB( colBorder, ubR,ubG,ubB);
    colBorder = RGBToColor( ubG,ubB,ubR); // shift and xor color components
    if( ((ULONG)(_tmNow*5))&1) {
      colBorder = (colBorder>>1) & 0x7F7F7F00; // darken flash and scale
      _fCustomScaling *= 0.933f;
    }
  }

  // prepare font and text dimensions
  CTString strValue;
  PIX pixCharWidth;
  FLOAT fValue, fNormValue, fCol, fRow;
  _pDP->SetFont( &_fdNumbersFont);
  pixCharWidth = _fdNumbersFont.GetWidth() + _fdNumbersFont.GetCharSpacing() +1;
  FLOAT fChrUnit = pixCharWidth * _fCustomScaling;

  const PIX pixTopBound    = 6;
  const PIX pixLeftBound   = 6;
  const PIX pixBottomBound = (480 * _pDP->dp_fWideAdjustment) -pixTopBound;
  const PIX pixRightBound  = 640-pixLeftBound;
  FLOAT fOneUnit  = (32+0) * _fCustomScaling;  // unit size
  FLOAT fAdvUnit  = (32+4) * _fCustomScaling;  // unit advancer
  FLOAT fNextUnit = (32+8) * _fCustomScaling;  // unit advancer
  FLOAT fHalfUnit = fOneUnit * 0.5f;
  FLOAT fMoverX, fMoverY;
  COLOR colDefault;

  // prepare and draw health info
  fValue = ClampDn( _penPlayer->GetHealth(), 0.0f);  // never show negative health
  fNormValue = fValue/TOP_HEALTH;
  strValue.PrintF( "%d", (SLONG)ceil(fValue));
  PrepareColorTransitions( colMax, colTop, colMid, C_RED, 0.5f, 0.25f, FALSE);
  fRow = pixBottomBound-fHalfUnit;
  fCol = pixLeftBound+fHalfUnit;
  colDefault = AddShaker( 5, fValue, penLast->m_iLastHealth, penLast->m_tmHealthChanged, fMoverX, fMoverY);
  HUD_DrawBorder( fCol+fMoverX, fRow+fMoverY, fOneUnit, fOneUnit, colBorder);
  fCol += fAdvUnit+fChrUnit*3/2 -fHalfUnit;
  HUD_DrawBorder( fCol, fRow, fChrUnit*3, fOneUnit, colBorder);
  HUD_DrawText( fCol, fRow, strValue, colDefault, fNormValue);
  fCol -= fAdvUnit+fChrUnit*3/2 -fHalfUnit;
  HUD_DrawIcon( fCol+fMoverX, fRow+fMoverY, _toHealth, _colHUD, fNormValue, TRUE);

  // prepare and draw armor info (eventually)
  fValue = _penPlayer->m_fArmor;
  if( fValue > 0.0f) {
    fNormValue = fValue/TOP_ARMOR;
    strValue.PrintF( "%d", (SLONG)ceil(fValue));
    PrepareColorTransitions( colMax, colTop, colMid, C_lGRAY, 0.5f, 0.25f, FALSE);
    fRow = pixBottomBound- (fNextUnit+fHalfUnit);//_pDP->dp_fWideAdjustment;
    fCol = pixLeftBound+    fHalfUnit;
    colDefault = AddShaker( 3, fValue, penLast->m_iLastArmor, penLast->m_tmArmorChanged, fMoverX, fMoverY);
    HUD_DrawBorder( fCol+fMoverX, fRow+fMoverY-12, fOneUnit, fOneUnit, colBorder);
    fCol += fAdvUnit+fChrUnit*3/2 -fHalfUnit;
    HUD_DrawBorder( fCol, fRow-12, fChrUnit*3, fOneUnit, colBorder);
    HUD_DrawText( fCol, fRow-12, strValue, NONE, fNormValue);
    fCol -= fAdvUnit+fChrUnit*3/2 -fHalfUnit;
    HUD_DrawIcon( fCol+fMoverX, fRow+fMoverY-12, _toArmor, _colHUD, fNormValue, FALSE);
  }

  // prepare and draw ammo and weapon info
  CTextureObject *ptoCurrentAmmo=NULL, *ptoCurrentWeapon=NULL, *ptoWantedWeapon=NULL;
  INDEX iCurrentWeapon = _penWeapons->m_iCurrentWeapon;
  INDEX iWantedWeapon  = _penWeapons->m_iWantedWeapon;
  // determine corresponding ammo and weapon texture component
  ptoCurrentWeapon = _awiWeapons[iCurrentWeapon].wi_ptoWeapon;
  ptoWantedWeapon  = _awiWeapons[iWantedWeapon].wi_ptoWeapon;

  AmmoInfo *paiCurrent = _awiWeapons[iCurrentWeapon].wi_paiAmmo;
  if( paiCurrent!=NULL) ptoCurrentAmmo = paiCurrent->ai_ptoAmmo;

  // draw complete weapon info if knife isn't current weapon
  if( ptoCurrentAmmo!=NULL && !GetSP()->sp_bInfiniteAmmo) {
    // determine ammo quantities
    FLOAT fMaxValue = _penWeapons->GetMaxAmmo();
    fValue = _penWeapons->GetAmmo();
    fNormValue = fValue / fMaxValue;
    strValue.PrintF( "%d", (SLONG)ceil(fValue));
    PrepareColorTransitions( colMax, colTop, colMid, C_RED, 0.5f, 0.25f, FALSE);
    BOOL bDrawAmmoIcon = _fCustomScaling<=1.0f;
    // draw ammo, value and weapon
    fRow = pixBottomBound-fHalfUnit;
    fCol = 175 + fHalfUnit;
    colDefault = AddShaker( 4, fValue, penLast->m_iLastAmmo, penLast->m_tmAmmoChanged, fMoverX, fMoverY);
    HUD_DrawBorder( fCol+fMoverX, fRow+fMoverY, fOneUnit, fOneUnit, colBorder);
    fCol += fAdvUnit+fChrUnit*3/2 -fHalfUnit;
    HUD_DrawBorder( fCol, fRow, fChrUnit*3, fOneUnit, colBorder);
    if( bDrawAmmoIcon) {
      fCol += fAdvUnit+fChrUnit*3/2 -fHalfUnit;
      HUD_DrawBorder( fCol, fRow, fOneUnit, fOneUnit, colBorder);
      HUD_DrawIcon( fCol, fRow, *ptoCurrentAmmo, _colHUD, fNormValue, TRUE);
      fCol -= fAdvUnit+fChrUnit*3/2 -fHalfUnit;
    }
    HUD_DrawText( fCol, fRow, strValue, colDefault, fNormValue);
    fCol -= fAdvUnit+fChrUnit*3/2 -fHalfUnit;
    HUD_DrawIcon( fCol+fMoverX, fRow+fMoverY, *ptoCurrentWeapon, _colHUD, fNormValue, !bDrawAmmoIcon);
  } else if( ptoCurrentWeapon!=NULL) {
    // draw only knife or colt icons (ammo is irrelevant)
    fRow = pixBottomBound-fHalfUnit;
    fCol = 205 + fHalfUnit;
    HUD_DrawBorder( fCol, fRow, fOneUnit, fOneUnit, colBorder);
    HUD_DrawIcon(   fCol, fRow, *ptoCurrentWeapon, _colHUD, fNormValue, FALSE);
  }


  // display all ammo infos
  INDEX i;
  FLOAT fAdv;
  COLOR colIcon, colBar;
  PrepareColorTransitions( colMax, colTop, colMid, C_RED, 0.5f, 0.25f, FALSE);
  // reduce the size of icon slightly
  _fCustomScaling = ClampDn( _fCustomScaling*0.8f, 0.5f);
  const FLOAT fOneUnitS  = fOneUnit  *0.8f;
  const FLOAT fAdvUnitS  = fAdvUnit  *0.8f;
  //const FLOAT fNextUnitS = fNextUnit *0.8f;
  const FLOAT fHalfUnitS = fHalfUnit *0.8f;

  // prepare postition and ammo quantities
  fRow = pixBottomBound-fHalfUnitS;
  fCol = pixRightBound -fHalfUnitS;
  const FLOAT fBarPos = fHalfUnitS*0.7f;
  FillWeaponAmmoTables();

  // loop thru all ammo types
  if (!GetSP()->sp_bInfiniteAmmo) {
    for( i=7; i>=0; i--) {
      // if no ammo and hasn't got that weapon - just skip this ammo
      AmmoInfo &ai = _aaiAmmo[i];
      ASSERT( ai.ai_iAmmoAmmount>=0);
      if( ai.ai_iAmmoAmmount==0 && !ai.ai_bHasWeapon) continue;
      // display ammo info
      colIcon = _colHUD;
      if( ai.ai_iAmmoAmmount==0) colIcon = C_GRAY;
      if( ptoCurrentAmmo == ai.ai_ptoAmmo) colIcon = C_WHITE; 
      fNormValue = (FLOAT)ai.ai_iAmmoAmmount / ai.ai_iMaxAmmoAmmount;
      colBar = AddShaker( 4, ai.ai_iAmmoAmmount, ai.ai_iLastAmmoAmmount, ai.ai_tmAmmoChanged, fMoverX, fMoverY);
      HUD_DrawBorder( fCol,         fRow+fMoverY, fOneUnitS, fOneUnitS, colBorder);
      HUD_DrawIcon(   fCol,         fRow+fMoverY, *_aaiAmmo[i].ai_ptoAmmo, colIcon, fNormValue, FALSE);
      HUD_DrawBar(    fCol+fBarPos, fRow+fMoverY, fOneUnitS/5, fOneUnitS-2, BO_DOWN, colBar, fNormValue);
      // advance to next position
      fCol -= fAdvUnitS;
    }
  }

  // if weapon change is in progress
  _fCustomScaling = hud_fScaling;
  hud_tmWeaponsOnScreen = Clamp( hud_tmWeaponsOnScreen, 0.0f, 10.0f);   
  if( (_tmNow - _penWeapons->m_tmWeaponChangeRequired) < hud_tmWeaponsOnScreen) {
    // determine number of weapons that player has
    INDEX ctWeapons = 0;
    for( i=WEAPON_NONE+1; i<WEAPON_LAST; i++) {
      if( _awiWeapons[i].wi_wtWeapon!=WEAPON_NONE && _awiWeapons[i].wi_wtWeapon!=WEAPON_DOUBLECOLT &&
          _awiWeapons[i].wi_bHasWeapon) ctWeapons++;
    }
    // display all available weapons
    fRow = pixBottomBound - fHalfUnit - 3*fNextUnit;
    fCol = 320.0f - (ctWeapons*fAdvUnit-fHalfUnit)/2.0f;
    // display all available weapons
    for( i=WEAPON_NONE+1; i<WEAPON_LAST; i++) {
      // skip if hasn't got this weapon
      if( _awiWeapons[i].wi_wtWeapon==WEAPON_NONE       ||  _awiWeapons[i].wi_wtWeapon==WEAPON_DOUBLECOLT
       /*|| _awiWeapons[i].wi_wtWeapon==WEAPON_NUKECANNON*/ || !_awiWeapons[i].wi_bHasWeapon) continue;
      // display weapon icon
      colIcon = _colHUD;
      if( _awiWeapons[i].wi_paiAmmo!=NULL && _awiWeapons[i].wi_paiAmmo->ai_iAmmoAmmount==0) colIcon = C_dGRAY;
      if( ptoWantedWeapon == _awiWeapons[i].wi_ptoWeapon) colIcon = C_WHITE;
      HUD_DrawBorder( fCol, fRow, fOneUnit, fOneUnit, colIcon);
      HUD_DrawIcon(   fCol, fRow, *_awiWeapons[i].wi_ptoWeapon, colIcon, 1.0f, FALSE);
      // advance to next position
      fCol += fAdvUnit;
    }
  }

  const FLOAT fUpperSize = ClampDn(_fCustomScaling*0.5f, 0.5f)/_fCustomScaling;
  _fCustomScaling*=fUpperSize;
  ASSERT( _fCustomScaling>=0.5f);
  fChrUnit  *= fUpperSize;
  fOneUnit  *= fUpperSize;
  fHalfUnit *= fUpperSize;
  fAdvUnit  *= fUpperSize;
  fNextUnit *= fUpperSize;

  // draw oxygen info if needed
  BOOL bOxygenOnScreen = FALSE;
  fValue = _penPlayer->en_tmMaxHoldBreath - (_pTimer->CurrentTick() - _penPlayer->en_tmLastBreathed);
  if( _penPlayer->IsConnected() && (_penPlayer->GetFlags()&ENF_ALIVE) && fValue<30.0f) { 
    // prepare and draw oxygen info
    fRow = pixTopBound + fOneUnit + fNextUnit;
    fCol = 280.0f;
    fAdv = fAdvUnit + fOneUnit*4/2 - fHalfUnit;
    PrepareColorTransitions( colMax, colTop, colMid, C_RED, 0.5f, 0.25f, FALSE);
    fNormValue = fValue/30.0f;
    fNormValue = ClampDn(fNormValue, 0.0f);
    HUD_DrawBorder( fCol,      fRow, fOneUnit,         fOneUnit, colBorder);
    HUD_DrawBorder( fCol+fAdv, fRow, fOneUnit*4,       fOneUnit, colBorder);
    HUD_DrawBar(    fCol+fAdv, fRow, fOneUnit*4*0.975, fOneUnit*0.9375, BO_LEFT, NONE, fNormValue);
    HUD_DrawIcon(   fCol,      fRow, _toOxygen, _colHUD, fNormValue, TRUE);
    bOxygenOnScreen = TRUE;
  }

  // draw boss energy if needed
  if( _penPlayer->m_penMainMusicHolder!=NULL) {
    CMusicHolder &mh = (CMusicHolder&)*_penPlayer->m_penMainMusicHolder;
    fNormValue = 0;

    if( mh.m_penBoss!=NULL && (mh.m_penBoss->en_ulFlags&ENF_ALIVE)) {
      CEnemyBase &eb = (CEnemyBase&)*mh.m_penBoss;
      ASSERT( eb.m_fMaxHealth>0);
      fValue = eb.GetHealth();
      fNormValue = fValue/eb.m_fMaxHealth;
    }
    if( mh.m_penCounter!=NULL) {
      CEnemyCounter &ec = (CEnemyCounter&)*mh.m_penCounter;
      if (ec.m_iCount>0) {
        fValue = ec.m_iCount;
        fNormValue = fValue/ec.m_iCountFrom;
      }
    }
    if (fNormValue>0) {
      // prepare and draw boss energy info
      PrepareColorTransitions( colMax, colTop, colMid, C_RED, 0.5f, 0.25f, FALSE);
      fRow = pixTopBound + fOneUnit + fNextUnit;
      fCol = 184.0f;
      fAdv = fAdvUnit+ fOneUnit*16/2 -fHalfUnit;
      if( bOxygenOnScreen) fRow += fNextUnit;
      HUD_DrawBorder( fCol,      fRow, fOneUnit,          fOneUnit, colBorder);
      HUD_DrawBorder( fCol+fAdv, fRow, fOneUnit*16,       fOneUnit, colBorder);
      HUD_DrawBar(    fCol+fAdv, fRow, fOneUnit*16*0.995, fOneUnit*0.9375, BO_LEFT, NONE, fNormValue);
      HUD_DrawIcon(   fCol,      fRow, _toHealth, _colHUD, fNormValue, FALSE);
    }
  }

  // determine scaling of normal text and play mode
  const FLOAT fTextScale  = (_fResolutionScaling+1) *0.5f;
  const BOOL bSinglePlay  =  GetSP()->sp_bSinglePlayer;
  const BOOL bCooperative =  GetSP()->sp_bCooperative && !bSinglePlay;
  const BOOL bScoreMatch  = !GetSP()->sp_bCooperative && !GetSP()->sp_bUseFrags;
  const BOOL bFragMatch   = !GetSP()->sp_bCooperative &&  GetSP()->sp_bUseFrags;
  COLOR colMana, colFrags, colDeaths, colHealth, colArmor;
  COLOR colScore  = _colHUD;
  INDEX iScoreSum = 0;

  // if not in single player mode, we'll have to calc (and maybe printout) other players' info
  if( !bSinglePlay)
  {
    // set font and prepare font parameters
    _pfdDisplayFont->SetVariableWidth();
    _pDP->SetFont( _pfdDisplayFont);
    _pDP->SetTextScaling( fTextScale);
    FLOAT fCharHeight = (_pfdDisplayFont->GetHeight()-2)*fTextScale;
    // generate and sort by mana list of active players
    BOOL bMaxScore=TRUE, bMaxMana=TRUE, bMaxFrags=TRUE, bMaxDeaths=TRUE;
    hud_iSortPlayers = Clamp( hud_iSortPlayers, -1, 6);
    SortKeys eKey = (SortKeys)hud_iSortPlayers;
    if (hud_iSortPlayers==-1) {
           if (bCooperative) eKey = PSK_HEALTH;
      else if (bScoreMatch)  eKey = PSK_SCORE;
      else if (bFragMatch)   eKey = PSK_FRAGS;
      else { ASSERT(FALSE);  eKey = PSK_NAME; }
    }
    if( bCooperative) eKey = (SortKeys)Clamp( (INDEX)eKey, 0, 3);
    if( eKey==PSK_HEALTH && (bScoreMatch || bFragMatch)) { eKey = PSK_NAME; }; // prevent health snooping in deathmatch
    INDEX iPlayers = SetAllPlayersStats(eKey);
    // loop thru players 
    for( INDEX i=0; i<iPlayers; i++)
    { // get player name and mana
      CPlayer *penPlayer = _apenPlayers[i];
      const CTString strName = penPlayer->GetPlayerName();
      const INDEX iScore  = penPlayer->m_psGameStats.ps_iScore;
      const INDEX iMana   = penPlayer->m_iMana;
      const INDEX iFrags  = penPlayer->m_psGameStats.ps_iKills;
      const INDEX iDeaths = penPlayer->m_psGameStats.ps_iDeaths;
      const INDEX iHealth = ClampDn( (INDEX)ceil( penPlayer->GetHealth()), 0);
      const INDEX iArmor  = ClampDn( (INDEX)ceil( penPlayer->m_fArmor),    0);
      CTString strScore, strMana, strFrags, strDeaths, strHealth, strArmor;
      strScore.PrintF(  "%d", iScore);
      strMana.PrintF(   "%d", iMana);
      strFrags.PrintF(  "%d", iFrags);
      strDeaths.PrintF( "%d", iDeaths);
      strHealth.PrintF( "%d", iHealth);
      strArmor.PrintF(  "%d", iArmor);
      // detemine corresponding colors
      colHealth = C_mlRED;
      colMana = colScore = colFrags = colDeaths = colArmor = C_lGRAY;
      if( iMana   > _penPlayer->m_iMana)                      { bMaxMana   = FALSE; colMana   = C_WHITE; }
      if( iScore  > _penPlayer->m_psGameStats.ps_iScore)      { bMaxScore  = FALSE; colScore  = C_WHITE; }
      if( iFrags  > _penPlayer->m_psGameStats.ps_iKills)      { bMaxFrags  = FALSE; colFrags  = C_WHITE; }
      if( iDeaths > _penPlayer->m_psGameStats.ps_iDeaths)     { bMaxDeaths = FALSE; colDeaths = C_WHITE; }
      if( penPlayer==_penPlayer) colScore = colMana = colFrags = colDeaths = _colHUD; // current player
      if( iHealth>25) colHealth = _colHUD;
      if( iArmor >25) colArmor  = _colHUD;
      // eventually print it out
      if( hud_iShowPlayers==1 || (hud_iShowPlayers==-1 && !bSinglePlay)) {
        // printout location and info aren't the same for deathmatch and coop play
        const FLOAT fCharWidth = (PIX)((_pfdDisplayFont->GetWidth()-2) *fTextScale);
        if( bCooperative) { 
          _pDP->PutTextR( strName+":", _pixDPWidth-8*fCharWidth, fCharHeight*i+fOneUnit*2, colScore |_ulAlphaHUD);
          _pDP->PutText(  "/",         _pixDPWidth-4*fCharWidth, fCharHeight*i+fOneUnit*2, _colHUD  |_ulAlphaHUD);
          _pDP->PutTextC( strHealth,   _pixDPWidth-6*fCharWidth, fCharHeight*i+fOneUnit*2, colHealth|_ulAlphaHUD);
          _pDP->PutTextC( strArmor,    _pixDPWidth-2*fCharWidth, fCharHeight*i+fOneUnit*2, colArmor |_ulAlphaHUD);
        } else if( bScoreMatch) { 
          _pDP->PutTextR( strName+":", _pixDPWidth-12*fCharWidth, fCharHeight*i+fOneUnit*2, _colHUD |_ulAlphaHUD);
          _pDP->PutText(  "/",         _pixDPWidth- 5*fCharWidth, fCharHeight*i+fOneUnit*2, _colHUD |_ulAlphaHUD);
          _pDP->PutTextC( strScore,    _pixDPWidth- 8*fCharWidth, fCharHeight*i+fOneUnit*2, colScore|_ulAlphaHUD);
          _pDP->PutTextC( strMana,     _pixDPWidth- 2*fCharWidth, fCharHeight*i+fOneUnit*2, colMana |_ulAlphaHUD);
        } else { // fragmatch!
          _pDP->PutTextR( strName+":", _pixDPWidth-8*fCharWidth, fCharHeight*i+fOneUnit*2, _colHUD  |_ulAlphaHUD);
          _pDP->PutText(  "/",         _pixDPWidth-4*fCharWidth, fCharHeight*i+fOneUnit*2, _colHUD  |_ulAlphaHUD);
          _pDP->PutTextC( strFrags,    _pixDPWidth-6*fCharWidth, fCharHeight*i+fOneUnit*2, colFrags |_ulAlphaHUD);
          _pDP->PutTextC( strDeaths,   _pixDPWidth-2*fCharWidth, fCharHeight*i+fOneUnit*2, colDeaths|_ulAlphaHUD);
        }
      }
      // calculate summ of scores (for coop mode)
      iScoreSum += iScore;  
    }
  
    // prepare color for local player printouts
    bMaxScore  ? colScore  = C_WHITE : colScore  = C_lGRAY;
    bMaxMana   ? colMana   = C_WHITE : colMana   = C_lGRAY;
    bMaxFrags  ? colFrags  = C_WHITE : colFrags  = C_lGRAY;
    bMaxDeaths ? colDeaths = C_WHITE : colDeaths = C_lGRAY;
  }

  // printout player latency if needed
  if( hud_bShowLatency) {
    CTString strLatency;
    strLatency.PrintF( "%4.0fms", _penPlayer->m_tmLatency*1000.0f);
    PIX pixFontHeight = (PIX)(_pfdDisplayFont->GetHeight() *fTextScale +fTextScale+1);
    _pfdDisplayFont->SetFixedWidth();
    _pDP->SetFont( _pfdDisplayFont);
    _pDP->SetTextScaling( fTextScale);
    _pDP->SetTextCharSpacing( -2.0f*fTextScale);
    _pDP->PutTextR( strLatency, _pixDPWidth, _pixDPHeight-pixFontHeight, C_WHITE|CT_OPAQUE);
  }
  // restore font defaults
  _pfdDisplayFont->SetVariableWidth();
  _pDP->SetFont( &_fdNumbersFont);
  _pDP->SetTextCharSpacing(1);

  // prepare output strings and formats depending on game type
  FLOAT fWidthAdj = 8;
  INDEX iScore = _penPlayer->m_psGameStats.ps_iScore;
  INDEX iMana  = _penPlayer->m_iMana;
  if( bFragMatch) {
    fWidthAdj = 4;
    iScore = _penPlayer->m_psGameStats.ps_iKills;
    iMana  = _penPlayer->m_psGameStats.ps_iDeaths;
  } else if( bCooperative) {
    // in case of coop play, show squad (common) score
    iScore = iScoreSum;
  }

  // prepare and draw score or frags info 
  strValue.PrintF( "%d", iScore);
  fRow = pixTopBound  +fHalfUnit;
  fCol = pixLeftBound +fHalfUnit;
  fAdv = fAdvUnit+ fChrUnit*fWidthAdj/2 -fHalfUnit;
  HUD_DrawBorder( fCol,      fRow, fOneUnit,           fOneUnit, colBorder);
  HUD_DrawBorder( fCol+fAdv, fRow, fChrUnit*fWidthAdj, fOneUnit, colBorder);
  HUD_DrawText(   fCol+fAdv, fRow, strValue, colScore, 1.0f);
  HUD_DrawIcon(   fCol,      fRow, _toFrags, colScore, 1.0f, FALSE);

  // eventually draw mana info 
  if( bScoreMatch || bFragMatch) {
    strValue.PrintF( "%d", iMana);
    fRow = pixTopBound  + fNextUnit+fHalfUnit;
    fCol = pixLeftBound + fHalfUnit;
    fAdv = fAdvUnit+ fChrUnit*fWidthAdj/2 -fHalfUnit;
    HUD_DrawBorder( fCol,      fRow, fOneUnit,           fOneUnit, colBorder);
    HUD_DrawBorder( fCol+fAdv, fRow, fChrUnit*fWidthAdj, fOneUnit, colBorder);
    HUD_DrawText(   fCol+fAdv, fRow, strValue,  colMana, 1.0f);
    HUD_DrawIcon(   fCol,      fRow, _toDeaths, colMana, 1.0f, FALSE);
  }

  // if single player or cooperative mode
  if( bSinglePlay || bCooperative)
  {
    // prepare and draw hiscore info 
    strValue.PrintF( "%d", Max(_penPlayer->m_iHighScore, _penPlayer->m_psGameStats.ps_iScore));
    BOOL bBeating = _penPlayer->m_psGameStats.ps_iScore>_penPlayer->m_iHighScore;
    fRow = pixTopBound+fHalfUnit;
    fCol = 320.0f-fOneUnit-fChrUnit*8/2;
    fAdv = fAdvUnit+ fChrUnit*8/2 -fHalfUnit;
    HUD_DrawBorder( fCol,      fRow, fOneUnit,   fOneUnit, colBorder);
    HUD_DrawBorder( fCol+fAdv, fRow, fChrUnit*8, fOneUnit, colBorder);
    HUD_DrawText(   fCol+fAdv, fRow, strValue, NONE, bBeating ? 0.0f : 1.0f);
    HUD_DrawIcon(   fCol,      fRow, _toHiScore, _colHUD, 1.0f, FALSE);

    // prepare and draw unread messages
    if( hud_bShowMessages && _penPlayer->m_ctUnreadMessages>0) {
      strValue.PrintF( "%d", _penPlayer->m_ctUnreadMessages);
      fRow = pixTopBound+fHalfUnit;
      fCol = pixRightBound-fHalfUnit-fAdvUnit-fChrUnit*4;
      const FLOAT tmIn = 0.5f;
      const FLOAT tmOut = 0.5f;
      const FLOAT tmStay = 2.0f;
      FLOAT tmDelta = _pTimer->GetLerpedCurrentTick()-_penPlayer->m_tmAnimateInbox;
      COLOR col = _colHUD;
      if (tmDelta>0 && tmDelta<(tmIn+tmStay+tmOut) && bSinglePlay) {
        FLOAT fRatio = 0.0f;
        if (tmDelta<tmIn) {
          fRatio = tmDelta/tmIn;
        } else if (tmDelta>tmIn+tmStay) {
          fRatio = (tmIn+tmStay+tmOut-tmDelta)/tmOut;
        } else {
          fRatio = 1.0f;
        }
        fRow+=fAdvUnit*5*fRatio;
        fCol-=fAdvUnit*15*fRatio;
        col = LerpColor(_colHUD, C_WHITE|0xFF, fRatio);
      }
      fAdv = fAdvUnit+ fChrUnit*4/2 -fHalfUnit;
      HUD_DrawBorder( fCol,      fRow, fOneUnit,   fOneUnit, col);
      HUD_DrawBorder( fCol+fAdv, fRow, fChrUnit*4, fOneUnit, col);
      HUD_DrawText(   fCol+fAdv, fRow, strValue,   col, 1.0f);
      HUD_DrawIcon(   fCol,      fRow, _toMessage, col, 0.0f, TRUE);
    }
  }

  // draw cheat modes
  if( GetSP()->sp_ctMaxPlayers==1) {
    INDEX iLine=1;
    ULONG ulAlpha = sin(_tmNow*16)*96 +128;
    PIX pixFontHeight = _pfdConsoleFont->fd_pixCharHeight;
    const COLOR colCheat = _colHUD;
    _pDP->SetFont( _pfdConsoleFont);
    _pDP->SetTextScaling( g_cb.globalScale);
    const FLOAT fchtTM = cht_fTranslationMultiplier; // for text formatting sake :)
    if( fchtTM > 1.0f)  { _pDP->PutTextR( "turbo",     _pixDPWidth-1, _pixDPHeight-pixFontHeight*iLine, colCheat|ulAlpha); iLine++; }
    if( cht_bInvisible) { _pDP->PutTextR( "invisible", _pixDPWidth-1, _pixDPHeight-pixFontHeight*iLine, colCheat|ulAlpha); iLine++; }
    if( cht_bGhost)     { _pDP->PutTextR( "ghost",     _pixDPWidth-1, _pixDPHeight-pixFontHeight*iLine, colCheat|ulAlpha); iLine++; }
    if( cht_bFly)       { _pDP->PutTextR( "fly",       _pixDPWidth-1, _pixDPHeight-pixFontHeight*iLine, colCheat|ulAlpha); iLine++; }
    if( cht_bGod)       { _pDP->PutTextR( "god",       _pixDPWidth-1, _pixDPHeight-pixFontHeight*iLine, colCheat|ulAlpha); iLine++; }
  }
}



// initialized all whats need for drawing HUD
extern void InitHUD(void)
{
  // try to
  try {
    // initialize and load HUD numbers font
    DECLARE_CTFILENAME( fnFont, "Fonts\\Numbers3.fnt");
    _fdNumbersFont.Load_t( fnFont);
    //_fdNumbersFont.SetCharSpacing(0);

    // initialize status bar textures
    _toHealth.SetData_t(  CTFILENAME("Textures\\Interface\\HSuper.tex"));
    _toArmor.SetData_t(   CTFILENAME("Textures\\Interface\\ArStrong.tex"));
    _toOxygen.SetData_t(  CTFILENAME("Textures\\Interface\\Oxygen-2.tex"));
    _toFrags.SetData_t(   CTFILENAME("Textures\\Interface\\IBead.tex"));
    _toDeaths.SetData_t(  CTFILENAME("Textures\\Interface\\ISkull.tex"));
    _toScore.SetData_t(   CTFILENAME("Textures\\Interface\\IScore.tex"));
    _toHiScore.SetData_t( CTFILENAME("Textures\\Interface\\IHiScore.tex"));
    _toMessage.SetData_t( CTFILENAME("Textures\\Interface\\IMessage.tex"));
    _toMana.SetData_t(    CTFILENAME("Textures\\Interface\\IValue.tex"));
    // initialize ammo textures                    
    _toAShells.SetData_t(      CTFILENAME("Textures\\Interface\\AmShells.tex"));
    _toABullets.SetData_t(     CTFILENAME("Textures\\Interface\\AmBullets.tex"));
    _toARockets.SetData_t(     CTFILENAME("Textures\\Interface\\AmRockets.tex"));
    _toAGrenades.SetData_t(    CTFILENAME("Textures\\Interface\\AmGrenades.tex"));
    _toANapalm.SetData_t(      CTFILENAME("Textures\\Interface\\AmFuelReservoir.tex"));
    _toAElectricity.SetData_t( CTFILENAME("Textures\\Interface\\AmElectricity.tex"));
    _toAIronBall.SetData_t(    CTFILENAME("Textures\\Interface\\AmCannon.tex"));
    // initialize weapon textures
    _toWKnife.SetData_t(           CTFILENAME("Textures\\Interface\\WKnife.tex"));
    _toWColt.SetData_t(            CTFILENAME("Textures\\Interface\\WColt.tex"));
    _toWSingleShotgun.SetData_t(   CTFILENAME("Textures\\Interface\\WSingleShotgun.tex"));
    _toWDoubleShotgun.SetData_t(   CTFILENAME("Textures\\Interface\\WDoubleShotgun.tex"));
    _toWTommygun.SetData_t(        CTFILENAME("Textures\\Interface\\WTommygun.tex"));
    _toWMinigun.SetData_t(         CTFILENAME("Textures\\Interface\\WMinigun.tex"));
    _toWRocketLauncher.SetData_t(  CTFILENAME("Textures\\Interface\\WRocketLauncher.tex"));
    _toWGrenadeLauncher.SetData_t( CTFILENAME("Textures\\Interface\\WGrenadeLauncher.tex"));
    _toWPipeBomb.SetData_t(        CTFILENAME("Textures\\Interface\\WPipeBomb.tex"));
    _toWFlamer.SetData_t(          CTFILENAME("Textures\\Interface\\WFlamer.tex"));
    _toWGhostBuster.SetData_t(     CTFILENAME("Textures\\Interface\\WGhostBuster.tex"));
    _toWLaser.SetData_t(           CTFILENAME("Textures\\Interface\\WLaser.tex"));
    _toWIronCannon.SetData_t(      CTFILENAME("Textures\\Interface\\WCannon.tex"));
    // initialize tile texture
    _toTile.SetData_t( CTFILENAME("Textures\\Interface\\Tile.tex"));

    // set all textures as constant
    ((CTextureData*)_toHealth .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toArmor  .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toOxygen .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toFrags  .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toDeaths .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toScore  .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toHiScore.GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toMessage.GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toMana   .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toAShells     .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toABullets    .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toARockets    .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toAGrenades   .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toANapalm     .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toAElectricity.GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toAIronBall   .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWKnife          .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWColt           .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWSingleShotgun  .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWDoubleShotgun  .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWTommygun       .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWMinigun        .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWRocketLauncher .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWGrenadeLauncher.GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWPipeBomb       .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWFlamer         .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWGhostBuster    .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWLaser          .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toWIronCannon     .GetData())->Force(TEX_CONSTANT);
    ((CTextureData*)_toTile            .GetData())->Force(TEX_CONSTANT);
  }
  catch( char *strError) {
    FatalError( strError);
  }

}


// clean up
extern void EndHUD(void)
{

}

