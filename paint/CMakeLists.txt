project (paint)
  file (GLOB FILES *.h *.cpp include/*.h base/*.cpp sse41/*.cpp avx2/*.cpp avx512bw/*.cpp)
  add_library (${PROJECT_NAME} ${FILES})
  target_link_libraries (${PROJECT_NAME} PRIVATE gui common)
