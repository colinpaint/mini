project (decoders)
  add_library (${PROJECT_NAME} iAudioDecoder.h
                               cAudioParser.h cAudioParser.cpp
                               cFFmpegAudioDecoder.h cFFmpegAudioDecoder.cpp
                               stb_image.h
                               lodepng.h lodepng.cpp
                               wuffs-v0.3.c
                               #fpng.h fpng.cpp
                               )

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    target_include_directories (${PROJECT_NAME} PUBLIC common ../include/ffmpeg)
    target_link_directories (${PROJECT_NAME} PUBLIC ../lib64)
    target_link_libraries (${PROJECT_NAME} PUBLIC common avdevice avfilter avformat avcodec avutil swresample swscale)

  else()
    find_package (PkgConfig REQUIRED)
    pkg_check_modules (FFMPEG REQUIRED IMPORTED_TARGET
                       libavdevice libavfilter libavformat libavcodec libavutil libswresample libswscale)

    target_include_directories (${PROJECT_NAME} PUBLIC common PkgConfig::FFMPEG)
    target_link_libraries (${PROJECT_NAME} PUBLIC common PkgConfig::FFMPEG)

  endif()
