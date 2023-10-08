project (gui)

add_library (${PROJECT_NAME} cWindow.h cMiniWindow.cpp
                             cTexture.h cTexture.cpp
                             cDrawAA.h cDrawTexture.h cDrawTexture.cpp
                             ../stb/stb_truetype.h freeSansBold.h freeSansBold.cpp
                             basicBoxes.h
                             cLogBox.h)

target_include_directories (${PROJECT_NAME} PUBLIC miniFB decoders common simd)
target_link_libraries (${PROJECT_NAME} PUBLIC miniFB decoders common simd)

# shutup warnings in third party library
if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  set_source_files_properties (cTexture.cpp PROPERTIES COMPILE_FLAGS "/wd4505 /wd4100 /wd4127 /wd4310")
else()
  #target_compile_options (${PROJECT_NAME} PRIVATE -Wno-unused-parameter)
endif()
