FIND_PATH(LUA_INCLUDE_PATH lua/lua.h
        /usr/include
        /usr/local/include
        /opt/local/include
        external/include)

FIND_LIBRARY(LUA_LIBRARY NAMES lua PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /opt/local/lib
        external/lib)


IF (LUA_INCLUDE_PATH AND LUA_LIBRARY)
    SET(LUA_FOUND TRUE)
ENDIF ()