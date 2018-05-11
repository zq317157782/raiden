cmake_minimum_required(VERSION 3.0)
project(lodepng)
aux_source_directory(. LODEPNG_SOURCES)

set(LODEPNG_LIBRARY lodepng)
add_library(${LODEPNG_LIBRARY} STATIC ${LODEPNG_SOURCES})
