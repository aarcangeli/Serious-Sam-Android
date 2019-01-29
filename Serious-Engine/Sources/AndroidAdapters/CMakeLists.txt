include(glm)
include_directories(${GLM_INCLUDE_DIR})

set(
  ANDROID_ADAPTER_SOURCE

  gles_adapter.cpp
  commons.cpp
  gles_glMessageCallback.cpp
)

add_library(android-adapters STATIC ${ANDROID_ADAPTER_SOURCE})
target_include_directories(android-adapters PRIVATE ${SE_SOURCES})
target_include_directories(android-adapters PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
