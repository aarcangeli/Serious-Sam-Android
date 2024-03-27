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

343
%{
#include "EntitiesMP/StdH/StdH.h"
#include "ModelsMP/Enemies/Grunt/Grunt.h"
%}

uses "EntitiesMP/EnemyBase";
uses "EntitiesMP/BasicEffects";

enum GruntType {
  0 GT_SOLDIER    "Grunt soldier",
  1 GT_COMMANDER  "Grunt commander",
};

%{
#define STRETCH_SOLDIER   1.2f
#define STRETCH_COMMANDER 1.4f
  
// info structure
static EntityInfo eiGruntSoldier = {
  EIBT_FLESH, 200.0f,
  0.0f, 1.9f*STRETCH_SOLDIER, 0.0f,     // source (eyes)
  0.0f, 1.3f*STRETCH_SOLDIER, 0.0f,     // target (body)
};

static EntityInfo eiGruntCommander = {
  EIBT_FLESH, 250.0f,
  0.0f, 1.9f*STRETCH_COMMANDER, 0.0f,     // source (eyes)
  0.0f, 1.3f*STRETCH_COMMANDER, 0.0f,     // target (body)
};

#define FIREPOS_SOLDIER      FLOAT3D(0.07f, 1.36f, -0.78f)*STRETCH_SOLDIER
#define FIREPOS_COMMANDER_UP  FLOAT3D(0.09f, 1.45f, -0.62f)*STRETCH_COMMANDER
#define FIREPOS_COMMANDER_DN  FLOAT3D(0.10f, 1.30f, -0.60f)*STRETCH_COMMANDER
%}


class CGrunt: CEnemyBase {
name      "Grunt";
thumbnail "Thumbnails\\Grunt.tbn";

properties:
  1 enum GruntType m_gtType "Type" 'Y' = GT_SOLDIER,

  10 CSoundObject m_soFire1,
  11 CSoundObject m_soFire2,

// class internal
    
components:
  1 class   CLASS_BASE            "Classes\\EnemyBase.ecl",
  3 class   CLASS_PROJECTILE      "Classes\\Projectile.ecl",

 10 model   MODEL_GRUNT           "ModelsMP\\Enemies\\Grunt\\Grunt.mdl",
 11 model   MODEL_GUN_COMMANDER   "ModelsMP\\Enemies\\Grunt\\Gun_Commander.mdl",
 12 model   MODEL_GUN_SOLDIER     "ModelsMP\\Enemies\\Grunt\\Gun.mdl",
 
 20 texture TEXTURE_SOLDIER       "ModelsMP\\Enemies\\Grunt\\Soldier.tex",
 21 texture TEXTURE_COMMANDER     "ModelsMP\\Enemies\\Grunt\\Commander.tex",
 22 texture TEXTURE_GUN_COMMANDER "ModelsMP\\Enemies\\Grunt\\Gun_Commander.tex",
 23 texture TEXTURE_GUN_SOLDIER   "ModelsMP\\Enemies\\Grunt\\Gun.tex",
 
// ************** SOUNDS **************
 50 sound   SOUND_IDLE            "ModelsMP\\Enemies\\Grunt\\Sounds\\Idle.wav",
 52 sound   SOUND_SIGHT           "ModelsMP\\Enemies\\Grunt\\Sounds\\Sight.wav",
 53 sound   SOUND_WOUND           "ModelsMP\\Enemies\\Grunt\\Sounds\\Wound.wav",
 57 sound   SOUND_FIRE            "ModelsMP\\Enemies\\Grunt\\Sounds\\Fire.wav",
 58 sound   SOUND_DEATH           "ModelsMP\\Enemies\\Grunt\\Sounds\\Death.wav",

functions:
    
  // describe how this enemy killed player
  virtual CTString GetPlayerKillDescription(const CTString &strPlayerName, const EDeath &eDeath)
  {
    CTString str;
    str.PrintF(TRANSV("A Grunt sent %s into the halls of Valhalla"), (const char *) strPlayerName);
    return str;
  }
  
  // --------------------------------------------------------------------------------------
  // [SSE]
  // Returns amount of damage which should be received for performing wound state.
  // --------------------------------------------------------------------------------------
  virtual FLOAT GetDamageWounded(void)
  { 
    return 5.0F;
  }
  
  // --------------------------------------------------------------------------------------
  // [SSE]
  // Returns TRUE when enemy can be wound, otherwise returns FALSE.
  // --------------------------------------------------------------------------------------
  virtual BOOL CanBeWounded(void)
  {
    return GetHealth() < (m_fMaxHealth / 2);
  }

