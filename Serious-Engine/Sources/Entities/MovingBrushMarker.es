102
%{
#include "Entities/StdH/StdH.h"
%}

uses "Entities/Marker";

class CMovingBrushMarker: CMarker {
name      "Moving Brush Marker";
thumbnail "Thumbnails\\MovingBrushMarker.tbn";

properties:
  1 BOOL m_bInverseRotate                 "Inverse Rotate" 'R' = FALSE,
  2 FLOAT m_fSpeed                        "Speed" 'S' = -1.0f,
  3 FLOAT m_fWaitTime                     "Wait time" 'W' = -1.0f,
  4 BOOL m_bStopMoving                    "Stop moving" 'O' = FALSE,
  6 enum BoolEType m_betMoveOnTouch       "Move on touch" 'M' = BET_IGNORE,
  7 FLOAT m_fBlockDamage                  "Block damage" 'D' = -1.0f,

 // send event on marker
 10 enum EventEType m_eetMarkerEvent  "Marker Event - Type" 'J' = EET_IGNORE,  // type of event to send
 11 CEntityPointer m_penMarkerEvent   "Marker Event - Target" 'K',             // target to send event to

 // send event on touch
 16 enum EventEType m_eetTouchEvent     "Touch Event - Type" 'U' = EET_IGNORE,  // type of event to send
 17 CEntityPointer m_penTouchEvent      "Touch Event - Target" 'I',             // target to send event to

 // sound target
 20 CEntityPointer m_penSoundStart    "Sound start entity" 'Q',   // sound start entity
 21 CEntityPointer m_penSoundStop     "Sound stop entity" 'Z',    // sound stop entity
 22 CEntityPointer m_penSoundFollow   "Sound follow entity" 'F',  // sound follow entity

components:
  1 model   MODEL_MARKER     "Models\\Editor\\MovingBrushMarker.mdl",
  2 texture TEXTURE_MARKER   "Models\\Editor\\GravityMarker.tex"

functions:
  /* Check if entity can drop marker for making linked route. */
  BOOL DropsMarker(CTFileName &fnmMarkerClass, CTString &strTargetProperty) const {
    fnmMarkerClass = CTFILENAME("Classes\\MovingBrushMarker.ecl");
    strTargetProperty = "Target";
    return TRUE;
  }
procedures:
  Main()
  {
    InitAsEditorModel();
    SetPhysicsFlags(EPF_MODEL_IMMATERIAL);
    SetCollisionFlags(ECF_IMMATERIAL);

    // set appearance
    SetModel(MODEL_MARKER);
    SetModelMainTexture(TEXTURE_MARKER);

    return;
  }
};

