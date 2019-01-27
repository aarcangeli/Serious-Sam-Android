set(
  ANDROID_ADAPTER_SOURCE

  gles_adapter.cpp
  commons.cpp
  test.cpp
)

add_library(android-adapters STATIC ${ANDROID_ADAPTER_SOURCE})
target_include_directories(android-adapters PRIVATE ${SE_SOURCES})
target_include_directories(android-adapters PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