  /* Entity info */
  void *GetEntityInfo(void) {
    if (m_gtType==GT_SOLDIER) {
      return &eiGruntSoldier;
    } else if (m_gtType==GT_COMMANDER) {
      return &eiGruntSoldier;
    } else {
      ASSERTALWAYS("Unknown grunt type!");
      return NULL;
    }
  };

  virtual const CTFileName &GetComputerMessageName(void) const {
    static DECLARE_CTFILENAME(fnmSoldier,     "DataMP\\Messages\\Enemies\\GruntSoldier.txt");
    static DECLARE_CTFILENAME(fnmCommander,   "DataMP\\Messages\\Enemies\\GruntCommander.txt");
    switch(m_gtType) {
    default: ASSERT(FALSE);
    case GT_SOLDIER:  return fnmSoldier;
    case GT_COMMANDER: return fnmCommander;
    }
  };

  void Precache(void) {
    CEnemyBase::Precache();
    
   if (m_gtType==GT_SOLDIER) {
      PrecacheClass(CLASS_PROJECTILE, PRT_GRUNT_PROJECTILE_SOL);
    }
    if (m_gtType==GT_COMMANDER) {
      PrecacheClass(CLASS_PROJECTILE, PRT_GRUNT_PROJECTILE_COM);
    }

    PrecacheSound(SOUND_IDLE);
    PrecacheSound(SOUND_SIGHT);
    PrecacheSound(SOUND_WOUND);
    PrecacheSound(SOUND_FIRE);
    PrecacheSound(SOUND_DEATH);
  };

  /* Receive damage */
  void ReceiveDamage(CEntity *penInflictor, enum DamageType dmtType,
    FLOAT fDamageAmmount, const FLOAT3D &vHitPoint, const FLOAT3D &vDirection) 
  {
    CEnemyBase::ReceiveDamage(penInflictor, dmtType, fDamageAmmount, vHitPoint, vDirection);
  };

  // damage anim
  INDEX AnimForDamage(FLOAT fDamage) {
    INDEX iAnim;
    iAnim = GRUNT_ANIM_WOUND01;
    StartModelAnim(iAnim, 0);
    return iAnim;
  };

  // death
  INDEX AnimForDeath(void) {
    INDEX iAnim;
    FLOAT3D vFront;
    GetHeadingDirection(0, vFront);
    FLOAT fDamageDir = m_vDamage%vFront;
    if (fDamageDir<0) {
      iAnim = GRUNT_ANIM_DEATHBACKWARD;
    } else {
      iAnim = GRUNT_ANIM_DEATHFORWARD;
    }

    StartModelAnim(iAnim, 0);
    return iAnim;
  };

  FLOAT WaitForDust(FLOAT3D &vStretch) {
    vStretch=FLOAT3D(1,1,2);
    if(GetModelObject()->GetAnim()==GRUNT_ANIM_DEATHBACKWARD)
    {
      return 0.5f;
    }
    else if(GetModelObject()->GetAnim()==GRUNT_ANIM_DEATHFORWARD)
    {
      return 1.0f;
    }
    return -1.0f;
  };

  void DeathNotify(void) {
    ChangeCollisionBoxIndexWhenPossible(GRUNT_COLLISION_BOX_DEATH);
    en_fDensity = 500.0f;
  };

  // virtual anim functions
  void StandingAnim(void) {
    StartModelAnim(GRUNT_ANIM_IDLE, AOF_LOOPING|AOF_NORESTART);
  };
  /*void StandingAnimFight(void)
  {
    StartModelAnim(HEADMAN_ANIM_IDLE_FIGHT, AOF_LOOPING|AOF_NORESTART);
  }*/
  void RunningAnim(void) {
    StartModelAnim(GRUNT_ANIM_RUN, AOF_LOOPING|AOF_NORESTART);
  };
    void WalkingAnim(void) {
    RunningAnim();
  };
  void RotatingAnim(void) {
    RunningAnim();
  };

  // virtual sound functions
  void IdleSound(void) {
    PlaySound(m_soSound, SOUND_IDLE, SOF_3D);
  };
  void SightSound(void) {
    PlaySound(m_soSound, SOUND_SIGHT, SOF_3D);
  };
  void WoundSound(void) {
    PlaySound(m_soSound, SOUND_WOUND, SOF_3D);
  };
  void DeathSound(void) {
    PlaySound(m_soSound, SOUND_DEATH, SOF_3D);
  };

