project (paint)
  file (GLOB FILES cPaint.h cPaint.cpp cPaintbox.h cPaintBox.cpp
                   include/*.h 
                   base/*.cpp sse41/*.cpp avx2/*.cpp avx512bw/*.cpp)
  add_library (${PROJECT_NAME} ${FILES})

  target_link_libraries (${PROJECT_NAME} PRIVATE common gui)
