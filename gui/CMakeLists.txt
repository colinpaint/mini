project (gui)
  add_library (${PROJECT_NAME} basicBoxes.h
                               ../stb/stb_image.h ../stb/stb_truetype.h
                               cLogBox.h
                               freeSansBold.h freeSansBold.cpp
                               cWindow.h cMiniWindow.cpp
                               cTexture.h cTexture.cpp
                               cDrawAA.h
                               cDrawTexture.h cDrawTexture.cpp
                               )


  # shutup warnings in third party library
  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_include_directories (${PROJECT_NAME} PUBLIC miniFB decoders common windowsLib64)
    target_link_libraries (${PROJECT_NAME} PUBLIC miniFB decoders common Simd.lib)
    set_source_files_properties (cTexture.cpp PROPERTIES COMPILE_FLAGS "/wd4505 /wd4100 /wd4127 /wd4310")
  else()
    target_include_directories (${PROJECT_NAME} PUBLIC miniFB decoders common ../linuxLib64/)
    target_link_libraries (${PROJECT_NAME} PUBLIC miniFB decoders common Simd)
    #target_compile_options (${PROJECT_NAME} PRIVATE -Wno-unused-parameter)
  endif()
