project (miniFB)

set (files include/MiniFB.h
           include/MiniFB_cpp.h src/MiniFB_cpp.cpp
           include/MiniFB_enums.h
           src/MiniFB_internal.h src/MiniFB_internal.cpp
           src/WindowData.h
           src/MiniFB_common.cpp
           src/MiniFB_timer.cpp
           src/gl/MiniFB_GL.h src/gl/MiniFB_GL.cpp
           )

if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  set (platform src/windows/WindowData_Win.h src/windows/WinMiniFB.cpp)
else()
  set (platform src/x11/WindowData_X11.h src/x11/X11MiniFB.cpp src/MiniFB_linux.cpp)
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
