set(
  SOURCES

  DedicatedServer.cpp
)

add_executable(DedicatedServer ${SOURCES})

# attach serious engine
target_link_libraries(DedicatedServer Engine)
target_link_libraries(DedicatedServer EntitiesMP)
target_link_libraries(DedicatedServer GameMP)
add_dependencies(DedicatedServer EntitiesMP)