  // adjust sound and watcher parameters here if needed
  void EnemyPostInit(void) 
  {
    // set sound default parameters
    m_soFire1.Set3DParameters(160.0f, 50.0f, 1.0f, 1.0f);
    m_soFire2.Set3DParameters(160.0f, 50.0f, 1.0f, 1.0f);
  };

procedures:
/************************************************************
 *                A T T A C K   E N E M Y                   *
 ************************************************************/
  Fire(EVoid) : CEnemyBase::Fire {
    // soldier
    if (m_gtType == GT_SOLDIER) {
      autocall SoldierAttack() EEnd;
    // commander
    } else if (m_gtType == GT_COMMANDER) {
      autocall CommanderAttack() EEnd;
    // should never get here
    } else{
      ASSERT(FALSE);
    }
    return EReturn();
  };
    
  // Soldier attack
  SoldierAttack(EVoid) {
    StandingAnimFight();
    autowait(0.2f + FRnd()*0.25f);

    StartModelAnim(GRUNT_ANIM_FIRE, 0);
    ShootProjectile(PRT_GRUNT_PROJECTILE_SOL, FIREPOS_SOLDIER * ClampDn(m_fStretchMultiplier, 0.2F), ANGLE3D(0, 0, 0)); // [SSE] Better Enemy Stretching
    
    PlaySound(m_soFire1, SOUND_FIRE, SOF_3D);

    autowait(0.15f + FRnd()*0.1f);
    StartModelAnim(GRUNT_ANIM_FIRE, 0);
    ShootProjectile(PRT_GRUNT_PROJECTILE_SOL, FIREPOS_SOLDIER * ClampDn(m_fStretchMultiplier, 0.2F), ANGLE3D(0, 0, 0)); // [SSE] Better Enemy Stretching
    
     PlaySound(m_soFire2, SOUND_FIRE, SOF_3D);

    autowait(FRnd()*0.333f);
    return EEnd();
  };

