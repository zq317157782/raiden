#include "raiden.h"
#include "texture.h"

//提供三种包围模式[裁剪 重复 纯黑]
enum class WrapMode{Clamp,Repeat,Black};

//新的Texel对应的旧的相应的Texel的权重
//只表示一行的权重，默认filter宽度为4,所以只要存4个数据
struct ResampleWeight{
    int firstTexel;//第一个旧texel的坐标，离散坐标
    Float weights[4];
};

//mipmap实现
template <typename T>
class MIPMap{
private:
    Point2i _resolution;

    //计算某一行，或者某一列的权重的函数
    std::unique_ptr<ResampleWeight[]> ResampleWeights(int oldRes,int newRes){

    }

public:
    MIPMap(const Point2i& resolution,T* data):_resolution(resolution){

        //resample后的图像指针
        std::unique_ptr<T[]> resampledImage=nullptr;
       
        //1.判断分辨率是都是2的幂
        if(!IsPowerOf2(_resolution[0])||!IsPowerOf2(_resolution[1])){
            //获得2的幂的分辨率
            Point2i resampledResolution(RoundUpPow2(_resolution[0]),RoundUpPow2(_resolution[1]));
            LInfo<<"MIPMap从"<<_resolution<<" 到 "<<resampledResolution<<" Ratio:"<<(resampledResolution[0]*resampledResolution[1])/(_resolution[0]*_resolution[1]);
        }
    }
};