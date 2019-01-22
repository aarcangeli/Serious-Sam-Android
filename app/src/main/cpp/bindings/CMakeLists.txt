add_library(SeriousSamNatives SHARED bindings.cpp)
target_link_libraries(SeriousSamNatives GLESv2 log)

# attach serious engine
target_link_libraries(SeriousSamNatives se-engine)
