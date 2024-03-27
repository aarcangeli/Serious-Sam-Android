206
%{
#include "Entities/StdH/StdH.h"
%}

%{

BOOL ConsiderAll(CEntity*pen) 
{
  return TRUE;
}
BOOL ConsiderPlayers(CEntity*pen) 
{
  return IsDerivedFromClass(pen, "Player");
}
%}

class CTouchField: CRationalEntity {
name      "Touch Field";
thumbnail "Thumbnails\\TouchField.tbn";
features "HasName", "IsTargetable";

properties:
  1 CTString m_strName            "Name" 'N' = "Touch Field",       // class name
  2 CEntityPointer m_penEnter     "Enter Target" 'T' COLOR(C_BROWN|0xFF), // target to send event to
  3 enum EventEType m_eetEnter    "Enter Event" 'E' = EET_TRIGGER,  // event to send on enter
  7 CEntityPointer m_penExit      "Exit Target" COLOR(C_dRED|0xFF), // target to send event to
  8 enum EventEType m_eetExit     "Exit Event" = EET_TRIGGER,      // event to send on exit
  4 BOOL m_bActive                "Active" 'A' = TRUE,              // is field active
  5 BOOL m_bPlayersOnly           "Players only" 'P' = TRUE,        // reacts only on players
  6 FLOAT m_tmExitCheck           "Exit check time" 'X' = 0.0f,     // how often to check for exit
  9 BOOL m_bBlockNonPlayers       "Block non-players" 'B' = FALSE,  // everything except players cannot pass

  100 CEntityPointer m_penLastIn,
{
  CFieldSettings m_fsField;
}

components:
 1 texture TEXTURE_FIELD  "Models\\Editor\\CollisionBox.tex",

functions:
  void SetupFieldSettings(void) {
    m_fsField.fs_toTexture.SetData(GetTextureDataForComponent(TEXTURE_FIELD));
    m_fsField.fs_colColor = C_WHITE|CT_OPAQUE;
  }

  CFieldSettings *GetFieldSettings(void) {
    if (m_fsField.fs_toTexture.GetData()==NULL) {
      SetupFieldSettings();      
    }
    return &m_fsField;
  };

procedures:
  // field is active
  WaitingEntry() {
    m_bActive = TRUE;
    wait() {
      on (EBegin) : { resume; }
      on (EDeactivate) : { jump Frozen(); }
      // when someone passes the polygons
      on (EPass ep) : {
        // if should react only on players and not player
        if (m_bPlayersOnly && !IsDerivedFromClass(ep.penOther, "Player")) {
          // ignore
          resume;
        }
        // send event
        SendToTarget(m_penEnter, m_eetEnter, ep.penOther);
        // if checking for exit
        if (m_tmExitCheck>0) {
          // remember who entered
          m_penLastIn = ep.penOther;
          // wait for exit
          jump WaitingExit();
        }
        resume;
      }
    }
  };

  // waiting for entity to exit
  WaitingExit() {
    while(TRUE) {
      // wait
      wait(m_tmExitCheck) {
        on (EBegin) : { resume; }
        on (EDeactivate) : { jump Frozen(); }
        on (ETimer) : {
          // check for entities inside
          CEntity *penNewIn;
          if (m_bPlayersOnly) {
            penNewIn = TouchingEntity(ConsiderPlayers, m_penLastIn);
          } else {
            penNewIn = TouchingEntity(ConsiderAll, m_penLastIn);
          }
          // if there are no entities in anymore
          if (penNewIn==NULL) {
            // send event
            SendToTarget(m_penExit, m_eetExit, m_penLastIn);
            // wait new entry
            jump WaitingEntry();
          }
          m_penLastIn = penNewIn;
          stop;
        }
      }
    }
  };

  // field is frozen
  Frozen() {
    m_bActive = FALSE;
    wait() {
      on (EBegin) : { resume; }
      on (EActivate) : { jump WaitingEntry(); }
    }
  };

  // main initialization
  Main(EVoid) {
    InitAsFieldBrush();
    SetPhysicsFlags(EPF_BRUSH_FIXED);
    if ( !m_bBlockNonPlayers ) {
      SetCollisionFlags( ((ECBI_MODEL)<<ECB_TEST) | ((ECBI_BRUSH)<<ECB_IS) | ((ECBI_MODEL)<<ECB_PASS) );
    } else {
      SetCollisionFlags( ((ECBI_MODEL|ECBI_PLAYER|ECBI_PROJECTILE_SOLID|ECBI_PROJECTILE_MAGIC)<<ECB_TEST) 
        | ((ECBI_BRUSH)<<ECB_IS) | ((ECBI_PLAYER)<<ECB_PASS) );
    }

    if (m_bActive) {
      jump WaitingEntry();
    } else {
      jump Frozen();
    }

    return;
  };
};