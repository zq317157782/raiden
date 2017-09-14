FIND_PATH(GLOG_INCLUDE_PATH glog/logging.h
        /usr/include
        /usr/local/include
        /opt/local/include
        external/include)

FIND_LIBRARY(GLOG_LIBRARY NAMES glog PATHS
        /usr/lib
        /usr/lib64
        /usr/local/lib
        /opt/local/lib
        external/lib)


IF (GLOG_INCLUDE_PATH AND GLOG_LIBRARY)
    SET(GLOG_FOUND TRUE)
ENDIF ()