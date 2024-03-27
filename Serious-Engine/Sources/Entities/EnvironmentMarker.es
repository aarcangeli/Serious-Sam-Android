901
%{
#include "Entities/StdH/StdH.h"
%}

uses "Entities/Marker";

class CEnvironmentMarker: CMarker {
name      "Environment Marker";
thumbnail "Thumbnails\\EnvironmentMarker.tbn";

properties:
  1 FLOAT m_fWaitTime       "Wait time" 'W' = 0.0f,           // time to wait (or do anything until go to another marker)
  2 FLOAT m_fRandomTime     "Random time" 'E' = 0.0f,         // random time to wait (or do anything until go to another marker)
  3 RANGE m_fMarkerRange    "Marker range" 'R' = 5.0f,        // range around marker (marker doesn't have to be hit directly)
  4 BOOL m_bFixedAnimLength "Fixed anim length" 'F' = FALSE,  // fixed anim length (like play once)
  5 BOOL m_bChangeDefaultAnim "Change default anim" 'C' = FALSE,  // change default anim
  6 FLOAT m_fMoveSpeed      "Move speed" 'V' = -1.0f,         // moving speed
  7 FLOAT m_fRotateSpeed    "Rotate speed" 'B' = -1.0f,       // rotate speed

 // for browsing animations on marker for base environment model
 20 CTFileName m_fnMdl           "Model" 'M' = CTFILENAME("Models\\Editor\\Axis.mdl"),
 21 ANIMATION m_iAnim            "Animation" 'A' =0,
 22 CModelObject m_moAnimData,

components:
  1 model   MODEL_MARKER     "Models\\Editor\\EnvironmentMarker.mdl",
  2 texture TEXTURE_MARKER   "Models\\Editor\\EnvironmentMarker.tex"

functions:
  /* Check if entity is moved on a route set up by its targets. */
  BOOL MovesByTargetedRoute(CTString &strTargetProperty) const {
    strTargetProperty = "Target";
    return TRUE;
  };
  /* Check if entity can drop marker for making linked route. */
  BOOL DropsMarker(CTFileName &fnmMarkerClass, CTString &strTargetProperty) const {
    fnmMarkerClass = CTFILENAME("Classes\\EnvironmentMarker.ecl");
    strTargetProperty = "Target";
    return TRUE;
  };
  /* Get anim data for given animation property - return NULL for none. */
  CAnimData *GetAnimData(SLONG slPropertyOffset) {
    if(slPropertyOffset==offsetof(CEnvironmentMarker, m_iAnim)) {
      return m_moAnimData.GetData();

    } else {
      return CEntity::GetAnimData(slPropertyOffset);
    }
  };

  // >>> original source from CEntity::SetModel <<<
  // set mdoel object
  void SetModelObject(void) {
    // try to
    try {
      // load the new model data
      m_moAnimData.SetData_t(m_fnMdl);
    // if failed
    } catch(char *strError) {
      strError;
      DECLARE_CTFILENAME(fnmDefault, "Models\\Editor\\Axis.mdl");
      // try to
      try {
        // load the default model data
        m_moAnimData.SetData_t(fnmDefault);
      // if failed
      } catch(char *strErrorDefault) {
        FatalError(TRANS("Cannot load default model '%s':\n%s"),
          (const char*)(CTString&)fnmDefault, strErrorDefault);
      }
    }
  };


procedures:
  Main() {
    InitAsEditorModel();
    SetPhysicsFlags(EPF_MODEL_IMMATERIAL);
    SetCollisionFlags(ECF_IMMATERIAL);

    // set appearance
    SetModel(MODEL_MARKER);
    SetModelMainTexture(TEXTURE_MARKER);

    // set model data for anim browser
    SetModelObject();

    return;
  }
};