  // Commander attack (predicted firing on moving player)
  CommanderAttack(EVoid) {
    StandingAnimFight();
    autowait(0.2f + FRnd()*0.25f);

    /*FLOAT3D vGunPosAbs   = GetPlacement().pl_PositionVector + FLOAT3D(0.0f, 1.0f, 0.0f)*GetRotationMatrix();
    FLOAT3D vEnemySpeed  = ((CMovableEntity&) *m_penEnemy).en_vCurrentTranslationAbsolute;
    FLOAT3D vEnemyPos    = ((CMovableEntity&) *m_penEnemy).GetPlacement().pl_PositionVector;
    FLOAT   fLaserSpeed  = 45.0f; // m/s
    FLOAT3D vPredictedEnemyPosition = CalculatePredictedPosition(vGunPosAbs,
      vEnemyPos, fLaserSpeed, vEnemySpeed, GetPlacement().pl_PositionVector(2) );
    ShootPredictedProjectile(PRT_GRUNT_LASER, vPredictedEnemyPosition, FLOAT3D(0.0f, 1.0f, 0.0f), ANGLE3D(0, 0, 0));*/

    StartModelAnim(GRUNT_ANIM_FIRE, 0);
    ShootProjectile(PRT_GRUNT_PROJECTILE_COM, FIREPOS_COMMANDER_DN * ClampDn(m_fStretchMultiplier, 0.2F), ANGLE3D(-20, 0, 0)); // [SSE] Better Enemy Stretching

    PlaySound(m_soFire1, SOUND_FIRE, SOF_3D);

    autowait(0.035f);
    StartModelAnim(GRUNT_ANIM_FIRE, 0);
    ShootProjectile(PRT_GRUNT_PROJECTILE_COM, FIREPOS_COMMANDER_DN * ClampDn(m_fStretchMultiplier, 0.2F), ANGLE3D(-10, 0, 0)); // [SSE] Better Enemy Stretching
    
    PlaySound(m_soFire2, SOUND_FIRE, SOF_3D);

    autowait(0.035f);
    StartModelAnim(GRUNT_ANIM_FIRE, 0);
    ShootProjectile(PRT_GRUNT_PROJECTILE_COM, FIREPOS_COMMANDER_DN * ClampDn(m_fStretchMultiplier, 0.2F), ANGLE3D(0, 0, 0)); // [SSE] Better Enemy Stretching

    PlaySound(m_soFire1, SOUND_FIRE, SOF_3D);

    autowait(0.035f);
    StartModelAnim(GRUNT_ANIM_FIRE, 0);
    ShootProjectile(PRT_GRUNT_PROJECTILE_COM, FIREPOS_COMMANDER_DN * ClampDn(m_fStretchMultiplier, 0.2F), ANGLE3D(10, 0, 0)); // [SSE] Better Enemy Stretching
    
    PlaySound(m_soFire2, SOUND_FIRE, SOF_3D);
    

    autowait(0.035f);
    StartModelAnim(GRUNT_ANIM_FIRE, 0);
    ShootProjectile(PRT_GRUNT_PROJECTILE_COM, FIREPOS_COMMANDER_DN * ClampDn(m_fStretchMultiplier, 0.2F), ANGLE3D(20, 0, 0)); // [SSE] Better Enemy Stretching
    
    PlaySound(m_soFire2, SOUND_FIRE, SOF_3D);

    autowait(FRnd()*0.5f);
    return EEnd();
  };

/************************************************************
 *                       M  A  I  N                         *
 ************************************************************/
  Main(EVoid) {
    // declare yourself as a model
    InitAsModel();
    SetPhysicsFlags(EPF_MODEL_WALKING|EPF_HASLUNGS);
    SetCollisionFlags(ECF_MODEL);
    SetFlags(GetFlags()|ENF_ALIVE);
    en_tmMaxHoldBreath = 5.0f;
    en_fDensity = 2000.0f;
    //m_fBlowUpSize = 2.0f;

    // set your appearance
    SetModel(MODEL_GRUNT);
    switch (m_gtType) {
      case GT_SOLDIER:
        // set your texture
        SetModelMainTexture(TEXTURE_SOLDIER);
        AddAttachment(GRUNT_ATTACHMENT_GUN_SMALL, MODEL_GUN_SOLDIER, TEXTURE_GUN_SOLDIER);
        // setup moving speed
        m_fWalkSpeed = FRnd() + 2.5f;
        m_aWalkRotateSpeed = AngleDeg(FRnd()*10.0f + 500.0f);
        m_fAttackRunSpeed = FRnd() + 6.5f;
        m_aAttackRotateSpeed = AngleDeg(FRnd()*50 + 245.0f);
        m_fCloseRunSpeed = FRnd() + 6.5f;
        m_aCloseRotateSpeed = AngleDeg(FRnd()*50 + 245.0f);
        // setup attack distances
        m_fAttackDistance = 80.0f;
        m_fCloseDistance = 0.0f;
        m_fStopDistance = 8.0f;
        m_fAttackFireTime = 2.0f;
        m_fCloseFireTime = 1.0f;
        m_fIgnoreRange = 200.0f;
        //m_fBlowUpAmount = 65.0f;
        m_fBlowUpAmount = 80.0f;
        m_fBodyParts = 4;
        m_fDamageWounded = 5.0f; // [SSE] Balance Change. Default=0.0F.
        m_iScore = 500;
        SetHealth(40.0f);
        m_fMaxHealth = 40.0f;
        // set stretch factors for height and width
        GetModelObject()->StretchModel(FLOAT3D(STRETCH_SOLDIER, STRETCH_SOLDIER, STRETCH_SOLDIER));
        break;
  
      case GT_COMMANDER:
        // set your texture
        SetModelMainTexture(TEXTURE_COMMANDER);
        AddAttachment(GRUNT_ATTACHMENT_GUN_COMMANDER, MODEL_GUN_COMMANDER, TEXTURE_GUN_COMMANDER);
        // setup moving speed
        m_fWalkSpeed = FRnd() + 2.5f;
        m_aWalkRotateSpeed = AngleDeg(FRnd()*10.0f + 500.0f);
        m_fAttackRunSpeed = FRnd() + 8.0f;
        m_aAttackRotateSpeed = AngleDeg(FRnd()*50 + 245.0f);
        m_fCloseRunSpeed = FRnd() + 8.0f;
        m_aCloseRotateSpeed = AngleDeg(FRnd()*50 + 245.0f);
        // setup attack distances
        m_fAttackDistance = 90.0f;
        m_fCloseDistance = 0.0f;
        m_fStopDistance = 15.0f;
        m_fAttackFireTime = 4.0f;
        m_fCloseFireTime = 2.0f;
        m_fIgnoreRange = 200.0f;

        // damage/explode properties
        //m_fBlowUpAmount = 180.0f;
        m_fBodyParts = 5;
        m_fDamageWounded = 5.0f; // [SSE] Balance Change. Default=0.0F.
        m_iScore = 800;
        SetHealth(60.0f);
        m_fMaxHealth = 60.0f;
        // set stretch factors for height and width
        GetModelObject()->StretchModel(FLOAT3D(STRETCH_COMMANDER, STRETCH_COMMANDER, STRETCH_COMMANDER));
        break;
    }

    ModelChangeNotify();
    StandingAnim();

    // continue behavior in base class
    jump CEnemyBase::MainLoop();
  };
};
