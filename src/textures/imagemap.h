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
#include <map>
//纹理的信息
struct TexInfo{
	TexInfo(const std::string& name, WrapMode wm, Float s,bool g):fileName(name), wrapMode(wm), scale(s), gamma(g){

	}
	std::string fileName;//纹理文件的名字
	WrapMode wrapMode; //wrapMode
	Float scale;
	bool gamma;

	bool operator<(const TexInfo &t2) const {
		if (fileName != t2.fileName) {
			return fileName < t2.fileName;
		}
		if (scale != t2.scale) {
			return scale < t2.scale;
		}
		if (gamma != t2.gamma) {
			return !gamma;
		}
		return wrapMode < t2.wrapMode;
	}
};



//ImageTexture
//PBRT是实现了储存使用一种格式，返回也使用一种格式的方式
//我目前不使用全频谱的格式，因此我这边只实现储存和查询是一种格式的纹理
//以后有需求再重构成两种格式的
//需求他就来了*_*
template<typename Tmemory,typename Treturn>
class ImageTexture: public Texture<Tmemory> {
private:
	const std::unique_ptr<TextureMapping2D> _mapping;//纹理映射函数
	MIPMap<Tmemory>* _mipmap;

	//纹理缓存
	static std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>> _textures;
public:
	ImageTexture(std::unique_ptr<TextureMapping2D> mapping,std::string& fileName,WrapMode wrapMode,bool doTrilinear,Float maxAnisotropy,Float scale=1,bool gamma=false):_mapping(std::move(mapping)) {
		_mipmap = GetTexture(fileName, wrapMode,doTrilinear,maxAnisotropy,scale, gamma);
	}
	virtual Treturn Evaluate(const SurfaceInteraction & is) const override {
		Vector2f dstdx, dstdy;
		Point2f st = _mapping->Map(is, &dstdx, &dstdy);
		Tmemory mem= _mipmap->Lookup(st, dstdx, dstdy);
		Treturn ret;
		ConvertOut(mem, &ret);
		return ret;
	}
	virtual ~ImageTexture(){}

	static MIPMap<Tmemory>* GetTexture(std::string& fileName,WrapMode wrapMode,bool doTrilinear,Float maxAnisotropy,Float scale=1,bool gamma=false);

private:
	//一系列转换函数，从from类型转换到to类型，并且根据参数进行缩放和GAMMA校正
	static void ConvertIn(const RGBSpectrum& from,RGBSpectrum* to,Float scale,bool gamma);
	static void ConvertOut(const RGBSpectrum& from, RGBSpectrum* to);
};

template<typename Tmemory, typename Treturn>
std::map<TexInfo, std::unique_ptr<MIPMap<Tmemory>>> ImageTexture<Tmemory, Treturn>::_textures;


template<typename Tmemory,typename Treturn> 
void ImageTexture<Tmemory,Treturn>::ConvertIn(const RGBSpectrum& from,RGBSpectrum* to,Float scale,bool gamma){
     for(int i=0;i<RGBSpectrum::numSample;++i){   
		(*to)[i]=(gamma?InverseGammaCorrect(from[i]):from[i])*scale;
     }
}

template<typename Tmemory, typename Treturn>
void ImageTexture<Tmemory, Treturn>::ConvertOut(const RGBSpectrum& from, RGBSpectrum* to) {
	for (int i = 0; i<RGBSpectrum::numSample; ++i) {
		(*to)[i] = from[i];
	}
}


template<typename Tmemory,typename Treturn> 
MIPMap<Tmemory>*  ImageTexture<Tmemory,Treturn>::GetTexture(std::string& fileName,WrapMode wrapMode,bool doTrilinear,Float maxAnisotropy,Float scale,bool gamma){
	TexInfo texInfo(fileName, wrapMode, scale, gamma);
	if (_textures.find(texInfo) != _textures.end()) {
		return _textures[texInfo].get();
	}

	//读取PNG图片
	std::vector<unsigned char> rawData;
	uint32_t width, height;
	uint32_t error = lodepng::decode(rawData, width, height, fileName);
	std::unique_ptr<Tmemory[]> image;
	if (error) {
		//载入图像失败
		LError << "decoder error " << error << ": " << lodepng_error_text(error);
		
		texInfo.fileName = "error";
		texInfo.wrapMode = WrapMode::Repeat;
		texInfo.scale = 1;
		texInfo.gamma = false;
		if (_textures.find(texInfo) != _textures.end()) {
			return _textures[texInfo].get();
		}
		RGBSpectrum errorRGB;
		errorRGB[0] = 1;
		errorRGB[1] = 0.0196;
		errorRGB[2] = 0.9529;
		_textures[texInfo].reset(new MIPMap<Tmemory>(Point2i(1, 1), &errorRGB, WrapMode::Repeat,doTrilinear,maxAnisotropy));
		return _textures[texInfo].get();
	}
	else {
		//成功载入图像
		
		//1. 首先转换到float 类型
		std::unique_ptr<RGBSpectrum[]> rgbData(new Tmemory[width*height]);
		uint32_t index = 0;
		Float invDiv = 1.0 / 255.0;
		for (int j = 0; j<height; ++j) {
			for (int i = 0; i<width; ++i) {
				int k = i*height + j;
				rgbData[k][0] = rawData[0 + index] * invDiv;
				rgbData[k][1] = rawData[1 + index] * invDiv;
				rgbData[k][2] = rawData[2 + index] * invDiv;
				index += 4;//递增4个字节，因为PNG是按照RGBA 4Byte*8Bit的方式组织的
			}
		}

		//2. 转换PNG图片到Tmemory
		//根据分辨率分配空间
		image.reset(new Tmemory[width*height]);
		for (int i = 0; i<width*height; ++i) {
			ConvertIn(rgbData[i], &(image[i]), scale, gamma);
		}

		//3.创建MIPMap
		_textures[texInfo].reset(new MIPMap<Tmemory>(Point2i(width, height), image.get(), wrapMode,doTrilinear,maxAnisotropy));
		return _textures[texInfo].get();
	}
}

ImageTexture<RGBSpectrum,Spectrum> *CreateImageSpectrumTexture(const Transform &tex2world,
	const TextureParams &tp);