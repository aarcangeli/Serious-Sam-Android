105
%{
#include "Entities/StdH/StdH.h"
%}

%{

extern DECL_DLL void JumpFromBouncer(CEntity *penToBounce, CEntity *penBouncer)
{
  CEntity *pen = penToBounce;
  CBouncer *pbo = (CBouncer *)penBouncer;
  // if it is a movable model and some time has passed from the last jump
  if ( (pen->GetRenderType()==CEntity::RT_MODEL) &&
       (pen->GetPhysicsFlags()&EPF_MOVABLE) ) {
    CMovableEntity *pmen = (CMovableEntity *)pen;
    if (pmen->en_penReference==NULL) {
      return;
    }
    // give it speed
    FLOAT3D vDir;
    AnglesToDirectionVector(pbo->m_aDirection, vDir);
    pmen->FakeJump(pmen->en_vIntendedTranslation, vDir, pbo->m_fSpeed, 
      -pbo->m_fParallelComponentMultiplier, pbo->m_fNormalComponentMultiplier, pbo->m_fMaxExitSpeed, pbo->m_tmControl);
  }
}

%}

class CBouncer : CRationalEntity {
name      "Bouncer";
thumbnail "Thumbnails\\Bouncer.tbn";
features  "HasName";

properties:
  1 CTString m_strName            "Name" 'N' = "Bouncer",
  2 CTString m_strDescription = "",
  
  4 FLOAT m_fSpeed                "Speed [m/s]" 'S' = 20.0f,
  5 ANGLE3D m_aDirection          "Direction" 'D' = ANGLE3D(0,90,0),
  6 FLOAT m_tmControl             "Control time" 'T' = 5.0f,
  7 BOOL m_bEntrySpeed            = TRUE,
 10 FLOAT m_fMaxExitSpeed                 "Max exit speed" 'M' = 200.0f,
 12 FLOAT m_fNormalComponentMultiplier    "Normal component multiplier" 'O' = 1.0f,
 13 FLOAT m_fParallelComponentMultiplier  "Parallel component multiplier" 'P' = 0.0f,

components:
functions:
procedures:
  Main() {
    // declare yourself as a brush
    InitAsBrush();
    SetPhysicsFlags(EPF_BRUSH_FIXED|EPF_NOIMPACT);
    SetCollisionFlags(ECF_BRUSH);

    // if old flag "entry speed" has been reset
    if (!m_bEntrySpeed)
    {
      // kill normal component by default (same behaviour by default)
      m_fNormalComponentMultiplier = 0.0f;
      m_bEntrySpeed = TRUE;
    }
    return;
  }
};
                                                  