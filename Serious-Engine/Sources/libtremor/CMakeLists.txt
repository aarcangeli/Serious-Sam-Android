set(
        SOURCES
		block.c
		codebook.c
		floor0.c
		floor1.c
		info.c
		mapping0.c
		mdct.c
		registry.c
		res012.c
		sharedbook.c
		synthesis.c
		vorbisfile.c
		window.c
)
if (STATIC_LINKING)
  add_library(vorbis STATIC ${SOURCES})
else ()
  add_library(vorbis SHARED ${SOURCES})
endif ()
target_link_libraries(vorbis libogg)
target_compile_options(vorbis PUBLIC "-I${CMAKE_CURRENT_SOURCE_DIR}/include")
