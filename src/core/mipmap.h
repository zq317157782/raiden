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

//临时实现的暂时存放mipmap数据的结构
template <typename T>
struct MIPMapArray{
    MIPMapArray(int w,int h,T* dd):MIPMapArray(w,h){
		for(int i=0;i<w*h;++i){
            data[i]=dd[i];
        }
    }

    MIPMapArray(int w,int h):width(w),height(h){
        data.reset(new T[w*h]);
    }
    std::unique_ptr<T[]> data;
    uint32_t width;
    uint32_t height;

    void Resize(uint32_t w,uint32_t h){
        width=w;
        height=s;
        data.reset(new T[w*h]);
    }
};

//mipmap实现
template <typename T>
class MIPMap{
private:
    Point2i _resolution;
    std::unique_ptr<T[]> _data;
    WrapMode _wrapMode;
    //PBRT使用的是BlockedArray类型，
    //我还没有研究和实现，
    //所以先暂时自己实现个简单的结构
    std::vector<std::unique_ptr<MIPMapArray<T>>> _pyramid;
    uint32_t _numLevel;
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

	//从某个层级的mipmap获取信息
	T Texel(uint32_t level, uint32_t s, uint32_t t) const {
		uint32_t w = _pyramid[level]->width;
		uint32_t h = _pyramid[level]->height;
		//首先处理包围模式
		if (_wrapMode == WrapMode::Repeat) {
			s = Mod(s, w);
			t = Mod(t, h);
		}
		else if (_wrapMode == WrapMode::Clamp) {
			s = Clamp(s, 0, w - 1);
			t = Clamp(t, 0, h - 1);
		}

		if (s >= 0 && s<w&&t >= 0 &&t<h) {
			return _pyramid[level]->data[t*w + s];
		}
		else {
			//BLACK包围模式
			return 0;
		}
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
        }

        _numLevel=Log2Int(std::max(_resolution[0],_resolution[1]))+1;
        _pyramid.resize(_numLevel);
        //最上层
        _pyramid[0].reset(new MIPMapArray<T>(_resolution[0],_resolution[1],resampledImage.get()));
        //使用BOX过滤生成mipmap
        for(int i=1;i<_numLevel;++i){
			uint32_t w=std::max((uint32_t)1, (uint32_t)_pyramid[i-1]->width/2);
			uint32_t h=std::max((uint32_t)1, (uint32_t)_pyramid[i-1]->height/2);
			_pyramid[i].reset(new MIPMapArray<T>(w,h));

			//并行处理过滤
			ParallelFor([&](int t) {
				for (int s = 0; s < w; ++s) {
					//BOX过滤
					_pyramid[i]->data[t*w + s] = 0.25*(Texel(i - 1, 2 * s, 2 * t) + Texel(i - 1, 2 * s + 1, 2 * t) + Texel(i - 1, 2 * s, 2 * t + 1) + Texel(i - 1, 2 * s + 1, 2 * t + 1));
				}
			}, h, 16);
        }
		WriteMIPMap("MIPMap");
    }

   

    T Lookup(const Point2f &st) const{

		//_data[(int)(st.x*_resolution[0])* _resolution[1] + (int)(st.y* _resolution[1])];
		//LInfo << rgb[0] << " " << rgb[1] << " " << rgb[2] << " ";
        return Texel(0,st[0]* _resolution[0],st[1]* _resolution[1]);
    }


	void WriteMIPMap(std::string name) {

		for (int lv = 0; lv < _numLevel; ++lv) {
			std::vector<uint8_t> image;
			uint32_t w = _pyramid[lv]->width;
			uint32_t h = _pyramid[lv]->height;
			T* data = _pyramid[lv]->data.get();
			Float rgb[3];
			for (int i = 0; i < w; ++i) {
				for (int j = 0; j < h; ++j) {
					T p = data[i*h + j];
					p.ToRGB(rgb);
					//进行sRGB空间下的gamma校验
					rgb[0] = GammaCorrect(rgb[0]);
					rgb[1] = GammaCorrect(rgb[1]);
					rgb[2] = GammaCorrect(rgb[2]);

					//这里clmap了值在0~1LHR范围内
					//这里只是暂时的代码，以后要换成HDR，做ToneMapping
					rgb[0] = Clamp(rgb[0], 0, 1);
					rgb[1] = Clamp(rgb[1], 0, 1);
					rgb[2] = Clamp(rgb[2], 0, 1);

					//Info("[ x:" << i << " y:" << j << "][" << rgb[0] * 255 << " " << rgb[1] * 255 << " " << rgb[2] * 255 << "]");
					image.push_back(rgb[0] * 255);//R
					image.push_back(rgb[1] * 255);//G
					image.push_back(rgb[2] * 255);//B
					image.push_back(255);		//A
				}
			}
			std::ostringstream stringStream;
			stringStream << name << "_level_" << lv << ".png";
			std::string copyOfStr = stringStream.str();
			lodepng::encode(copyOfStr, image, w,
				h);
		}


	}
};