if(WIN32)
  # 编译zlib (only on Windows)
  # 需要给OpenEXR使用
  set(ZLIB_BUILD_STATIC_LIBS ON CACHE BOOL " " FORCE)
  set(ZLIB_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
  add_subdirectory(zlib)

  set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib" CACHE PATH " " FORCE)
  set(ZLIB_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/zlib/${CMAKE_BUILD_TYPE}/zlibstatic.lib" CACHE FILEPATH " " FORCE)
  #MESSAGE(${CMAKE_BUILD_TYPE})
  set_property(TARGET zlibstatic PROPERTY FOLDER "dependencies")
  include_directories(${ZLIB_INCLUDE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/zlib")
endif()

#编译OpenEXR 使用了Nori内的版本
set(ILMBASE_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
set(OPENEXR_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
set(ILMBASE_NAMESPACE_VERSIONING OFF CACHE BOOL " " FORCE)
set(OPENEXR_NAMESPACE_VERSIONING OFF CACHE BOOL " " FORCE)
add_subdirectory(openexr)
set_property(TARGET IexMath eLut toFloat b44ExpLogTable dwaLookups CopyIlmBaseLibs IlmThread Half Iex Imath IlmImf PROPERTY FOLDER "dependencies")

#编译Lua
add_subdirectory(lua)

#编译glog
set(WITH_GFLAGS OFF)
add_subdirectory(glog)

#编译lodepng
add_subdirectory(lodepng)

if(RAIDEN_UNITTEST)
  #编译gtest
  set(gtest_force_shared_crt ON)
  set(BUILD_GMOCK OFF)
  set(BUILD_GTEST ON)
  add_subdirectory(googletest)
endif(RAIDEN_UNITTEST)

#设置需要包含的头文件目录
# OpenEXR
set(OPENEXR_INCLUDE_DIRS 
  ${CMAKE_CURRENT_SOURCE_DIR}/openexr/IlmBase/Imath 
  ${CMAKE_CURRENT_SOURCE_DIR}/openexr/IlmBase/Iex 
  ${CMAKE_CURRENT_SOURCE_DIR}/openexr/IlmBase/Half 
  ${CMAKE_CURRENT_SOURCE_DIR}/openexr/OpenEXR/IlmImf 
  ${CMAKE_CURRENT_BINARY_DIR}/openexr/OpenEXR/config 
  ${CMAKE_CURRENT_BINARY_DIR}/openexr/IlmBase/config )
set(LUA_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lua/src)
set(GLOG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/glog/src/glog)
set(LODEPNG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/lodepng)
if(RAIDEN_UNITTEST)
  set(GTEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/googletest/googletest/include/gtest)
endif(RAIDEN_UNITTEST)

#暴露参数给父作用域
set(CompilerFlags OPENEXR_INCLUDE_DIRS LUA_INCLUDE_DIRS GLOG_INCLUDE_DIRS LODEPNG_INCLUDE_DIRS GTEST_INCLUDE_DIRS)
foreach(CompilerFlag ${CompilerFlags})
  set(${CompilerFlag} "${${CompilerFlag}}" PARENT_SCOPE)
endforeach()