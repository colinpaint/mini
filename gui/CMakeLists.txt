project (gui)
  add_library (${PROJECT_NAME} cWindow.h cWindow.cpp
                               ../stb/stb_image.h ../stb/stb_truetype.h
                               freeSansBold.h freeSansBold.cpp
                               cTexture.h cTexture.cpp
                               cDrawAA.h
                               cDrawTexture.h cDrawTexture.cpp
                               basicBoxes.h
                               cIndexBox.h cListBox.h
                               cWindowBox.h cTextureBox.h cLogBox.h
                               cClockBox.h cCalenderBox.h cDateBox.h
                               )

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    # windows
    target_link_directories (${PROJECT_NAME} PUBLIC ../lib/windows64)
    target_link_libraries (${PROJECT_NAME} PUBLIC miniFB decoders Simd.lib)

    # shutup third party library warnings
    set_source_files_properties (cTexture.cpp PROPERTIES COMPILE_FLAGS "/wd4505 /wd4100 /wd4127 /wd4310")

  else()
    # linux
    target_include_directories (${PROJECT_NAME} PUBLIC miniFB decoders lib/linux64)
    target_link_libraries (${PROJECT_NAME} PUBLIC miniFB decoders Simd)

    # shutup third party library warnings
    target_compile_options (${PROJECT_NAME} PRIVATE -Wno-unused-parameter)

  endif()
