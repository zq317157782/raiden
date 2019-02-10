
#include "config.h"
#ifdef WIN32
  #include "getopt.h"
  #define GOOGLE_GLOG_DLL_DECL           // 使用静态glog库用这个
  #define GLOG_NO_ABBREVIATED_SEVERITIES // 没这个编译会出错,传说因为和Windows.h冲突
#else
  #include <unistd.h>
#endif

#include "gtest.h"

int main(int argc, char* argv[]) {
	int ret=0;
    testing::InitGoogleTest(&argc,argv);
    ret= RUN_ALL_TESTS();
    return ret;
}

