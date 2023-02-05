project (miniFB)

set (files MiniFB.h
           MiniFB_cpp.h MiniFB_cpp.cpp
           MiniFB_enums.h
           MiniFB_internal.h MiniFB_internal.cpp
           WindowData.h
           MiniFB_common.cpp
           MiniFB_timer.cpp
           MiniFB_GL.h MiniFB_GL.cpp
           )

if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  set (platform WindowData_Win.h WinMiniFB.cpp pktDef.h winTab.h)
else()
  set (platform WindowData_X11.h X11MiniFB.cpp MiniFB_linux.cpp)
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
