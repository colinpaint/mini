project (simd)
  file (GLOB FILES *.h *.cpp include/*.h sse41/*.cpp avx2/*.cpp avx512/*.cpp)
  add_library (${PROJECT_NAME} ${FILES})
  #
  # internal include directory labelled Simd to allow direct use of Simd_library files
  target_include_directories (${PROJECT_NAME} PRIVATE . ./Simd)
  target_link_libraries (${PROJECT_NAME} PRIVATE common)
  #
  # shutup warnings in third party library
  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_compile_options (${PROJECT_NAME} PRIVATE /wd4100 /wd4244 /wd4310 /wd4127 /wd4714 /wd4189)
  else()
    target_compile_options (${PROJECT_NAME} PRIVATE -Wno-unused-parameter)
  endif()
