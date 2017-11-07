#include "raiden.h"
#include "texture.h"
#include "parallel.h"

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
    std::unique_ptr<T[]> _data;
    WrapMode _wrapMode;
    //计算某一行，或者某一列的权重的函数
    std::unique_ptr<ResampleWeight[]> ResampleWeights(int oldRes,int newRes){
        Assert(oldRes<newRes);

        //默认过滤宽度
        Float filterWidth=2;

        //1. 生成一个存放每个新像素权重的数组
        std::unique_ptr<ResampleWeight[]> wt(new ResampleWeight[newRes]);


        
        //2. 填充权重
        for(int i=0;i<newRes;++i){
            //oldRes/newRes可以提取成一个变量，但是这样子可读性更加高
            Float center=(i+0.5)*oldRes/newRes;
            //这里是非常trick的一种获取第一个像素位置的方式
            wt[i].firstTexel=std::floor((center-filterWidth)+0.5);
            
            
            //使用Lanczos分布计算权重
            for(int j=0;j<4;++j){
                Float pos=wt[i].firstTexel+j+0.5;
                wt[i].weights[j]=Lanczos((pos-center)/filterWidth,2);
            }

            //标准化权重
            Float invSumWeight=1.0/(wt[i].weights[0]+wt[i].weights[1]+wt[i].weights[2]+wt[i].weights[3]);
            for(int j=0;j<4;++j){
                wt[i].weights[j]*=invSumWeight;   
            }


        }

        return wt;
    }

public:
    MIPMap(const Point2i& resolution,T* data,WrapMode wm):_resolution(resolution),_wrapMode(wm){

        //resample后的图像指针
        std::unique_ptr<T[]> resampledImage=nullptr;
       
        //1.判断分辨率是都是2的幂
        if(!IsPowerOf2(_resolution[0])||!IsPowerOf2(_resolution[1])){
            //获得2的幂的分辨率
            Point2i resampledRes(RoundUpPow2(_resolution[0]),RoundUpPow2(_resolution[1]));
            //LInfo<<"MIPMap从"<<_resolution<<" 到"<<resampledResolution<<" Ratio:"<<(resampledResolution[0]*resampledResolution[1])/(_resolution[0]*_resolution[1]);
            
            //为正式的分辨率分配空间
            resampledImage.reset(new T[resampledRes[0]*resampledRes[1]]);
            //计算s方向上的权重
            std::unique_ptr<ResampleWeight[]>sWeights=ResampleWeights(resolution[0],resampledRes[0]);
            //以16列为一组，多线程处理resample s坐标
            ParallelFor([&](int t){
                for(int i=0;i<resampledRes[0];++i){
                    //初始化当前新的文素
                    resampledImage[t*resampledRes[0]+i]=0;

                    for(int j=0;j<4;++j){
                        //计算得到旧坐标系下的S坐标
                        int origS=sWeights[i].firstTexel+j;
                        if(_wrapMode==WrapMode::Repeat){
                            origS=Mod(origS,resolution[0]);
                        }
                        else if(_wrapMode==WrapMode::Clamp){
                            origS=Clamp(origS,0,resolution[0]-1);
                        }
                        if(origS>=0&&origS<resolution[0]){
                            //获得相应的旧纹素值并且乘以相应的权重值
                            resampledImage[t*resampledRes[0]+i]+=data[origS*resolution[1]+t]*sWeights[i].weights[j];
                        }
                    }
                }

            },resolution[1],16);
            
             //计算t方向上的权重
             std::unique_ptr<ResampleWeight[]>tWeights=ResampleWeights(resolution[1], resampledRes[1]);
             //处理t方向上的时候需要一些临时缓存来防止污染resampledImage中的数据
             //临时空间需要手动删除
             std::vector<T*> tempBuf;
             Float numThread= MaxThreadIndex();
             for(int i=0;i<numThread;++i){
                 tempBuf.push_back(new T[resampledRes[1]]);//分配线程数量的临时空间
             }
            
             //以32列为一组，多线程处理resample t坐标
             ParallelFor([&](int s){
                 //拿取临时空间
                 T* workData=tempBuf[ThreadIndex];
                 for(int i=0;i<resampledRes[1];++i){
                     //初始化当前新的文素
                     workData[i]=0;
                     for(int j=0;j<4;++j){
                         //计算得到旧坐标系下的S坐标
                        int origT=tWeights[i].firstTexel+j;

                        if(_wrapMode==WrapMode::Repeat){
                            origT=Mod(origT,resolution[1]);
                        }
                        else if(_wrapMode==WrapMode::Clamp){
                            origT=Clamp(origT,0,resolution[1]-1);
                        }
                        if(origT>=0&&origT<resolution[1]){
                             //获得相应的旧纹素值并且乘以相应的权重值
                         workData[i]+=resampledImage[origT*resampledRes[0]+s]*tWeights[i].weights[j];
                        }
                    }
                 }
                  //覆盖到resampledImage中
                  for(int i=0;i<resampledRes[1];++i){
                     resampledImage[i*resampledRes[0] + s]=workData[i];
                  }
             }, resampledRes[0],32);
             
			 
			 //删除临时空间
             for(int i=0;i<numThread;++i){
                 delete[] tempBuf[i];
             }
            //完成数据的赋值
            _resolution=resampledRes;
            _data=std::move(resampledImage);
        }
        else{
            //设置成原来的图片数据
            _data.reset(data);
        }
    }

    T Lookup(const Point2f &st) const{

		//_data[(int)(st.x*_resolution[0])* _resolution[1] + (int)(st.y* _resolution[1])];
		//LInfo << rgb[0] << " " << rgb[1] << " " << rgb[2] << " ";
        return _data[(int)(st.x*_resolution[0])+(int)(st.y* _resolution[1])* _resolution[0]];
    }
};