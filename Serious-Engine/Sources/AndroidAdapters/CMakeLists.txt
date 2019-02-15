include(glm)

set(
  ANDROID_ADAPTER_SOURCE

  gles_adapter.cpp
  gles_adapter_unused.cpp
  commons.cpp
  gles_glMessageCallback.cpp
)

add_library(android-adapters STATIC ${ANDROID_ADAPTER_SOURCE})
target_compile_options(android-adapters PRIVATE "-I${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(android-adapters PUBLIC "-I${GLM_INCLUDE_DIR}")
