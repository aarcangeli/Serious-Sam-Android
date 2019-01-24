set(
  ANDROID_ADAPTER_SOURCE

  commons.cpp
  test.cpp
  SeriousSam.cpp
)

add_library(android-adapters STATIC ${ANDROID_ADAPTER_SOURCE})
target_link_libraries(android-adapters se-engine)
target_include_directories(android-adapters PRIVATE ${SE_SOURCES})
target_include_directories(android-adapters PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
