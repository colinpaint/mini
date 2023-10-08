project (tiledMap)
  add_library (${PROJECT_NAME} cTiledMap.h cTiledMap.cpp)
  target_include_directories (${PROJECT_NAME} PUBLIC ../include)
  target_link_libraries (${PROJECT_NAME} PUBLIC common gui)
#
#
project (tiledMapGui)
  add_library (${PROJECT_NAME} cTiledMapBox.h cTiledMapBox.cpp 
                               cTiledMapOverviewBox.h cTiledMapOverviewBox.cpp)
  target_link_libraries (${PROJECT_NAME} PUBLIC tiledMap)
