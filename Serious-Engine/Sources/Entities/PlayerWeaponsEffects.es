405
%{
#include "Entities/StdH/StdH.h"
#define EPF_MODEL_SHELL (EPF_ONBLOCK_BOUNCE|EPF_TRANSLATEDBYGRAVITY|EPF_MOVABLE)
#define ECF_MODEL_SHELL ( \
  ((ECBI_BRUSH|ECBI_MODEL_HOLDER)<<ECB_TEST) |\
  ((ECBI_CORPSE)<<ECB_IS))
%}

uses "Entities/Player";

enum WeaponEffectType {
  0 WET_SHOTGUNSHELL          "",   // shotgun shell
  1 WET_MACHINEGUNSHELL       "",   // machinegun shell
};

// input parameter for viewer
event EWeaponEffectInit {
  CEntityPointer penOwner,            // who owns it
  enum WeaponEffectType EwetEffect,   // effect type
};

%{
void CPlayerWeaponsEffects_Precache(void)
{
  CDLLEntityClass *pdec = &CPlayerWeaponsEffects_DLLClass;
  pdec->PrecacheModel(MODEL_SG_SHELL);
  pdec->PrecacheTexture(TEXTURE_SG_SHELL);
  pdec->PrecacheModel(MODEL_MG_SHELL);
  pdec->PrecacheTexture(TEXTURE_MG_SHELL);
}
%}

class CPlayerWeaponsEffects: CMovableEntity {
name      "Player Weapons Effects";
thumbnail "";
features "CanBePredictable";

properties:
  1 CEntityPointer m_penOwner,            // class which owns it
  2 enum WeaponEffectType m_EwetEffect = WET_SHOTGUNSHELL,   // weapon effect type

components:
// ************** SHOTGUN SHELL ************
  1 model   MODEL_SG_SHELL              "Models\\Weapons\\SingleShotgun\\Shell\\Shell.mdl",
  2 texture TEXTURE_SG_SHELL            "Models\\Weapons\\SingleShotgun\\Shell\\Shell.tex",
// ************** MACHINEGUN SHELL ************
  3 model   MODEL_MG_SHELL              "Models\\Weapons\\Minigun\\Shell\\Shell.mdl",
  4 texture TEXTURE_MG_SHELL            "Models\\Weapons\\Minigun\\Shell\\Shell.tex",

functions:
procedures:
  ShotgunShell(EVoid) {
    // init as model
    InitAsModel();
    SetPhysicsFlags(EPF_MODEL_SHELL);
    SetCollisionFlags(ECF_MODEL_SHELL);
    // set appearance
    GetModelObject()->StretchModel(FLOAT3D(0.5f, 0.5f, 0.5f));
    SetModel(MODEL_SG_SHELL);
    ModelChangeNotify();
    SetModelMainTexture(TEXTURE_SG_SHELL);
    // speed
    LaunchAsFreeProjectile(FLOAT3D(FRnd()+2.0f, FRnd()+5.0f, -FRnd()-2.0f), (CMovableEntity*)&*m_penOwner);
    // wait a while
    autowait(1.5f);
    return EEnd();
  };
  MachinegunShell(EVoid) {
    // init as model
    InitAsModel();
    SetPhysicsFlags(EPF_MODEL_SHELL);
    SetCollisionFlags(ECF_MODEL_SHELL);
    // set appearance
    GetModelObject()->StretchModel(FLOAT3D(0.5f, 0.5f, 0.5f));
    SetModel(MODEL_MG_SHELL);
    ModelChangeNotify();
    SetModelMainTexture(TEXTURE_MG_SHELL);
    // speed
    LaunchAsFreeProjectile(FLOAT3D(FRnd()+2.0f, FRnd()+5.0f, -FRnd()-2.0f), (CMovableEntity*)&*m_penOwner);
    // wait a while
    autowait(0.5f);
    return EEnd();
  };

  Main(EWeaponEffectInit eInit) {
    // remember the initial parameters
    ASSERT(eInit.penOwner!=NULL);
    m_penOwner = eInit.penOwner;
    m_EwetEffect = eInit.EwetEffect;
    SetFlags(GetFlags()|ENF_SEETHROUGH);
    SetPredictable(TRUE);

    if (m_EwetEffect==WET_SHOTGUNSHELL) {
      autocall ShotgunShell() EEnd;
    } else if (m_EwetEffect==WET_MACHINEGUNSHELL) {
      autocall MachinegunShell() EEnd;
    } else {
      ASSERTALWAYS("Uknown weapon effect type");
    }

    // cease to exist
    Destroy();

    return;
  };
};

