project (audio)

set (files iAudio.h
           kiss_fft.h kiss_fft.c kiss_fft_guts.h kiss_fftr.h kiss_fftr.c)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  set (platform cWinAudio32.h cWinAudio32.cpp)
else()
  set (platform cLinuxAudio.h)
endif()

add_library (${PROJECT_NAME} ${files} ${platform})

if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  target_link_libraries (${PROJECT_NAME} PRIVATE common)
else()
  target_link_libraries (${PROJECT_NAME} PRIVATE common asound)
endif()
