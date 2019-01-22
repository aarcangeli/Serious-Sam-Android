set(
        SOURCES
        src/bitwise.c
        src/framing.c
)
add_library(libogg STATIC ${SOURCES})
target_include_directories(libogg PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
