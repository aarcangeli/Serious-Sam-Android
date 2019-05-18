set(
  SOURCES
  CompMessage.cpp
  CompModels.cpp
  Computer.cpp
  Console.cpp
  Controls.cpp
  Game.cpp
  LCDDrawing.cpp
  LoadingHook.cpp
  Map.cpp
  SessionProperties.cpp
  StdAfx.cpp
  WEDInterface.cpp
  Camera.cpp
)

add_library(GameMP SHARED ${SOURCES})

target_link_libraries(GameMP EntitiesMP)

target_compile_definitions(GameMP PUBLIC ENGINE_API=)
target_compile_definitions(GameMP PUBLIC PLATFORM_UNIX)
target_compile_definitions(GameMP PUBLIC USE_PORTABLE_C)
