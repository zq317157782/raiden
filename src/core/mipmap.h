#pragma once
#include "raiden.h"
#include "texture.h"
#include "parallel.h"
#include "memory.h"
#include "logging.h"
//#include "lodepng.h"
//提供三种包围模式[裁剪 重复 纯黑]
enum class WrapMode{Clamp,Repeat,Black};

//高斯表的大小
static constexpr uint32_t WeightLUTSize=128; 
static Float WeightLUT[WeightLUTSize];//高斯表

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
    std::vector<std::unique_ptr<BlockedArray<T>>> _pyramid;
    uint32_t _numLevel;

    bool _doTrilinear;
    Float _maxAnisotropy;//最大各项异性比例 

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
	T Texel(int level, int s, int t) const {
		int w = _pyramid[level]->USize();
		int h = _pyramid[level]->VSize();
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
            return (*_pyramid[level])(s,t);
		}
		else {
			//BLACK包围模式
			return 0;
		}
	}

	// triangle过滤
	T Triangle(int level, const Point2f& st) const {
		//level = Clamp(level, 0, _numLevel - 1);
        if(level>=_numLevel){
            return Texel(_numLevel-1,0,0);
        }

		int w = _pyramid[level]->USize();
		int h = _pyramid[level]->VSize();
		//这里使用了一定的trick来把连续坐标变换到离散坐标
		Float s = w*st[0]-0.5;
		Float t = h*st[1]-0.5;
		
		//获得4个texel的左下角坐标
		int s0 = std::floor(s);
		int t0 = std::floor(t);

		//计算左下角和样本点之间的距离
		Float ds = s - s0;
		Float dt = t - t0;
		
		//运用三角过滤
		return (ds*dt*Texel(level, s0, t0))+
			((1 - ds)*dt*Texel(level, s0 + 1, t0))+
			(ds*(1 - dt)*Texel(level, s0, t0 + 1))+
			((1 - ds)*(1 - dt)*Texel(level, s0 + 1, t0 + 1));
    }
    
    //EWA过滤
    T EWA(int level, const Point2f& st,const Vector2f& dst0,const Vector2f& dst1) const{
        if(level>=_numLevel){
            return Texel(_numLevel-1,0,0);
        }

        int w = _pyramid[level]->USize();
		int h = _pyramid[level]->VSize();
		//这里使用了一定的trick来把连续坐标变换到离散坐标
		Float s = w*st[0]-0.5;
		Float t = h*st[1]-0.5;

        //把椭圆的两根轴也转换到当前level的坐标系下
        Vector2f majorAxis,minorAxis;
        majorAxis.x=dst0.x*w;
        majorAxis.y=dst0.y*h;
        minorAxis.x=dst1.x*w;
        minorAxis.y=dst1.y*h;

        //计算隐式椭圆的几个系数
        //具体的隐式函数的推导要看 [Heck89]Fundamentals of Texture Mapping and Image 
        Float A=majorAxis[1]*majorAxis[1]+minorAxis[1]*minorAxis[1]+1;
        Float B=-2*(majorAxis[0]*majorAxis[1]+minorAxis[0]*minorAxis[1]);
        Float C=majorAxis[0]*majorAxis[0]+minorAxis[0]*minorAxis[0]+1;
        Float invF=1/(A*C-B*B*0.25);
        A*=invF;
        B*=invF;
        C*=invF;

        //开始计算椭圆在纹理空间中的包围盒
        //公式我没太理解，还是需要看上述论文
        Float det = -B * B + 4 * A * C;
        Float invDet = 1 / det;
        Float uSqrt = std::sqrt(det * C);
        Float vSqrt = std::sqrt(A * det);
        int s0 = std::ceil (st[0] - 2 * invDet * uSqrt);
        int s1 = std::floor(st[0] + 2 * invDet * uSqrt);
        int t0 = std::ceil (st[1] - 2 * invDet * vSqrt);
        int t1 = std::floor(st[1] + 2 * invDet * vSqrt);

        Float sumWeight=0;
        T sum(0);
        //开始遍历包围盒，计算每个Texeld的贡献
        for(int it=t0;it<=t1;++it){
            Float tt=it-st[1];//转换到st为原点的空间
            for(int is=s0;is<=s1;++is){
                Float ss=is-st[0];//转换到st为原点的空间
    
                //计算椭圆系数
                Float r2 = A * ss * ss + B * ss * tt + C * tt * tt;
                //如果当前的texel在椭圆中的话
                if(r2<1){
                    //累积贡献和权重
                    Float weight=WeightLUT[int((WeightLUTSize-1)*r2)];
                    sum+=Texel(level,s,t)*weight;
                    sumWeight+=weight;
                }
            }
        }
        return sum/sumWeight;
    }

