#cmake 最低版本要求
cmake_minimum_required(VERSION 2.8.0)

#项目名 (雷電　ライデン)
set(RAIDEN raiden)
PROJECT (${RAIDEN})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)

#加入依赖库
add_subdirectory(external)

message(STATUS "------------------------------------主体--------------------------------------")
#根据操作系统配置不同的设置
IF (WIN32)  
    MESSAGE(STATUS "在Windows操作系统下") 
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")	
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules")
ELSEIF (APPLE)  
    MESSAGE(STATUS "在MACOSX操作系统下") 
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_ROOT}/Modules")
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")
ELSEIF (UNIX)  
    MESSAGE(STATUS "在类UNIX操作系统下")  
ENDIF ()  
message(STATUS "当前Cmake根目录:" ${CMAKE_ROOT})
message(STATUS "当前CMAKE_MODULE_PATH:" ${CMAKE_MODULE_PATH})

#使用MSVC的编译器的时候需要设置下
if(MSVC)     
    # Use the static C library for all build types
    foreach(var 
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
      )
      if(${var} MATCHES "/MDd")
        string(REGEX REPLACE "/MDd" "/MD" ${var} "${${var}}")
      endif()
    endforeach()    
endif(MSVC)

#设置源文件目录
set(SOURCE_DIR src)

#添加头文件路径和库路径
include_directories(${SOURCE_DIR})
include_directories(${SOURCE_DIR}/core)
include_directories(${OPENEXR_INCLUDE_DIRS})
message(STATUS "OpenEXR头文件路径: " ${OPENEXR_INCLUDE_DIRS})
include_directories(${LUA_INCLUDE_DIRS})
message(STATUS "Lua头文件路径: " ${LUA_INCLUDE_DIRS})
include_directories(${GLOG_INCLUDE_DIRS})
message(STATUS "glog头文件路径: " ${GLOG_INCLUDE_DIRS})
include_directories(${LODEPNG_INCLUDE_DIRS})
message(STATUS "lodepng头文件路径: " ${LODEPNG_INCLUDE_DIRS})

# #glog库
# find_package(glog REQUIRED)
# include_directories(${GLOG_INCLUDE_PATH})
# message(STATUS "glog头文件路径: " ${GLOG_INCLUDE_PATH})
# message(STATUS "glog库文件路径: " ${GLOG_LIBRARY})

#lua库
# find_package(Lua REQUIRED)
# include_directories(${LUA_INCLUDE_DIR})
# message(STATUS "Lua头文件路径: " ${LUA_INCLUDE_DIR})
# message(STATUS "Lua库文件路径: " ${LUA_LIBRARY})

# #build Zlib库
# if(WIN32)
    
# endif()

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
aux_source_directory(${SOURCE_DIR}/wrapper RAIDEN_SRC_WRAPPER)


set(SOURCE_FILE ${RAIDEN_SRC} ${RAIDEN_SRC_WRAPPER} ${RAIDEN_SRC_CORE} ${RAIDEN_SRC_ACCE} ${RAIDEN_SRC_CAMERA} ${RAIDEN_SRC_FILTER} ${RAIDEN_SRC_INTEGRATOR} ${RAIDEN_SRC_LIGHT} ${RAIDEN_SRC_MATERIAL} ${RAIDEN_SRC_MEDIA} ${RAIDEN_SRC_SAMPLER} ${RAIDEN_SRC_SHAPE} ${RAIDEN_SRC_TEXTURE})
#MESSAGE(STATUS ${SOURCE_FILE})
add_executable(${RAIDEN} ${SOURCE_FILE})

#链接库
target_link_libraries(${RAIDEN} glog)
target_link_libraries(${RAIDEN} lua5.3.4)
target_link_libraries(${RAIDEN} IlmImf)
target_link_libraries(${RAIDEN} lodepng)

if (WIN32)
    set_target_properties(${RAIDEN} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/runtimes")
    set(GETOPT_LIB ${CMAKE_CURRENT_SOURCE_DIR}/external/getopt/wingetopt.lib)
    target_link_libraries(${RAIDEN} ${GETOPT_LIB})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/getopt)
endif()

#工具们
#objtolua
aux_source_directory(${SOURCE_DIR}/tools/objtolua RAIDEN_SRC_OBJ_TO_LUA)
set(OBJ_TO_LUA obj2lua)
add_executable(${OBJ_TO_LUA} ${RAIDEN_SRC_OBJ_TO_LUA})
if (WIN32)
    set_target_properties(${OBJ_TO_LUA} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/runtimes")
endif()