set(
  SOURCES

  bindings.cpp
  seriousSamAndroid.cpp
)

set_property(SOURCE seriousSamAndroid.cpp PROPERTY COMPILE_DEFINITIONS SSA_VERSION="${SSA_VERSION}")

add_library(SeriousSamNatives SHARED ${SOURCES})
target_link_libraries(SeriousSamNatives EGL GLESv2 log)

# attach serious engine
target_link_libraries(SeriousSamNatives Engine)
