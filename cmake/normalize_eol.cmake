if (NOT DEFINED INPUT)
  message(FATAL_ERROR "normalize_eol.cmake: INPUT is not set")
endif ()

if (NOT DEFINED OUTPUT)
  message(FATAL_ERROR "normalize_eol.cmake: OUTPUT is not set")
endif ()

file(READ "${INPUT}" _content)
string(REPLACE "\r\n" "\n" _content "${_content}")
string(REPLACE "\r" "\n" _content "${_content}")
file(WRITE "${OUTPUT}" "${_content}")
