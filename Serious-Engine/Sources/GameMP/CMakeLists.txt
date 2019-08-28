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

if (STATIC_LINKING)
  add_library(GameMP STATIC ${SOURCES})
else ()
  add_library(GameMP SHARED ${SOURCES})
endif ()

target_link_libraries(GameMP EntitiesMP)

target_compile_definitions(GameMP PUBLIC ENGINE_API=)
target_compile_definitions(GameMP PUBLIC PLATFORM_UNIX)
target_compile_definitions(GameMP PUBLIC USE_PORTABLE_C)
