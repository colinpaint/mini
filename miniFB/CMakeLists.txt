project (miniFB)

set (files miniFB.h miniFBenums.h sWindowData.h miniFB.cpp
           miniFBinternal.h miniFBinternal.cpp
           miniFBgl.h miniFBgl.cpp)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  set (platform sWindowDataWindows.h miniFBwindows.cpp pktDef.h winTab.h)
else()
  set (platform sWindowDataX11.h miniFBx11.cpp miniFBlinux.cpp)
endif()

add_library (${PROJECT_NAME} ${files} ${platform})

target_include_directories (${PROJECT_NAME} PUBLIC common)
target_link_libraries (${PROJECT_NAME} PUBLIC common)

find_package (OpenGL REQUIRED)
if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  target_link_libraries (${PROJECT_NAME} PRIVATE opengl32 d3d11 winmm)
else()
  find_package (X11 REQUIRED)
  target_include_directories (${PROJECT_NAME} PRIVATE "${X11_X11_INCLUDE_PATH}")
  target_link_libraries (${PROJECT_NAME} PRIVATE -lGL -lX11)
endif()
