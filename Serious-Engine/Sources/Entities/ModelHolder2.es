210
%{
#include "Entities/StdH/StdH.h"
%}

uses "Entities/ModelDestruction";
uses "Entities/AnimationChanger";
uses "Entities/BloodSpray";

enum CustomShadingType {
  0 CST_NONE             "None",
  1 CST_FULL_CUSTOMIZED  "Full customized",
  2 CST_CONSTANT_SHADING "Constant shading" 
};

enum ShadowType {
  0 ST_NONE      "None",
  1 ST_CLUSTER   "Cluster shadows",
  2 ST_POLYGONAL "Polygonal" 
};

class CModelHolder2 : CRationalEntity {
name      "ModelHolder2";
thumbnail "Thumbnails\\ModelHolder.tbn";
features "HasName", "HasDescription";

properties:
  1 CTFileName m_fnModel      "Model" 'M' =CTFILENAME("Models\\Editor\\Axis.mdl"),
  2 CTFileName m_fnTexture    "Texture" 'T' =CTFILENAME("Models\\Editor\\Vector.tex"),
 22 CTFileName m_fnReflection "Reflection" =CTString(""),
 23 CTFileName m_fnSpecular   "Specular" =CTString(""),
 24 CTFileName m_fnBump       "Bump" =CTString(""),
  3 FLOAT m_fStretchAll       "StretchAll" 'S' = 1.0f,
  4 FLOAT m_fStretchX         "StretchX"   'X' = 1.0f,
  5 FLOAT m_fStretchY         "StretchY"   'Y' = 1.0f,
  6 FLOAT m_fStretchZ         "StretchZ"   'Z' = 1.0f,
  7 CTString m_strName        "Name" 'N' ="",
 12 CTString m_strDescription = "",
  8 BOOL m_bColliding       "Colliding" 'L' = FALSE,    // set if model is not immatierial
  9 ANIMATION m_iModelAnimation   "Model animation" = 0,
 10 ANIMATION m_iTextureAnimation "Texture animation" = 0,
 11 enum ShadowType m_stClusterShadows "Shadows" 'W' = ST_CLUSTER,   // set if model uses cluster shadows
 13 BOOL m_bBackground     "Background" 'B' = FALSE,   // set if model is rendered in background
 21 BOOL m_bTargetable     "Targetable" = FALSE, // st if model should be targetable

 // parameters for custom shading of a model (overrides automatic shading calculation)
 14 enum CustomShadingType m_cstCustomShading "Custom shading" 'H' = CST_NONE,
 15 ANGLE3D m_aShadingDirection "Light direction" 'D' = ANGLE3D( AngleDeg(45.0f),AngleDeg(45.0f),AngleDeg(45.0f)),
 16 COLOR m_colLight            "Light color" 'O' = C_WHITE,
 17 COLOR m_colAmbient          "Ambient color" 'A' = C_BLACK,
 18 CTFileName m_fnmLightAnimation "Light animation file" = CTString(""),
 19 ANIMATION m_iLightAnimation "Light animation" = 0,
 20 CAnimObject m_aoLightAnimation,
 25 BOOL m_bAttachments      "Attachments" = TRUE, // set if model should auto load attachments
 26 BOOL m_bActive "Active" = TRUE,
 31 FLOAT m_fMipAdd "Mip Add" = 0.0f,
 32 FLOAT m_fMipMul "Mip Mul" = 1.0f,
 33 FLOAT m_fMipFadeDist "Mip Fade Dist" = 0.0f,
 34 FLOAT m_fMipFadeLen  "Mip Fade Len" = 0.0f,
 
