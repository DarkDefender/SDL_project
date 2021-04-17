cmake_minimum_required (VERSION 3.10)
project (Tutorial)

if(UNIX)
  list( APPEND CMAKE_CXX_FLAGS "-Wall -ggdb -std=c++11 ${CMAKE_CXX_FLAGS}")
endif(UNIX)

# Optional dependecy for libpsf
find_package(ZLIB)
# Needed check for libpsf
if ( NOT ZLIB_FOUND )
	set(NO_ZLIB 1)
endif( NOT ZLIB_FOUND )

add_subdirectory(libs)
add_executable(${PROJECT_NAME} main.cpp game.cpp timer.cpp ogl_h_func.cpp)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

# Prefer LEGACY OpenGL to be compatible with all the existing releases and
# platforms which don't have GLVND yet. Only do it if preference was not set
# externally. (Only relevant on Linux)
if(NOT DEFINED OpenGL_GL_PREFERENCE)
  set(OpenGL_GL_PREFERENCE "LEGACY")
endif()

#OpenGL libraries
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

# Misc libraries
find_package(Assimp REQUIRED)
find_package(Bullet REQUIRED)

find_package(SDL2 REQUIRED)
find_package(SDL2TTF REQUIRED)
find_package(SDL2IMAGE REQUIRED)

# All subdirectores added after "include_directories" will have these include
# paths.
include_directories(${CMAKE_SOURCE_DIR})
include_directories(SYSTEM
  ${OPENGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS}
  ${SDL2_INCLUDE_DIRS}	${SDL2TTF_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS}
  ${BULLET_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} gui world
	${OPENGL_LIBRARIES} ${GLEW_LIBRARIES}
	${SDL2_LIBRARIES} ${SDL2TTF_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
	${BULLET_LIBRARIES} ${ASSIMP_LIBRARIES})

add_subdirectory(gui)
add_subdirectory(world)
