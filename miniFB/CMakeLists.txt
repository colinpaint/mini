project (miniFB)
  add_library (${PROJECT_NAME} cMiniFB.h cMiniFB.cpp
                               pktDef.h winTab.h
                               )

  target_include_directories (${PROJECT_NAME} PUBLIC common)
  target_link_libraries (${PROJECT_NAME} PUBLIC common)

  find_package (OpenGL REQUIRED)
  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_link_libraries (${PROJECT_NAME} PRIVATE opengl32 d3d11 winmm)
  else()
    find_package (X11 REQUIRED)
    target_include_directories (${PROJECT_NAME} PRIVATE "${X11_X11_INCLUDE_PATH}")
    target_link_libraries (${PROJECT_NAME} PRIVATE -lGL -lX11 -lXi)
  endif()
