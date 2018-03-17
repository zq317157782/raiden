cmake_minimum_required(VERSION 3.0)
project(lua)

set(LUA_VERSION 5.3.4)

if(WIN32)
    add_definitions("-DLUA_BUILD_AS_DLL")
endif(WIN32)

aux_source_directory(src LUA_SOURCES)
list(REMOVE_ITEM LUA_SOURCES "src/lua.c" "src/luac.c")

set(LUA_LIBRARY lua${LUA_VERSION})
#add_library(${LUA_LIBRARY} SHARED ${LUA_SOURCES})
add_library(${LUA_LIBRARY} STATIC ${LUA_SOURCES})

# add_executable(lua src/lua.c)
# target_link_libraries(lua ${LUA_LIBRARY})
# if(UNIX AND NOT APPLE) #add math library for linux
# target_link_libraries(lua m)
# endif()

# add_executable(luac ${LUA_SOURCES} src/luac.c)
# if(UNIX AND NOT APPLE)
# target_link_libraries(luac m)
# endif()