set(
        SOURCES
        src/bitwise.c
        src/framing.c
)
add_library(libogg STATIC ${SOURCES})
target_compile_options(libogg PUBLIC "-I${CMAKE_CURRENT_SOURCE_DIR}/include")
