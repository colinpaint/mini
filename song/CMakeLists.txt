project (song)
  add_library (${PROJECT_NAME} cDvbSource.h cDvbSource.cpp
                               cSong.h cSong.cpp
                               cSongLoader.h cSongLoader.cpp
                               cSongPlayer.h cSongPlayer.cpp
                               iVideoPool.h cSongVideoPool.cpp
                               )

  target_link_libraries (${PROJECT_NAME} PRIVATE common audio net decoders)

  if (CMAKE_HOST_WIN32)
    target_link_libraries (${PROJECT_NAME} PRIVATE ws2_32)
  endif()
#
#
project (songGui)
  add_library (${PROJECT_NAME} cSongLoaderBox.h cSongLoaderBox.cpp)
  target_link_libraries (${PROJECT_NAME} PUBLIC song gui common)
