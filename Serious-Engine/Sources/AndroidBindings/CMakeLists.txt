set(
  SOURCES

  bindings.cpp
)

add_library(SeriousSamNatives SHARED ${SOURCES})
target_link_libraries(SeriousSamNatives EGL GLESv2 log)

# attach serious engine
target_link_libraries(SeriousSamNatives Engine)
target_link_libraries(SeriousSamNatives SeriousSam)

if (STATIC_LINKING)
	if (FIRST_ENCOUNTER)
	target_link_libraries(SeriousSamNatives Entities GameMP)
	else()
	target_link_libraries(SeriousSamNatives EntitiesMP GameMP)
	endif()
endif()
