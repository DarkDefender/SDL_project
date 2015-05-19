add_library(world game_obj.cpp ship_obj.cpp hut_obj.cpp turr_obj.cpp trail_obj.cpp mesh.cpp level.cpp camera.cpp terrain.cpp)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(SYSTEM ${BULLET_INCLUDE_DIRS})

target_link_libraries(world)