public:
    MIPMap(const Point2i& resolution,T* data,WrapMode wm=WrapMode::Repeat,bool doTrilinear=false,Float maxAnisotropy=8.0):_resolution(resolution),_wrapMode(wm),_doTrilinear(doTrilinear),_maxAnisotropy(maxAnisotropy){

        //resample后的图像指针
        std::unique_ptr<T[]> resampledImage=nullptr;
       
        //1.判断分辨率是都是2的幂
        if(!IsPowerOf2(_resolution[0])||!IsPowerOf2(_resolution[1])){
            //获得2的幂的分辨率
            Point2i resampledRes(RoundUpPow2(_resolution[0]),RoundUpPow2(_resolution[1]));
            LInfo<<"MIPMap from"<<_resolution<<" to"<< resampledRes <<" Ratio:"<<(resampledRes[0]* resampledRes[1])/(_resolution[0]*_resolution[1]);
            
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

            //初始化高斯表
            Float alpha=2;
            if(WeightLUT[0]==0){
                for(int i=0;i<WeightLUTSize;++i){
                    Float r2=Float(i)/Float(WeightLUTSize-1);
                    WeightLUT[i]=std::exp(-alpha*r2)-std::exp(-alpha);
                }
            }
		}
		

        _numLevel=Log2Int(std::max(_resolution[0],_resolution[1]))+1;
        _pyramid.resize(_numLevel);
		//LInfo << _numLevel;
        //最上层
        _pyramid[0].reset(new BlockedArray<T>(_resolution[0],_resolution[1], resampledImage?resampledImage.get():data));
        //使用BOX过滤生成mipmap
        for(int i=1;i<_numLevel;++i){
			uint32_t w=std::max((uint32_t)1, (uint32_t)_pyramid[i-1]->USize()/2);
			uint32_t h=std::max((uint32_t)1, (uint32_t)_pyramid[i-1]->VSize()/2);
			_pyramid[i].reset(new BlockedArray<T>(w,h));

			//并行处理过滤
			ParallelFor([&](int t) {
				for (int s = 0; s < w; ++s) {
					//BOX过滤
					(*_pyramid[i])(s,t) = 0.25*(Texel(i - 1, 2 * s, 2 * t) + Texel(i - 1, 2 * s + 1, 2 * t) + Texel(i - 1, 2 * s, 2 * t + 1) + Texel(i - 1, 2 * s + 1, 2 * t + 1));
				}
			}, h, 16);
        }
		//WriteMIPMap("MIPMap");
    }

	

	T Lookup(const Point2f &st, const Vector2f& dstdx, const Vector2f& dstdy) const {
		Float width = std::max(std::max(std::abs(dstdx[0]), std::abs(dstdx[1])),std::max(std::abs(dstdy[0]), std::abs(dstdy[1])));
        if(_doTrilinear){
            //使用Triangle过滤
            return Lookup(st, width*2);
        }

       
        //各项异性版本的EWA过滤
        //判断主轴和副轴
        Vector2f majorAxis;
        Vector2f minorAxis;
        Float majorLength;
        Float minorLength;
        if(dstdx.LengthSquared()>dstdy.LengthSquared()){
            majorAxis=dstdx;
            minorAxis=dstdy;
        }
        else{
            majorAxis=dstdy;
            minorAxis=dstdx;
        }
        //计算Axis长度
        majorLength=majorAxis.Length();
        minorLength=minorAxis.Length();
        //判断是否需要对 minorLength进行缩放
        if(minorLength*_maxAnisotropy<majorLength&&minorLength>0){
            //这里是限制texel最大个数的缩放操作
            Float scale=minorLength/(minorLength*_maxAnisotropy);
            minorAxis=minorAxis*scale;
            minorLength*=scale;
        }
        
        if(minorLength<0){
            //使用细节度最高的三角过滤
            return Triangle(0, st);
        }
        
        //使用minorLength来选择lod
        //这里和三角过滤lookup一样
        Float lod = std::max((Float)0, _numLevel - (Float)1 + Log2(minorLength));
        int lodInt = std::floor(lod);
        Float delta = lod - lodInt;
        return Lerp(delta, EWA(lod, st,majorAxis,minorAxis), EWA(lod+1,st, majorAxis,minorAxis));

	}

	//各项同性版本的Triangle过滤器Lookup
    T Lookup(const Point2f &st,Float width=0.0) const{
		
		// resolution = 2^(_numLevel-1-l)
		// l=_numLevel-1-log2(width);
		//PBRT的公式我认为是错的，它是add log2不是minus log2
		Float level = _numLevel - 1 + Log2(std::max(width, (Float)1e-8));

		if (level < 0) {
			//使用细节量最大的level
			return Triangle(0, st);
		}
		else if (level >= (_numLevel - 1)) {
			//使用最小的level
			return Texel(_numLevel-1,0,0);
		}
		else {
			int levelInt = std::floor(level);
			Float delta = level - levelInt;
			return Lerp(delta, Triangle(levelInt, st), Triangle(levelInt + 1, st));
		}
    }

	

	//void WriteMIPMap(std::string name) {

	//	for (int lv = 0; lv < _numLevel; ++lv) {
	//		std::vector<uint8_t> image;
	//		uint32_t w = _pyramid[lv]->USize();
	//		uint32_t h = _pyramid[lv]->VSize();
	//		Float rgb[3];
	//		for (int j = 0; j < h; ++j) {
	//			for (int i = 0; i < w; ++i) {
	//				T p = (*_pyramid[lv])(i,j);
	//				p.ToRGB(rgb);
	//				//进行sRGB空间下的gamma校验
	//				rgb[0] = GammaCorrect(rgb[0]);
	//				rgb[1] = GammaCorrect(rgb[1]);
	//				rgb[2] = GammaCorrect(rgb[2]);

	//				//这里clmap了值在0~1LHR范围内
	//				//这里只是暂时的代码，以后要换成HDR，做ToneMapping
	//				rgb[0] = Clamp(rgb[0], 0, 1);
	//				rgb[1] = Clamp(rgb[1], 0, 1);
	//				rgb[2] = Clamp(rgb[2], 0, 1);

	//				//Info("[ x:" << i << " y:" << j << "][" << rgb[0] * 255 << " " << rgb[1] * 255 << " " << rgb[2] * 255 << "]");
	//				image.push_back(rgb[0] * 255);//R
	//				image.push_back(rgb[1] * 255);//G
	//				image.push_back(rgb[2] * 255);//B
	//				image.push_back(255);		//A
	//			}
	//		}
	//		std::ostringstream stringStream;
	//		stringStream << name << "_level_" << lv << ".png";
	//		std::string copyOfStr = stringStream.str();
	//		lodepng::encode(copyOfStr, image, w,
	//			h);
	//	}


	//}
};