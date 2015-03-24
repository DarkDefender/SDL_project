cmake_minimum_required (VERSION 2.6)
project (Tutorial)

list( APPEND CMAKE_CXX_FLAGS "-Wall -ggdb -std=c++11 ${CMAKE_CXX_FLAGS}")

add_executable(${PROJECT_NAME} main.cpp)

include(FindPkgConfig)
include(FindOpenGL)

pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.12)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image)
pkg_search_module(BULLET REQUIRED bullet)

include_directories(${CMAKE_SOURCE_DIR} ${SDL2_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR}
	${SDL2TTF_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS} ${BULLET_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES}
	${SDL2TTF_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${BULLET_LIBRARIES})
