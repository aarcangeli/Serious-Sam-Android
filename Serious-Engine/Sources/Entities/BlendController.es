612
%{
#include "Entities/StdH/StdH.h"
#include "Entities/WorldSettingsController.h"
#include "Entities/BackgroundViewer.h"
%}

uses "Entities/Marker";

enum BlendControllerType {
  0 BCT_NONE                        "None",                      // no FX
  1 BCT_PYRAMID_PLATES              "Appear pyramid plates",     // effect of appearing of pyramid plates
  2 BCT_ACTIVATE_PLATE_1            "Activate plate 1",          // plate 1 activating
  3 BCT_ACTIVATE_PLATE_2            "Activate plate 2",          // plate 2 activating
  4 BCT_ACTIVATE_PLATE_3            "Activate plate 3",          // plate 3 activating
  5 BCT_ACTIVATE_PLATE_4            "Activate plate 4",          // plate 4 activating
  6 BCT_ACTIVATE_PYRAMID_MORPH_ROOM "Pyramid morph room",        // pyramid morph room activated
};

class CBlendController: CMarker
{
name      "Blend controller";
thumbnail "Thumbnails\\BlendController.tbn";
features "IsImportant";

properties:

  1 enum BlendControllerType m_bctType  "Blend type" 'Y' = BCT_NONE,         // type of effect

components:

  1 model   MODEL_CONTROLLER          "Models\\Editor\\BlendController.mdl",
  2 texture TEXTURE_CONTROLLER        "Models\\Editor\\BlendController.tex",


functions:
  // ----------- Obtain world settings controller
  CWorldSettingsController *GetWSC(void)
  {
    CWorldSettingsController *pwsc = NULL;
    // obtain bcg viewer
    CBackgroundViewer *penBcgViewer = (CBackgroundViewer *) GetWorld()->GetBackgroundViewer();
    if( penBcgViewer != NULL)
    {
      // obtain world settings controller 
      pwsc = (CWorldSettingsController *) &*penBcgViewer->m_penWorldSettingsController;
    }
    return pwsc;
  }
  
  /* Handle an event, return false if the event is not handled. */
  BOOL HandleEvent(const CEntityEvent &ee)
  {
    // obtain world settings controller
    CWorldSettingsController *pwsc = GetWSC();
    if( pwsc == NULL) {
      return FALSE;
    }
    FLOAT tmNow = _pTimer->CurrentTick();

    if (ee.ee_slEvent==EVENTCODE_EActivate)
    {
      switch(m_bctType)
      {
        case BCT_PYRAMID_PLATES:
          pwsc->m_tmPyramidPlatesStart = tmNow;
          break;
        case BCT_ACTIVATE_PLATE_1:
          pwsc->m_tmActivatedPlate1 = tmNow;
          pwsc->m_tmDeactivatedPlate1 = 1e6;
          break;
        case BCT_ACTIVATE_PLATE_2:
          pwsc->m_tmActivatedPlate2 = tmNow;
          pwsc->m_tmDeactivatedPlate2 = 1e6;
          break;
        case BCT_ACTIVATE_PLATE_3:
          pwsc->m_tmActivatedPlate3 = tmNow;
          pwsc->m_tmDeactivatedPlate3 = 1e6;
          break;
        case BCT_ACTIVATE_PLATE_4:
          pwsc->m_tmActivatedPlate4 = tmNow;
          pwsc->m_tmDeactivatedPlate4 = 1e6;
          break;
        case BCT_ACTIVATE_PYRAMID_MORPH_ROOM:
          pwsc->m_tmPyramidMorphRoomActivated = tmNow;
          break;
      }
    }
    else if (ee.ee_slEvent==EVENTCODE_EDeactivate)
    {
      switch(m_bctType)
      {
        case BCT_ACTIVATE_PLATE_1:
          pwsc->m_tmDeactivatedPlate1 = tmNow;
          break;
        case BCT_ACTIVATE_PLATE_2:
          pwsc->m_tmDeactivatedPlate2 = tmNow;
          break;
        case BCT_ACTIVATE_PLATE_3:
          pwsc->m_tmDeactivatedPlate3 = tmNow;
          break;
        case BCT_ACTIVATE_PLATE_4:
          pwsc->m_tmDeactivatedPlate4 = tmNow;
          break;
      }
    }
    return FALSE;
  }

procedures:

  Main()
  {
    // init model
    InitAsEditorModel();
    SetPhysicsFlags(EPF_MODEL_IMMATERIAL);
    SetCollisionFlags(ECF_IMMATERIAL);

    // set appearance
    SetModel(MODEL_CONTROLLER);
    SetModelMainTexture(TEXTURE_CONTROLLER);

    return;
  }
};