 // random values variables
 50 BOOL m_bRandomStretch   "Apply RND stretch"   = FALSE, // apply random stretch
 52 FLOAT m_fStretchRndX    "Stretch RND X (%)"   =  0.2f, // random stretch width
 51 FLOAT m_fStretchRndY    "Stretch RND Y (%)"   =  0.2f, // random stretch height
 53 FLOAT m_fStretchRndZ    "Stretch RND Z (%)"   =  0.2f, // random stretch depth
 54 FLOAT m_fStretchRndAll  "Stretch RND All (%)" =  0.0f, // random stretch all
 55 FLOAT3D m_fStretchRandom = FLOAT3D(1, 1, 1),

 // destruction values
 60 CEntityPointer m_penDestruction "Destruction" 'Q' COLOR(C_BLACK|0x20),    // model destruction entity
 61 FLOAT3D m_vDamage = FLOAT3D(0,0,0),    // current damage impact
 62 FLOAT m_tmLastDamage = -1000.0f,
 63 CEntityPointer m_penDestroyTarget "Destruction Target" COLOR(C_WHITE|0xFF), // targeted when destroyed
 64 CEntityPointer m_penLastDamager,
 65 FLOAT m_tmSpraySpawned = 0.0f,   // time when damage has been applied
 66 FLOAT m_fSprayDamage = 0.0f,     // total ammount of damage
 67 CEntityPointer m_penSpray,       // the blood spray
 68 FLOAT m_fMaxDamageAmmount  = 0.0f, // max ammount of damage recived in in last xxx ticks

 70 FLOAT m_fClassificationStretch  "Classification stretch" = 1.0f, // classification box multiplier
100 FLOAT m_fMaxTessellationLevel "Max tessellation level" = 0.0f,

{
  CTFileName m_fnOldModel;  // used for remembering last selected model (not saved at all)
}

components:
  1 class   CLASS_BLOOD_SPRAY     "Classes\\BloodSpray.ecl",

functions:
  void Precache(void) {
    PrecacheClass(CLASS_BLOOD_SPRAY, 0);
  };

  /* Fill in entity statistics - for AI purposes only */
  BOOL FillEntityStatistics(EntityStats *pes)
  {
    pes->es_strName = m_fnModel.FileName()+", "+m_fnTexture.FileName();
    pes->es_ctCount = 1;
    pes->es_ctAmmount = 1;
    if (m_penDestruction!=NULL) {
      pes->es_strName += " (destroyable)";
      pes->es_fValue = GetDestruction()->m_fHealth;
      pes->es_iScore = 0;
    } else {
      pes->es_fValue = 0;
      pes->es_iScore = 0;
    }
    return TRUE;
  }

  // classification box multiplier
  FLOAT3D GetClassificationBoxStretch(void)
  {
    return FLOAT3D( m_fClassificationStretch, m_fClassificationStretch, m_fClassificationStretch);
  }


  // maximum allowed tessellation level for this model (for Truform/N-Patches support)
  FLOAT GetMaxTessellationLevel(void)
  {
    return m_fMaxTessellationLevel;
  }


