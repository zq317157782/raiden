#cmake 最低版本要求
cmake_minimum_required(VERSION 2.8.0)

#项目名 (雷電　ライデン)
set(RAIDEN raiden)
PROJECT (${RAIDEN})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)


#根据操作系统配置不同的设置
IF (WIN32)  
    MESSAGE(STATUS "在Windows操作系统下") 
    message(STATUS "当前Cmake根目录:" ${CMAKE_ROOT})	
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
ELSEIF (APPLE)  
    MESSAGE(STATUS "在MACOSX操作系统下") 
    message(STATUS "当前Cmake根目录:" ${CMAKE_ROOT})
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
  ELSEIF (UNIX)  
    MESSAGE(STATUS "在类UNIX操作系统下")  
ENDIF ()  
 
 
set(SOURCE_DIR src)

#添加头文件路径和库路径
include_directories(${SOURCE_DIR})
include_directories(${SOURCE_DIR}/core)
#include_directories(${SOURCE_DIR}/core/lus)

#设置源文件
aux_source_directory(${SOURCE_DIR} RAIDEN_SRC)
aux_source_directory(${SOURCE_DIR}/core RAIDEN_SRC_CORE)
aux_source_directory(${SOURCE_DIR}/accelerators RAIDEN_SRC_ACCE)
aux_source_directory(${SOURCE_DIR}/cameras RAIDEN_SRC_CAMERA)
aux_source_directory(${SOURCE_DIR}/filters RAIDEN_SRC_FILTER)
aux_source_directory(${SOURCE_DIR}/integrators RAIDEN_SRC_INTEGRATOR)
aux_source_directory(${SOURCE_DIR}/lights RAIDEN_SRC_LIGHT)
aux_source_directory(${SOURCE_DIR}/materials RAIDEN_SRC_MATERIAL)
aux_source_directory(${SOURCE_DIR}/media RAIDEN_SRC_MEDIA)
aux_source_directory(${SOURCE_DIR}/samplers RAIDEN_SRC_SAMPLER)
aux_source_directory(${SOURCE_DIR}/shapes RAIDEN_SRC_SHAPE)
aux_source_directory(${SOURCE_DIR}/textures RAIDEN_SRC_TEXTURE)

aux_source_directory(${SOURCE_DIR}/core/lua RAIDEN_SRC_CORE_LUA)

set(SOURCE_FILE ${RAIDEN_SRC} ${RAIDEN_SRC_CORE} ${RAIDEN_SRC_CORE_LUA} ${RAIDEN_SRC_ACCE} ${RAIDEN_SRC_CAMERA} ${RAIDEN_SRC_FILTER} ${RAIDEN_SRC_INTEGRATOR} ${RAIDEN_SRC_LIGHT} ${RAIDEN_SRC_MATERIAL} ${RAIDEN_SRC_MEDIA} ${RAIDEN_SRC_SAMPLER} ${RAIDEN_SRC_SHAPE} ${RAIDEN_SRC_TEXTURE})
#MESSAGE(STATUS ${SOURCE_FILE})
add_executable(${RAIDEN} ${SOURCE_FILE})
