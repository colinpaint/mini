project (common)
  message (STATUS "using ${CMAKE_HOST_SYSTEM_NAME}")

  if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    message (STATUS "building for Windows")
    add_library (${PROJECT_NAME} basicTypes.h date.h cSemaphore.h cLog.h cLog.cpp resolve.h cFileList.h)

    target_link_directories (${PROJECT_NAME} PUBLIC ../lib64)
    target_link_libraries (${PROJECT_NAME} PUBLIC fmt tbb12)

    target_compile_definitions (${PROJECT_NAME} PUBLIC ${BUILD_D2D})
    target_compile_options (${PROJECT_NAME} PUBLIC /W4
                                                   ${BUILD_MSVC_COMPILE_OPTIONS}
                                                   /fp:fast
                                                   )
    if (USE_AVX512)
      target_compile_options (${PROJECT_NAME} PUBLIC /arch:AVX512)
    elseif (USE_AVX2)
      target_compile_options (${PROJECT_NAME} PUBLIC /arch:AVX2)
    else()
      target_compile_options (${PROJECT_NAME} PUBLIC /arch:AVX)
    endif()

  elseif()
    message (STATUS "building for Linux")
    add_library (${PROJECT_NAME} basicTypes.h date.h cSemaphore.h cLog.h cLog.cpp)

    include (findTBB.cmake)
    target_link_libraries (${PROJECT_NAME} PUBLIC fmt stdc++fs TBB::tbb pthread)

    target_compile_definitions (${PROJECT_NAME} PUBLIC _LARGEFILE64_SOURCE _FILE_OFFSET_BITS=64)
    target_compile_options (${PROJECT_NAME} PUBLIC -Wall
                                                   -Wextra
                                                   -Wno-missing-field-initializers
                                                   -Wno-format-security
                                                   -pedantic
                                                   -flax-vector-conversions
                                                   -ftree-vectorize
                                                   -fno-exceptions
                                                   -march=native
                                                   -g
                                                   ${BUILD_LINUX_COMPILE_OPTIONS}
                                                   #-Wno-format-overflow
                                                   )
  endif()
