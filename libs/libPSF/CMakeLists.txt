add_library(psf libPSF.c PSF.h)

include_directories(SYSTEM ${ZLIB_INCLUDE_DIRS})
target_link_libraries(psf ${ZLIB_LIBRARIES})
