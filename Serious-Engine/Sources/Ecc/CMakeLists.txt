# find ecc from /Serious-Engine/Sources/Tools.Win32
set(ECC_PRECOMPILED "${PROJECT_ROOT}/Serious-Engine/Tools.Win32/ecc.exe")

if (NOT EXISTS "${ECC_PRECOMPILED}")
  message(FATAL_ERROR "Serious-Engine/Tools.Win32/ecc.exe not found.\nYou need to compile it manually from Serious-Engine/Sources/Ecc/ecc.sln")
endif ()

set(ECC_EXECUTABLE "${ECC_PRECOMPILED}" CACHE STRING "Global scope" FORCE)
message("ECC_EXECUTABLE: ${ECC_EXECUTABLE}")
ADD_CUSTOM_TARGET(se-ecc DEPENDS "${ECC_EXECUTABLE}")
