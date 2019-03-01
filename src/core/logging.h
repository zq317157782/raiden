#include "glog/logging.h"
//打印错误的宏定义，我把它定义在这，PBRT的实现我没有深究，对我来说，这个目前已经做够了
// #define Error(x) LOG(ERROR)<<x;
#define LError LOG(ERROR)
#define LWarning LOG(WARNING)
#define LInfo LOG(INFO)