  /* Receive damage */
  void ReceiveDamage(CEntity *penInflictor, enum DamageType dmtType,
    FLOAT fDamageAmmount, const FLOAT3D &vHitPoint, const FLOAT3D &vDirection) 
  {
    FLOAT fNewDamage = fDamageAmmount;

    // if not destroyable
    if (m_penDestruction==NULL) {
      // do nothing
      return;
    }
    CModelDestruction *penDestruction = GetDestruction();
    // adjust damage
    fNewDamage *=DamageStrength(penDestruction->m_eibtBodyType, dmtType);
    // if no damage
    if (fNewDamage==0) {
      // do nothing
      return;
    }
    FLOAT fKickDamage = fNewDamage;
    if( (dmtType == DMT_EXPLOSION) || (dmtType == DMT_IMPACT) || (dmtType == DMT_CANNONBALL_EXPLOSION) )
    {
      fKickDamage*=1.5f;
    }
    if (dmtType == DMT_CLOSERANGE) {
      fKickDamage=0.0f;
    }
    if(dmtType == DMT_BULLET && penDestruction->m_eibtBodyType==EIBT_ROCK) {
      fKickDamage=0.0f;
    }

    // get passed time since last damage
    TIME tmNow = _pTimer->CurrentTick();
    TIME tmDelta = tmNow-m_tmLastDamage;
    m_tmLastDamage = tmNow;

    // remember who damaged you
    m_penLastDamager = penInflictor;

    // fade damage out
    if (tmDelta>=_pTimer->TickQuantum*3) {
      m_vDamage=FLOAT3D(0,0,0);
    }
    // add new damage
    FLOAT3D vDirectionFixed;
    if (vDirection.ManhattanNorm()>0.5f) {
      vDirectionFixed = vDirection;
    } else {
      vDirectionFixed = FLOAT3D(0,1,0);
    }
    //FLOAT3D vDamageOld = m_vDamage;
    m_vDamage += vDirectionFixed*fKickDamage;

    // NOTE: we don't receive damage here, but handle death differently
    if (m_vDamage.Length()>GetHealth()) {
      EDeath eDeath;  // we don't need any extra parameters
      SendEvent(eDeath);
    }

    if( m_fMaxDamageAmmount<fDamageAmmount) {
      m_fMaxDamageAmmount = fDamageAmmount;
    }

    // if it has no spray, or if this damage overflows it
    if (m_tmSpraySpawned<=_pTimer->CurrentTick()-_pTimer->TickQuantum*8 || 
      m_fSprayDamage+fNewDamage>50.0f) {

      // spawn blood spray
      CPlacement3D plSpray = CPlacement3D( vHitPoint, ANGLE3D(0, 0, 0));
      m_penSpray = CreateEntity( plSpray, CLASS_BLOOD_SPRAY);
      m_penSpray->SetParent( this);
      ESpawnSpray eSpawnSpray;
    
      // adjust spray power
      if( m_fMaxDamageAmmount > 10.0f) {
        eSpawnSpray.fDamagePower = 3.0f;
      } else if(m_fSprayDamage+fNewDamage>50.0f) {
        eSpawnSpray.fDamagePower = 2.0f;
      } else {
        eSpawnSpray.fDamagePower = 1.0f;
      }

      // remember spray type
      eSpawnSpray.sptType = penDestruction->m_sptType;
      eSpawnSpray.fSizeMultiplier = 1.0f;

      // get your down vector (simulates gravity)
      FLOAT3D vDn(-en_mRotation(1,2), -en_mRotation(2,2), -en_mRotation(3,2));
  
      // setup direction of spray
      FLOAT3D vHitPointRelative = vHitPoint - GetPlacement().pl_PositionVector;
      FLOAT3D vReflectingNormal;
      GetNormalComponent( vHitPointRelative, vDn, vReflectingNormal);
      vReflectingNormal.Normalize();
    
      vReflectingNormal(1)/=5.0f;
  
      FLOAT3D vProjectedComponent = vReflectingNormal*(vDirection%vReflectingNormal);
      FLOAT3D vSpilDirection = vDirection-vProjectedComponent*2.0f-vDn*0.5f;

      eSpawnSpray.vDirection = vSpilDirection;
      eSpawnSpray.penOwner = this;
  
      // initialize spray
      m_penSpray->Initialize( eSpawnSpray);
      m_tmSpraySpawned = _pTimer->CurrentTick();
      m_fSprayDamage = 0.0f;
      m_fMaxDamageAmmount = 0.0f;
    }
    m_fSprayDamage+=fNewDamage;
  };

  class CModelDestruction *GetDestruction(void)
  {
    ASSERT(m_penDestruction!=NULL && IsOfClass(m_penDestruction, "ModelDestruction"));
    return (CModelDestruction*) m_penDestruction.ep_pen;
  }
  BOOL IsTargetable(void) const
  {
    return m_bTargetable;
  }

