add_library(world game_obj.cpp ship_obj.cpp mesh.cpp level.cpp camera.cpp terrain.cpp)

include_directories(${CMAKE_SOURCE_DIR} ${BULLET_INCLUDE_DIRS})

target_link_libraries(world)
