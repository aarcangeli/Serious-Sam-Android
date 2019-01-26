set(
  SOURCES

  bindings.cpp
  glmocks.cpp
  SeriousSam.cpp
)

add_library(SeriousSamNatives SHARED ${SOURCES})
target_link_libraries(SeriousSamNatives GLESv2 log)

# attach serious engine
target_link_libraries(SeriousSamNatives Engine)
