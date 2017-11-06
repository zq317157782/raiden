/*
 * imagetexture.h
 *
 *  Created on: 2017年10月26日
 *      Author: zhuqian
 */

#pragma once
#include "raiden.h"
#include "texture.h"
#include "lodepng.h"
#include "mipmap.h"
//ImageTexture
//PBRT是实现了储存使用一种格式，返回也使用一种格式的方式
//我目前不使用全频谱的格式，因此我这边只实现储存和查询是一种格式的纹理
//以后有需求再重构成两种格式的
//需求他就来了*_*
template<typename Tmemory,typename Treturn>
class ImageTexture: public Texture<Tmemory> {
private:
	const std::unique_ptr<TextureMapping2D> _mapping;//纹理映射函数
	Point2i _resolution;
	std::unique_ptr<Tmemory[]> _image;
	std::unique_ptr<MIPMap<Tmemory>> _mipmap;
public:
	ImageTexture(std::unique_ptr<TextureMapping2D> mapping,std::string& fileName,Float scale=1,bool gamma=false):_mapping(std::move(mapping)) {
		//1.读取PNG图片
		std::vector<unsigned char> rawData;
		uint32_t width, height;
		uint32_t error = lodepng::decode(rawData, width, height, fileName);
		if(error){
			//菜如图像失败
			LError << "decoder error " << error << ": " << lodepng_error_text(error);
			_resolution.x=1;
			_resolution.y=1;
			_image.reset(new Tmemory[_resolution.x*_resolution.y]);
			RGBSpectrum errorRGB;
			errorRGB[0]=1;
			errorRGB[1]=0.0196;
			errorRGB[2]=0.9529;
			_image[0]=errorRGB;
		} else{
			//成功载入图像
			_resolution.x=width;
			_resolution.y=height;
			std::unique_ptr<RGBSpectrum[]> rgbData(new Tmemory[_resolution.x*_resolution.y]);
			
			uint32_t index=0;
			Float invDiv=1.0/255.0;
			for(int j=0;j<_resolution.y;++j){
				for(int i=0;i<_resolution.x;++i){
					int k=i*_resolution.y+j;
					rgbData[k][0]=rawData[0+index]*invDiv;
					rgbData[k][1]=rawData[1+index]*invDiv;
					rgbData[k][2]=rawData[2+index]*invDiv;
					index+=4;//递增4个字节，因为PNG是按照RGBA 4Byte*8Bit的方式组织的
				}
			}
	
			//2. 转换PNG图片到Tmemory
			//根据分辨率分配空间
			_image.reset(new Tmemory[_resolution.x*_resolution.y]);
			for(int i=0;i<_resolution.x*_resolution.y;++i){
				ConvertIn(rgbData[i],&(_image[i]),scale,gamma);
			}

			 _mipmap.reset(new MIPMap<Tmemory>(_resolution,&_image[0]));
		}
	}
	virtual Treturn Evaluate(const SurfaceInteraction & is) const override {
		Vector2f dstdx, dstdy;
		Point2f st = _mapping->Map(is, &dstdx, &dstdy);
		int w=std::min((int)(st.x*_resolution.x),_resolution.x);
		int h=std::min((int)(st.y*_resolution.y),_resolution.y);
		return _mipmap->Lookup(st);
		//return _image[w*_resolution.y+h];
	}
	virtual ~ImageTexture(){}

private:
	//一系列转换函数，从from类型转换到to类型，并且根据参数进行缩放和GAMMA校正
	static void ConvertIn(const RGBSpectrum& from,RGBSpectrum* to,Float scale,bool gamma);
};

template<typename Tmemory,typename Treturn> 
void ImageTexture<Tmemory,Treturn>::ConvertIn(const RGBSpectrum& from,RGBSpectrum* to,Float scale,bool gamma){
     for(int i=0;i<RGBSpectrum::numSample;++i){   
		(*to)[i]=(gamma?InverseGammaCorrect(from[i]):from[i])*scale;
     }
}

ImageTexture<RGBSpectrum,Spectrum> *CreateImageSpectrumTexture(const Transform &tex2world,
	const TextureParams &tp);