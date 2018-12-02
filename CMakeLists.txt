cmake_minimum_required(VERSION 3.0.0)
project(raiden VERSION 0.1.0)

include(CTest)
enable_testing()

#Window下默认这个变量没有赋值，所以这里默认赋值成release
if (WIN32)
    SET(CMAKE_BUILD_TYPE release)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

MESSAGE(STATUS "CMAKE_BUILD_TYPE = " ${CMAKE_BUILD_TYPE})
#设置输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")
#使用C++11的语言特征
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#设置的参数
option (RAIDEN_DOUBLE_PRECISION "Use double precision float number" OFF)
option (RAIDEN_FLOAT_PRECISION "Use float precision float number" ON)
option (RAIDEN_UNITTEST "Run unittest cases" ON)
MESSAGE(STATUS "Unit Test:" ${RAIDEN_UNITTEST})

# 加入一个配置头文件，用于处理 CMake 对源码的设置
configure_file (
  "${PROJECT_SOURCE_DIR}/config.h.in"
  "${PROJECT_BINARY_DIR}/config.h"
)

####依赖库的设置
#FIND_PACKAGE (ZLIB)
if(WIN32)
  # 编译zlib (only on Windows)
  # 需要给OpenEXR使用
  set(ZLIB_BUILD_STATIC_LIBS ON CACHE BOOL " " FORCE)
  set(ZLIB_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
  add_subdirectory(external/zlib)

  set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/zlib" CACHE PATH " " FORCE)
  
  set(ZLIB_LIBRARY zlibstatic)
  #MESSAGE(${CMAKE_BUILD_TYPE})
  set_property(TARGET zlibstatic PROPERTY FOLDER "external")
  include_directories(${ZLIB_INCLUDE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/external/zlib")
endif()
#message("+++" ${ZLIB_LIBRARY})

#编译OpenEXR 使用了Nori内的版本
set(ILMBASE_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
set(OPENEXR_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
set(ILMBASE_NAMESPACE_VERSIONING OFF CACHE BOOL " " FORCE)
set(OPENEXR_NAMESPACE_VERSIONING OFF CACHE BOOL " " FORCE)
add_subdirectory(external/openexr)
set_property(TARGET IexMath eLut toFloat b44ExpLogTable dwaLookups  IlmThread Half Iex Imath IlmImf PROPERTY FOLDER "external")

#编译Lua
add_subdirectory(external/lua)

#编译glog
set(WITH_GFLAGS OFF)
add_subdirectory(external/glog)

#编译lodepng
add_subdirectory(external/lodepng)

if(RAIDEN_UNITTEST)
  #编译gtest
  set(gtest_force_shared_crt ON)
  set(BUILD_GMOCK OFF)
  set(BUILD_GTEST ON)
  add_subdirectory(external/googletest)
endif(RAIDEN_UNITTEST)


#####包含头文件
include_directories(src)
include_directories(src/core)
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
set(OPENEXR_INCLUDE_DIRS 
  ${CMAKE_CURRENT_SOURCE_DIR}/external/openexr/IlmBase/Imath 
  ${CMAKE_CURRENT_SOURCE_DIR}/external/openexr/IlmBase/Iex 
  ${CMAKE_CURRENT_SOURCE_DIR}/external/openexr/IlmBase/Half 
  ${CMAKE_CURRENT_SOURCE_DIR}/external/openexr/OpenEXR/IlmImf 
  ${CMAKE_CURRENT_BINARY_DIR}/external/openexr/OpenEXR/config 
  ${CMAKE_CURRENT_BINARY_DIR}/external/openexr/IlmBase/config )
include_directories(${OPENEXR_INCLUDE_DIRS})#openexr
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/lua/src)#lua
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/glog/src/glog)#glog
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/lodepng)#lodepng
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/pugixml)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
if(RAIDEN_UNITTEST)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external//googletest/googletest/include/gtest)
endif(RAIDEN_UNITTEST)

#设置源文件
aux_source_directory(src/core RAIDEN_SRC_CORE)
aux_source_directory(src/accelerators RAIDEN_SRC_ACCE)
aux_source_directory(src/cameras RAIDEN_SRC_CAMERA)
aux_source_directory(src/filters RAIDEN_SRC_FILTER)
aux_source_directory(src/integrators RAIDEN_SRC_INTEGRATOR)
aux_source_directory(src/lights RAIDEN_SRC_LIGHT)
aux_source_directory(src/materials RAIDEN_SRC_MATERIAL)
aux_source_directory(src/media RAIDEN_SRC_MEDIA)
aux_source_directory(src/samplers RAIDEN_SRC_SAMPLER)
aux_source_directory(src/shapes RAIDEN_SRC_SHAPE)
aux_source_directory(src/textures RAIDEN_SRC_TEXTURE)
aux_source_directory(src/binders RAIDEN_SRC_BINDER)
if(RAIDEN_UNITTEST)
    aux_source_directory(test/core RAIDEN_TEST_CORE)
    aux_source_directory(test/binders RAIDEN_TEST_BINDERS)
endif(RAIDEN_UNITTEST)

add_executable(raiden src/main.cpp 
${RAIDEN_SRC_BINDER} 
${RAIDEN_SRC_CORE} 
${RAIDEN_SRC_ACCE} 
${RAIDEN_SRC_CAMERA} 
${RAIDEN_SRC_FILTER} 
${RAIDEN_SRC_INTEGRATOR} 
${RAIDEN_SRC_LIGHT} 
${RAIDEN_SRC_MATERIAL} 
${RAIDEN_SRC_MEDIA} 
${RAIDEN_SRC_SAMPLER} 
${RAIDEN_SRC_SHAPE} 
${RAIDEN_SRC_TEXTURE} 
${RAIDEN_TEST_CORE} 
${RAIDEN_TEST_BINDERS}
external/pugixml/pugixml.cpp)

#链接库
target_link_libraries(raiden glog)
target_link_libraries(raiden lua5.3.4)
IF(WIN32)
  SET(OPENEXR_LIBS IlmImf Imath Half ${ZLIB_LIBRARY})
ELSE()
  SET(OPENEXR_LIBS IlmImf Imath Half)
ENDIF()
target_link_libraries(raiden ${OPENEXR_LIBS})
target_link_libraries(raiden lodepng)
if(RAIDEN_UNITTEST)
  target_link_libraries(raiden gtest)
endif(RAIDEN_UNITTEST)

if (WIN32)
    #set_target_properties(${RAIDEN} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/runtimes")
    set(GETOPT_LIB ${CMAKE_CURRENT_SOURCE_DIR}/external/getopt/wingetopt.lib)
    target_link_libraries(raiden ${GETOPT_LIB})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/getopt)
endif()

add_custom_command(TARGET raiden POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy
                  "${CMAKE_CURRENT_SOURCE_DIR}/src/scripts/lua/raiden.lua"
                  "${CMAKE_CURRENT_BINARY_DIR}/raiden.lua")

#objtolua
aux_source_directory(src/tools/objtolua RAIDEN_SRC_OBJ_TO_LUA)
set(OBJ_TO_LUA obj2lua)
add_executable(obj2lua ${RAIDEN_SRC_OBJ_TO_LUA})
# if (WIN32)
#     set_target_properties(obj2lua PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/runtimes")
# endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)


