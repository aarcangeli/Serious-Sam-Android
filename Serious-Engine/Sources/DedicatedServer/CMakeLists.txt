set(SE_DS_BASE "${SE_SOURCES}/DedicatedServerAndroid")

set(
  SE_DS_SOURCES
  DedicatedServerAndroid.cpp
)

add_executable(DedicatedServerAndroid ${SE_DS_SOURCES})

target_include_directories(DedicatedServerAndroid PRIVATE
  "${SE_SOURCES}"
  "${SE_SOURCES}/DedicatedServer"
)

target_compile_definitions(DedicatedServerAndroid PRIVATE
  ENGINE_API=
  PLATFORM_UNIX
  USE_PORTABLE_C
)

target_link_libraries(DedicatedServerAndroid PRIVATE
  Engine
)

if (STATIC_LINKING)
  if (FIRST_ENCOUNTER)
    target_link_libraries(DedicatedServerAndroid PRIVATE
      Entities
      GameMP
    )
  else ()
    target_link_libraries(DedicatedServerAndroid PRIVATE
      EntitiesMP
      GameMP
    )
  endif ()
endif ()

if (ANDROID)
  set_target_properties(DedicatedServerAndroid PROPERTIES OUTPUT_NAME "DedicatedServerAndroid")
else ()
  set_target_properties(DedicatedServerAndroid PROPERTIES OUTPUT_NAME "DedicatedServer")
endif ()
