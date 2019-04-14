set(
  SOURCES

  bindings.cpp
)

add_library(SeriousSamNatives SHARED ${SOURCES})
target_link_libraries(SeriousSamNatives EGL GLESv2 log)

# attach serious engine
target_link_libraries(SeriousSamNatives Engine)
target_link_libraries(SeriousSamNatives SeriousSam)
