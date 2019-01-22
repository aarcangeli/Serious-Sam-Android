set(
        ANDROID_ADAPTER_SOURCE

        commons.cpp
        test.cpp
)

add_library(android-adapter ${ANDROID_ADAPTER_SOURCE})
target_link_libraries(android-adapter se-engine)
target_include_directories(android-adapter PRIVATE ${SE_SOURCES})
target_include_directories(android-adapter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
