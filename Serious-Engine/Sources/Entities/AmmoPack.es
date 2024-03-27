806
%{
#include "Entities/StdH/StdH.h"
#include "Models/Items/ItemHolder/ItemHolder.h"
%}

uses "Entities/Item";

// ammo type 
enum AmmoPackType {
  1 APT_CUSTOM        "Custom pack",
  2 APT_SERIOUS       "Serious pack",
};

// event for sending through receive item
event EAmmoPackItem {
  INDEX iShells,                
  INDEX iBullets,                
  INDEX iRockets,                
  INDEX iGrenades,                
//  INDEX iNapalm,                
  INDEX iElectricity,                
  INDEX iIronBalls,                
//  INDEX iNukeBalls,                
};

class CAmmoPack : CItem {
name      "Ammo Pack";
thumbnail "Thumbnails\\AmmoPack.tbn";

properties:
  1 enum AmmoPackType  m_aptPackType    "Type" 'Y' = APT_CUSTOM,     // pack type

 10 INDEX m_iShells                "Shells"        'S'   = MAX_SHELLS,
 11 INDEX m_iBullets               "Bullets"       'B'   = MAX_BULLETS, 
 12 INDEX m_iRockets               "Rockets"       'C'   = MAX_ROCKETS, 
 13 INDEX m_iGrenades              "Grenades"      'G'   = MAX_GRENADES,
// 14 INDEX m_iNapalm                "Napalm"        'P'   = MAX_NAPALM,
 15 INDEX m_iElectricity           "Electricity"   'E'   = MAX_ELECTRICITY,
 16 INDEX m_iIronBalls             "Iron balls"    'I'   = MAX_IRONBALLS,
// 17 INDEX m_iNukeBalls             "Nuke balls"    'U'   = MAX_NUKEBALLS,

components:
  0 class   CLASS_BASE        "Classes\\Item.ecl",

// ********* BACK PACK *********
 60 model   MODEL_BACKPACK      "Models\\Items\\PowerUps\\BackPack\\BackPack.mdl",
 61 texture TEXTURE_BACKPACK    "Models\\Items\\PowerUps\\BackPack\\BackPack.tex",

// ********* SERIOUS PACK *********
 70 model   MODEL_SERIOUSPACK      "Models\\Items\\PowerUps\\SeriousPack\\SeriousPack.mdl",
 71 texture TEXTURE_SERIOUSPACK    "Models\\Items\\PowerUps\\SeriousPack\\SeriousPack.tex",

// ************** FLARE FOR EFFECT **************
100 texture TEXTURE_FLARE "Models\\Items\\Flares\\Flare.tex",
101 model   MODEL_FLARE "Models\\Items\\Flares\\Flare.mdl",

// ************** SOUNDS **************
213 sound SOUND_PICK             "Sounds\\Items\\Ammo.wav",

functions:
  void Precache(void) {
    PrecacheSound(SOUND_PICK);
  }

  // render particles
  void RenderParticles(void)
  {
    // no particles when not existing or in DM modes
    if (GetRenderType()!=CEntity::RT_MODEL || GetSP()->sp_gmGameMode>CSessionProperties::GM_COOPERATIVE
      || !ShowItemParticles())
    {
      return;
    }

    Particles_Spiral(this, 3.0f*0.5, 2.5f*0.5, PT_STAR04, 10);
  }

  /* Fill in entity statistics - for AI purposes only */
  BOOL FillEntityStatistics(EntityStats *pes)
  {
    pes->es_ctCount = 1;
    pes->es_ctAmmount = 1;
    // compile description
//    pes->es_strName.PrintF("Back pack: %d Shells, %d Bullets, %d Rockets, %d Grenades, %d Napalm, %d Electricity, %d Iron balls, %d Nuke balls",
//      m_iShells, m_iBullets, m_iRockets, m_iGrenades, m_iNapalm, m_iElectricity, m_iIronBalls, m_iNukeBalls); 
    pes->es_strName.PrintF("Back pack: %d Shells, %d Bullets, %d Rockets, %d Grenades, %d Electricity, %d Iron balls",
      m_iShells, m_iBullets, m_iRockets, m_iGrenades, m_iElectricity, m_iIronBalls); 

    // calculate value
    pes->es_fValue = 
      m_iShells*AV_SHELLS + 
      m_iBullets*AV_BULLETS + 
      m_iRockets*AV_ROCKETS + 
      m_iGrenades*AV_GRENADES + 
//      m_iNapalm*AV_NAPALM + 
      m_iElectricity*AV_ELECTRICITY + 
      m_iIronBalls*AV_IRONBALLS /*+ 
      m_iNukeBalls*AV_NUKEBALLS*/;

    pes->es_iScore = 0;
    return TRUE;
  }