  /* Get anim data for given animation property - return NULL for none. */
  CAnimData *GetAnimData(SLONG slPropertyOffset) 
  {
    if (slPropertyOffset==offsetof(CModelHolder2, m_iModelAnimation)) {
      return GetModelObject()->GetData();
    } else if (slPropertyOffset==offsetof(CModelHolder2, m_iTextureAnimation)) {
      return GetModelObject()->mo_toTexture.GetData();
    } else if (slPropertyOffset==offsetof(CModelHolder2, m_iLightAnimation)) {
      return m_aoLightAnimation.GetData();
    } else {
      return CEntity::GetAnimData(slPropertyOffset);
    }
  };

  /* Adjust model mip factor if needed. */
  void AdjustMipFactor(FLOAT &fMipFactor)
  {
    // if should fade last mip
    if (m_fMipFadeDist>0) {
      CModelObject *pmo = GetModelObject();
      if(pmo==NULL) {
        return;
      }
      // adjust for stretch
      FLOAT fMipForFade = fMipFactor;
      if (pmo->mo_Stretch != FLOAT3D(1,1,1)) {
        fMipForFade -= Log2( Max(pmo->mo_Stretch(1),Max(pmo->mo_Stretch(2),pmo->mo_Stretch(3))));
      }

      // if not visible
      if (fMipForFade>m_fMipFadeDist) {
        // set mip factor so that model is never rendered
        fMipFactor = UpperLimit(0.0f);
        return;
      }

      // adjust fading
      FLOAT fFade = (m_fMipFadeDist-fMipForFade);
      if (m_fMipFadeLen>0) {
        fFade/=m_fMipFadeLen;
      } else {
        if (fFade>0) {
          fFade = 1.0f;
        }
      }
      
      fFade = Clamp(fFade, 0.0f, 1.0f);
      // make it invisible
      pmo->mo_colBlendColor = (pmo->mo_colBlendColor&~255)|UBYTE(255*fFade);
    }

    fMipFactor = fMipFactor*m_fMipMul+m_fMipAdd;
  }

  /* Adjust model shading parameters if needed. */
  BOOL AdjustShadingParameters(FLOAT3D &vLightDirection, COLOR &colLight, COLOR &colAmbient)
  {
    switch( m_cstCustomShading)
    {
    case CST_FULL_CUSTOMIZED:
      {
        // if there is color animation
        if (m_aoLightAnimation.GetData()!=NULL) {
          // get lerping info
          SLONG colFrame0, colFrame1; FLOAT fRatio;
          m_aoLightAnimation.GetFrame( colFrame0, colFrame1, fRatio);
          UBYTE ubAnimR0, ubAnimG0, ubAnimB0;
          UBYTE ubAnimR1, ubAnimG1, ubAnimB1;
          ColorToRGB( colFrame0, ubAnimR0, ubAnimG0, ubAnimB0);
          ColorToRGB( colFrame1, ubAnimR1, ubAnimG1, ubAnimB1);

          // calculate current animation color
          FLOAT fAnimR = NormByteToFloat( Lerp( ubAnimR0, ubAnimR1, fRatio));
          FLOAT fAnimG = NormByteToFloat( Lerp( ubAnimG0, ubAnimG1, fRatio));
          FLOAT fAnimB = NormByteToFloat( Lerp( ubAnimB0, ubAnimB1, fRatio));
          
          // decompose constant colors
          UBYTE ubLightR,   ubLightG,   ubLightB;
          UBYTE ubAmbientR, ubAmbientG, ubAmbientB;
          ColorToRGB( m_colLight,   ubLightR,   ubLightG,   ubLightB);
          ColorToRGB( m_colAmbient, ubAmbientR, ubAmbientG, ubAmbientB);
          colLight   = RGBToColor( ubLightR  *fAnimR, ubLightG  *fAnimG, ubLightB  *fAnimB);
          colAmbient = RGBToColor( ubAmbientR*fAnimR, ubAmbientG*fAnimG, ubAmbientB*fAnimB);

        // if there is no color animation
        } else {
          colLight   = m_colLight;
          colAmbient = m_colAmbient;
        }

        AnglesToDirectionVector(m_aShadingDirection, vLightDirection);
        vLightDirection = -vLightDirection;
        break;
      }
    case CST_CONSTANT_SHADING:
      {
        // combine colors with clamp
        UBYTE lR,lG,lB,aR,aG,aB,rR,rG,rB;
        ColorToRGB( colLight,   lR, lG, lB);
        ColorToRGB( colAmbient, aR, aG, aB);
        colLight = 0;
        rR = (UBYTE) Clamp( (ULONG)lR+aR, (ULONG)0, (ULONG)255);
        rG = (UBYTE) Clamp( (ULONG)lG+aG, (ULONG)0, (ULONG)255);
        rB = (UBYTE) Clamp( (ULONG)lB+aB, (ULONG)0, (ULONG)255);
        colAmbient = RGBToColor( rR, rG, rB);
        break;
      }
    case CST_NONE:
      {
        // do nothing
        break;
      }
    }

    return m_stClusterShadows!=ST_NONE;
  };

