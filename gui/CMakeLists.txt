project (gui)
  add_library (${PROJECT_NAME} basicBoxes.h
                               cWindowBox.h cTextureBox.h cLogBox.h
                               cClockBox.h cCalenderBox.h cDateBox.h
                               ../stb/stb_image.h ../stb/stb_truetype.h
                               freeSansBold.h freeSansBold.cpp
                               cWindow.h cMiniWindow.cpp
                               cTexture.h cTexture.cpp
                               cDrawAA.h
                               cDrawTexture.h cDrawTexture.cpp
                               )


  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_include_directories (${PROJECT_NAME} PUBLIC miniFB decoders windowsLib64)
    target_link_libraries (${PROJECT_NAME} PUBLIC miniFB decoders Simd.lib)

    # shutup warnings in third party library
    set_source_files_properties (cTexture.cpp PROPERTIES COMPILE_FLAGS "/wd4505 /wd4100 /wd4127 /wd4310")

  else()
    target_include_directories (${PROJECT_NAME} PUBLIC miniFB decoders linuxLib64/)
    target_link_libraries (${PROJECT_NAME} PUBLIC miniFB decoders Simd)

    # shutup warnings in third party library
    target_compile_options (${PROJECT_NAME} PRIVATE -Wno-unused-parameter)

  endif()
