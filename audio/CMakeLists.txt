project (audio)
  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    add_library (${PROJECT_NAME} iAudio.h
                                 kiss_fft.h kiss_fft_guts.h kiss_fftr.h
                                 kiss_fftr.c kiss_fft.c
                                 cWinAudio32.h cWinAudio32.cpp)
    target_link_libraries (${PROJECT_NAME} PRIVATE common)

  else()
    add_library (${PROJECT_NAME} iAudio.h
                                 kiss_fft.h kiss_fft_guts.h kiss_fftr.h
                                 kiss_fftr.c kiss_fft.c
                                 cLinuxAudio.h)
    target_link_libraries (${PROJECT_NAME} PRIVATE common asound)

  endif()