  // apply mirror and stretch to the entity
  void MirrorAndStretch(FLOAT fStretch, BOOL bMirrorX)
  {
    m_fStretchAll*=fStretch;
    if (bMirrorX) {
      m_fStretchX = -m_fStretchX;
    }
  }

  // Stretch model
  void StretchModel(void) {
    // stretch factors must not have extreme values
    if (Abs(m_fStretchX)  < 0.01f) { m_fStretchX   = 0.01f;  }
    if (Abs(m_fStretchY)  < 0.01f) { m_fStretchY   = 0.01f;  }
    if (Abs(m_fStretchZ)  < 0.01f) { m_fStretchZ   = 0.01f;  }
    if (m_fStretchAll< 0.01f) { m_fStretchAll = 0.01f;  }

    if (Abs(m_fStretchX)  >100.0f) { m_fStretchX   = 100.0f*Sgn(m_fStretchX); }
    if (Abs(m_fStretchY)  >100.0f) { m_fStretchY   = 100.0f*Sgn(m_fStretchY); }
    if (Abs(m_fStretchZ)  >100.0f) { m_fStretchZ   = 100.0f*Sgn(m_fStretchZ); }
    if (m_fStretchAll>100.0f) { m_fStretchAll = 100.0f; }

    if (m_bRandomStretch) {
      m_bRandomStretch = FALSE;
      // stretch
      m_fStretchRndX   = Clamp( m_fStretchRndX   , 0.0f, 1.0f);
      m_fStretchRndY   = Clamp( m_fStretchRndY   , 0.0f, 1.0f);
      m_fStretchRndZ   = Clamp( m_fStretchRndZ   , 0.0f, 1.0f);
      m_fStretchRndAll = Clamp( m_fStretchRndAll , 0.0f, 1.0f);

      m_fStretchRandom(1) = (FRnd()*m_fStretchRndX*2 - m_fStretchRndX) + 1;
      m_fStretchRandom(2) = (FRnd()*m_fStretchRndY*2 - m_fStretchRndY) + 1;
      m_fStretchRandom(3) = (FRnd()*m_fStretchRndZ*2 - m_fStretchRndZ) + 1;

      FLOAT fRNDAll = (FRnd()*m_fStretchRndAll*2 - m_fStretchRndAll) + 1;
      m_fStretchRandom(1) *= fRNDAll;
      m_fStretchRandom(2) *= fRNDAll;
      m_fStretchRandom(3) *= fRNDAll;
    }

    GetModelObject()->StretchModel( FLOAT3D(
      m_fStretchAll*m_fStretchX*m_fStretchRandom(1),
      m_fStretchAll*m_fStretchY*m_fStretchRandom(2),
      m_fStretchAll*m_fStretchZ*m_fStretchRandom(3)) );
    ModelChangeNotify();
  };

