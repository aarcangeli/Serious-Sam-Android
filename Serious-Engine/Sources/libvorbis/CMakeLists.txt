set(
        SOURCES
        lib/analysis.c
        lib/bitrate.c
        lib/block.c
        lib/codebook.c
        lib/envelope.c
        lib/floor0.c
        lib/floor1.c
        lib/info.c
        lib/lookup.c
        lib/lpc.c
        lib/lsp.c
        lib/mapping0.c
        lib/mdct.c
        lib/psy.c
        lib/registry.c
        lib/res0.c
        lib/sharedbook.c
        lib/smallft.c
        lib/synthesis.c
        lib/vorbisenc.c
        lib/vorbisfile.c
        lib/window.c
)
if (STATIC_LINKING)
  add_library(vorbis STATIC ${SOURCES})
else ()
  add_library(vorbis SHARED ${SOURCES})
endif ()
target_link_libraries(vorbis libogg)
target_compile_options(vorbis PUBLIC "-I${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_options(vorbis PRIVATE "-I${CMAKE_CURRENT_SOURCE_DIR}/lib")
