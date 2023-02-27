# cmake -D CMAKE_CXX_COMPILER="/bin/g++" -D CMAKE_C_COMPILER="/bin/gcc" ..
# cmake -D CMAKE_CXX_COMPILER="/bin/clang++" -D CMAKE_C_COMPILER="/bin/clang" ..
#
cmake_minimum_required (VERSION 3.18)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
#
set (CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "limited configs" FORCE)
#
if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  set (BUILD_MSVC_COMPILE_OPTIONS "/arch:AVX" CACHE STRING "msvc compile options")
  option (WINMAIN_ENTRY "winMain entry" OFF)
  option (USE_AVX512 "use avx512" OFF)
  option (USE_AVX2 "use avx2" OFF)
elseif (CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
  set (BUILD_LINUX_COMPILE_OPTIONS "-Ofast" CACHE STRING "gcc compile options")
endif()
#
#
add_subdirectory (fmt)
add_subdirectory (common)
add_subdirectory (simd)
add_subdirectory (miniFB)
add_subdirectory (net)
add_subdirectory (audio)
add_subdirectory (decoders)
add_subdirectory (gui)
add_subdirectory (song)
add_subdirectory (tiledMap)
add_subdirectory (paint)
#
project (mini VERSION 1.0.0)
  add_executable (${PROJECT_NAME} mini.cpp)
  target_include_directories (${PROJECT_NAME} PRIVATE paint songGui tiledMapGui)
  target_link_libraries (${PROJECT_NAME} PRIVATE paint songGui tiledMapGui)
  if (WINMAIN_ENTRY)
    target_compile_definitions (${PROJECT_NAME} PRIVATE WINMAIN_ENTRY)
    target_link_options (${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS)
  endif()