  /* Init model holder*/
  void InitModelHolder(void) {

    // must not crash when model is removed
    if (m_fnModel=="") {
      m_fnModel=CTFILENAME("Models\\Editor\\Axis.mdl");
    }

    if( m_fnReflection == CTString("Models\\Editor\\Vector.tex")) {
      m_fnReflection = CTString("");
    }
    if( m_fnSpecular == CTString("Models\\Editor\\Vector.tex")) {
      m_fnSpecular = CTString("");
    }
    if( m_fnBump == CTString("Models\\Editor\\Vector.tex")) {
      m_fnBump = CTString("");
    }

    if (m_bActive) {
      InitAsModel();
    } else {
      InitAsEditorModel();
    }
    // set appearance
    SetModel(m_fnModel);
    GetModelObject()->PlayAnim(m_iModelAnimation, AOF_LOOPING);

    // if initialized for the first time
    if (m_fnOldModel=="") {
      // just remember the model filename
      m_fnOldModel = m_fnModel;
    // if re-initialized
    } else {
      // if the model filename has changed
      if (m_fnOldModel != m_fnModel) {
        m_fnOldModel = m_fnModel;
        GetModelObject()->AutoSetTextures();
        m_fnTexture = GetModelObject()->mo_toTexture.GetName();
        m_fnReflection = GetModelObject()->mo_toReflection.GetName();
        m_fnSpecular = GetModelObject()->mo_toSpecular.GetName();
        m_fnBump = GetModelObject()->mo_toBump.GetName();
      }
    }
    
    if( m_bAttachments)
    {
      GetModelObject()->AutoSetAttachments();
    }
    else
    {
      GetModelObject()->RemoveAllAttachmentModels();
    }

    try
    {
      GetModelObject()->mo_toTexture.SetData_t(m_fnTexture);
      GetModelObject()->mo_toTexture.PlayAnim(m_iTextureAnimation, AOF_LOOPING);
      GetModelObject()->mo_toReflection.SetData_t(m_fnReflection);
      GetModelObject()->mo_toSpecular.SetData_t(m_fnSpecular);
      GetModelObject()->mo_toBump.SetData_t(m_fnBump);
    } catch (char *strError) {
      WarningMessage(strError);
    }

    // set model stretch
    StretchModel();
    ModelChangeNotify();

    if (m_bColliding) {
      SetPhysicsFlags(EPF_MODEL_FIXED);
      SetCollisionFlags(ECF_MODEL_HOLDER);
    } else {
      SetPhysicsFlags(EPF_MODEL_IMMATERIAL);
      SetCollisionFlags(ECF_IMMATERIAL);
    }

    switch(m_stClusterShadows) {
    case ST_NONE:
      {
        SetFlags(GetFlags()&~ENF_CLUSTERSHADOWS);
        //SetFlags(GetFlags()&~ENF_POLYGONALSHADOWS);
        break;
      }
    case ST_CLUSTER:
      {
        SetFlags(GetFlags()|ENF_CLUSTERSHADOWS);
        //SetFlags(GetFlags()&~ENF_POLYGONALSHADOWS);
        break;
      }
    case ST_POLYGONAL:
      {
        //SetFlags(GetFlags()|ENF_POLYGONALSHADOWS);
        SetFlags(GetFlags()&~ENF_CLUSTERSHADOWS);
        break;
      }
    }

    if (m_bBackground) {
      SetFlags(GetFlags()|ENF_BACKGROUND);
    } else {
      SetFlags(GetFlags()&~ENF_BACKGROUND);
    }

    try {
      m_aoLightAnimation.SetData_t(m_fnmLightAnimation);
    } catch (char *strError) {
      WarningMessage(TRANS("Cannot load '%s': %s"), (const char*)(CTString&)m_fnmLightAnimation,(const char*) strError);
      m_fnmLightAnimation = "";
    }
    if (m_aoLightAnimation.GetData()!=NULL) {
      m_aoLightAnimation.PlayAnim(m_iLightAnimation, AOF_LOOPING);
    }

    if (m_penDestruction==NULL) {
      m_strDescription.PrintF("%s,%s undestroyable", (const char*)m_fnModel.FileName(), (const char*)m_fnTexture.FileName());
    } else {
      m_strDescription.PrintF("%s,%s -> %s", (const char*)m_fnModel.FileName(), (const char*)m_fnTexture.FileName(),
        (const char*)m_penDestruction->GetName());
    }

    return;
  };

procedures:
  Die()
  {
    // for each child of this entity
    {FOREACHINLIST(CEntity, en_lnInParent, en_lhChildren, itenChild) {
      // send it destruction event
      itenChild->SendEvent(ERangeModelDestruction());
    }}

    // spawn debris 
    GetDestruction()->SpawnDebris(this);
    // if there is another phase in destruction
    CModelHolder2 *penNext = GetDestruction()->GetNextPhase();
    if (penNext!=NULL) {
      // copy it here
      CEntity *penNew = GetWorld()->CopyEntityInWorld( *penNext, GetPlacement() );
      penNew->GetModelObject()->StretchModel(GetModelObject()->mo_Stretch);
      penNew->ModelChangeNotify();
    }

/* currently, environment destruction does not yield score.
  update statistics, if score is re-enabled!
    // send score to who killed you
    if (m_penLastDamager!=NULL) {
      EReceiveScore eScore;
      eScore.fPoints = 10.0f;
      m_penLastDamager->SendEvent(eScore);
    }*/

    // if there is a destruction target
    if (m_penDestroyTarget!=NULL) {
      // notify it
      SendToTarget(m_penDestroyTarget, EET_TRIGGER, m_penLastDamager);
    }

    // destroy yourself
    Destroy();
    return;
  }
  Main()
  {
    // initialize the model
    InitModelHolder();

    // check your destruction pointer
    if (m_penDestruction!=NULL && !IsOfClass(m_penDestruction, "ModelDestruction")) {
      WarningMessage("Destruction '%s' is wrong class!",(const char*) m_penDestruction->GetName());
      m_penDestruction=NULL;
    }

    // wait forever
    wait() {
      // on the beginning
      on(EBegin): {
        // set your health
        if (m_penDestruction!=NULL) {
          SetHealth(GetDestruction()->m_fHealth);
        }
        resume;
      }
      // activate/deactivate shows/hides model
      on (EActivate): {
        SwitchToModel();
        m_bActive = TRUE;
        resume;
      }
      on (EDeactivate): {
        SwitchToEditorModel();
        m_bActive = FALSE;
        resume;
      }
      // when your parent is destroyed
      on(ERangeModelDestruction): {
        // destroy yourself
        Destroy();
        resume;
      }
      // when dead
      on(EDeath): {
        if (m_penDestruction!=NULL) {
          jump Die();
        }
        resume;
      }
      // when animation should be changed
      on(EChangeAnim eChange): {
        m_iModelAnimation   = eChange.iModelAnim;
        m_iTextureAnimation = eChange.iTextureAnim;
        m_iLightAnimation   = eChange.iLightAnim;
        if (m_aoLightAnimation.GetData()!=NULL) {
          m_aoLightAnimation.PlayAnim(m_iLightAnimation, eChange.bLightLoop?AOF_LOOPING:0);
        }
        if (GetModelObject()->GetData()!=NULL) {
          GetModelObject()->PlayAnim(m_iModelAnimation, eChange.bModelLoop?AOF_LOOPING:0);
        }
        if (GetModelObject()->mo_toTexture.GetData()!=NULL) {
          GetModelObject()->mo_toTexture.PlayAnim(m_iTextureAnimation, eChange.bTextureLoop?AOF_LOOPING:0);
        }
        resume;
      }
      otherwise(): {
        resume;
      }
    };
  }
};
