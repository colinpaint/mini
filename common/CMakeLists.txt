project (common)

set (files basicTypes.h date.h cSemaphore.h cLog.h cLog.cpp)

if (CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
  add_library (${PROJECT_NAME} ${files} resolve.h cFileList.h)

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

else()
  add_library (${PROJECT_NAME} ${files})

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
