project (audio)

set (files iAudio.h
           kiss_fft.h kiss_fft.c
           kiss_fft_guts.h
           kiss_fftr.h kiss_fftr.c)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  set (platform cWinAudio32.h cWinAudio32.cpp)
else()
  set (platform cLinuxAudio.h)
endif()

add_library (${PROJECT_NAME} ${files} ${platform})
target_link_libraries (${PROJECT_NAME} PRIVATE common)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
else()
  target_link_libraries (${PROJECT_NAME} PRIVATE asound)
endif()
