set(
  SOURCES

  GUI/Components/MenuGadget.cpp
  GUI/Components/MGArrow.cpp
  GUI/Components/MGButton.cpp
  GUI/Components/MGChangePlayer.cpp
  GUI/Components/MGEdit.cpp
  GUI/Components/MGFileButton.cpp
  GUI/Components/MGHighScore.cpp
  GUI/Components/MGKeyDefinition.cpp
  GUI/Components/MGLevelButton.cpp
  GUI/Components/MGModel.cpp
  GUI/Components/MGServerList.cpp
  GUI/Components/MGSlider.cpp
  GUI/Components/MGTitle.cpp
  GUI/Components/MGTrigger.cpp
  GUI/Components/MGVarButton.cpp

  GUI/Menus/GameMenu.cpp
  GUI/Menus/MAudioOptions.cpp
  GUI/Menus/MConfirm.cpp
  GUI/Menus/MControls.cpp
  GUI/Menus/MCustomizeAxis.cpp
  GUI/Menus/MCustomizeKeyboard.cpp
  GUI/Menus/MDisabled.cpp
  GUI/Menus/Menu.cpp
  GUI/Menus/MenuStartersAF.cpp
  GUI/Menus/MenuManager.cpp
  GUI/Menus/MenuPrinting.cpp
  GUI/Menus/MenuStuff.cpp
  GUI/Menus/MenuActions.cpp
  GUI/Menus/MHighScore.cpp
  GUI/Menus/MInGame.cpp
  GUI/Menus/MLevels.cpp
  GUI/Menus/MLoadSave.cpp
  GUI/Menus/MMain.cpp
  GUI/Menus/MNetwork.cpp
  GUI/Menus/MNetworkJoin.cpp
  GUI/Menus/MNetworkOpen.cpp
  GUI/Menus/MNetworkStart.cpp
  GUI/Menus/MOptions.cpp
  GUI/Menus/MPlayerProfile.cpp
  GUI/Menus/MSelectPlayers.cpp
  GUI/Menus/MServers.cpp
  GUI/Menus/MSinglePlayer.cpp
  GUI/Menus/MSinglePlayerNew.cpp
  GUI/Menus/MSplitScreen.cpp
  GUI/Menus/MSplitStart.cpp
  GUI/Menus/MVar.cpp
  GUI/Menus/MVideoOptions.cpp
  GUI/Menus/MenuStarters.cpp

  Credits.cpp
  CmdLine.cpp
  LCDDrawing.cpp
  LevelInfo.cpp
  GLSettings.cpp
  SeriousSam.cpp
  VarList.cpp
)
if (STATIC_LINKING)
  add_library(SeriousSam STATIC ${SOURCES})
else ()
  add_library(SeriousSam SHARED ${SOURCES})
endif ()
target_compile_options(SeriousSam PRIVATE "-I${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(SeriousSam PRIVATE "-I${CMAKE_CURRENT_SOURCE_DIR}/GUI/Menus")
target_link_libraries(SeriousSam Engine)
add_dependencies(SeriousSam EntitiesMP)