  // set ammo properties depending on ammo type
  void SetProperties(void)
  {
    switch (m_aptPackType)
    {
      case APT_SERIOUS:
        m_strDescription = "Serious:";
        // set appearance
        AddItem(MODEL_SERIOUSPACK, TEXTURE_SERIOUSPACK, 0,0,0);
        AddFlare(MODEL_FLARE, TEXTURE_FLARE, FLOAT3D(0,0.75f,0), FLOAT3D(2,2,1.3f) );
        StretchItem(FLOAT3D(0.5f, 0.5f, 0.5f));
        break;
      case APT_CUSTOM:
        m_strDescription = "Custom:";
        // set appearance
        AddItem(MODEL_BACKPACK, TEXTURE_BACKPACK, 0,0,0);
        AddFlare(MODEL_FLARE, TEXTURE_FLARE, FLOAT3D(0,0.75f,0), FLOAT3D(2,2,1.3f) );
        StretchItem(FLOAT3D(0.5f, 0.5f, 0.5f));
        break;
      default: ASSERTALWAYS("Uknown ammo");
    }

    m_fValue = 1.0f;
    m_fRespawnTime = 30.0f;
    if( m_iShells != 0) {m_strDescription.PrintF("%s: Shells (%d)", (const char *) m_strDescription, m_iShells);}
    if( m_iBullets != 0) {m_strDescription.PrintF("%s: Bullets (%d)", (const char *) m_strDescription, m_iBullets);}
    if( m_iRockets != 0) {m_strDescription.PrintF("%s: Rockets (%d)", (const char *) m_strDescription, m_iRockets);}
    if( m_iGrenades != 0) {m_strDescription.PrintF("%s: Grenades (%d)", (const char *) m_strDescription, m_iGrenades);}
//    if( m_iNapalm != 0) {m_strDescription.PrintF("%s: Napalm (%d)", (const char *) m_strDescription, m_iNapalm);}
    if( m_iElectricity != 0) {m_strDescription.PrintF("%s: Electricity (%d)", (const char *) m_strDescription, m_iElectricity);}
    if( m_iIronBalls != 0) {m_strDescription.PrintF("%s: Iron balls (%d)", (const char *) m_strDescription, m_iIronBalls);}
//    if( m_iNukeBalls != 0) {m_strDescription.PrintF("%s: Nuke balls (%d)", (const char *) m_strDescription, m_iNukeBalls);}
  }

  void AdjustDifficulty(void)
  {
    //m_fValue = ceil(m_fValue*GetSP()->sp_fAmmoQuantity);

    if (GetSP()->sp_bInfiniteAmmo && m_penTarget==NULL) {
      Destroy();
    }
  }

procedures:
  ItemCollected(EPass epass) : CItem::ItemCollected
  {
    ASSERT(epass.penOther!=NULL);

    // if ammo stays
    if (GetSP()->sp_bAmmoStays && !m_bPickupOnce) {
      // if already picked by this player
      BOOL bWasPicked = MarkPickedBy(epass.penOther);
      if (bWasPicked) {
        // don't pick again
        return;
      }
    }

    // send ammo to entity
    EAmmoPackItem eAmmo;
    eAmmo.iShells = m_iShells;
    eAmmo.iBullets = m_iBullets;
    eAmmo.iRockets = m_iRockets;
    eAmmo.iGrenades = m_iGrenades;
//    eAmmo.iNapalm = m_iNapalm;
    eAmmo.iElectricity = m_iElectricity;
    eAmmo.iIronBalls = m_iIronBalls;
//    eAmmo.iNukeBalls = m_iNukeBalls;
    // if health is received
    if (epass.penOther->ReceiveItem(eAmmo)) {
      // play the pickup sound
      m_soPick.Set3DParameters(50.0f, 1.0f, 1.0f, 1.0f);
      PlaySound(m_soPick, SOUND_PICK, SOF_3D);
      m_fPickSoundLen = GetSoundLength(SOUND_PICK);
      if (!GetSP()->sp_bAmmoStays || m_bPickupOnce) {
        jump CItem::ItemReceived();
      }
    }
    return;
  };

  Main() {
    m_iShells = Clamp( m_iShells, INDEX(0), MAX_SHELLS);
    m_iBullets = Clamp( m_iBullets, INDEX(0), MAX_BULLETS);
    m_iRockets = Clamp( m_iRockets, INDEX(0), MAX_ROCKETS);
    m_iGrenades = Clamp( m_iGrenades, INDEX(0), MAX_GRENADES);
//    m_iNapalm = Clamp( m_iNapalm, INDEX(0), MAX_NAPALM);
    m_iElectricity = Clamp( m_iElectricity, INDEX(0), MAX_ELECTRICITY);
    m_iIronBalls = Clamp( m_iIronBalls, INDEX(0), MAX_IRONBALLS);
//    m_iNukeBalls = Clamp( m_iNukeBalls, INDEX(0), MAX_NUKEBALLS);
    
    Initialize();     // initialize base class
    StartModelAnim(ITEMHOLDER_ANIM_MEDIUMOSCILATION, AOF_LOOPING|AOF_NORESTART);
    ForceCollisionBoxIndexChange(ITEMHOLDER_COLLISION_BOX_MEDIUM);
    SetProperties();  // set properties

    jump CItem::ItemLoop();
  };
};
