cmake_minimum_required (VERSION 2.6)
project (Tutorial)

list( APPEND CMAKE_CXX_FLAGS "-Wall -ggdb -std=c++11 ${CMAKE_CXX_FLAGS}")

find_package(ZLIB)
# Needed check for libpsf
if ( NOT ZLIB_FOUND )
	set(NO_ZLIB 1)
endif( NOT ZLIB_FOUND )

add_subdirectory(libs)
add_subdirectory(gui)
add_subdirectory(world)

add_executable(${PROJECT_NAME} main.cpp game.cpp timer.cpp ogl_h_func.cpp)

include(FindPkgConfig)
include(FindOpenGL)
include(FindGLEW)


pkg_search_module(SDL2 REQUIRED sdl2)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.12)
pkg_search_module(SDL2IMAGE REQUIRED SDL2_image)
pkg_search_module(BULLET REQUIRED bullet)
pkg_search_module(ASSIMP REQUIRED assimp)

include_directories(${CMAKE_SOURCE_DIR} 
	${OPENGL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} 
	${SDL2_INCLUDE_DIR}	${SDL2TTF_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS}
	${BULLET_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} gui world
	${OPENGL_LIBRARIES} ${GLEW_LIBRARIES}
	${SDL2_LIBRARIES} ${SDL2TTF_LIBRARIES} ${SDL2IMAGE_LIBRARIES} 
	${BULLET_LIBRARIES} ${ASSIMP_LIBRARIES})
