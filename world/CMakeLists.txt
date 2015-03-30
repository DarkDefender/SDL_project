add_library(world game_obj.cpp mesh.cpp)

include_directories(${CMAKE_SOURCE_DIR})

target_link_libraries(world)
