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
//ImageTexture
//PBRT是实现了储存使用一种格式，返回也使用一种格式的方式
//我目前不使用全频谱的格式，因此我这边只实现储存和查询是一种格式的纹理
//以后有需求再重构成两种格式的
template<typename T>
class ImageTexture: public Texture<T> {
private:
	const std::unique_ptr<TextureMapping2D> _mapping;//纹理映射函数
	Vector2i _resolution;
	std::unique_ptr<T[]> _image;
public:
	ImageTexture(std::unique_ptr<TextureMapping2D> mapping,std::string& fileName):_mapping(std::move(mapping)) {
		std::vector<unsigned char> rawData;
		uint32_t width, height;
		uint32_t error = lodepng::decode(rawData, width, height, fileName);
		if(error){
			LError << "decoder error " << error << ": " << lodepng_error_text(error);
			exit(-1);
		} 
		_resolution.x=width;
		_resolution.y=height;
		//根据分辨率分配空间
		_image.reset(new T[_resolution.x*_resolution.y]);
		//遍历原始数据，初始化图像
		uint32_t index=0;
		for(int j=0;j<_resolution.y;++j){
			for(int i=0;i<_resolution.x;++i){
				//读取纹素
				Float texel[4]; 
				Float invDiv=1.0/255.0;
				texel[0]=rawData[0+index]*invDiv;
				texel[1]=rawData[1+index]*invDiv;
				texel[2]=rawData[2+index]*invDiv;
				texel[3]=rawData[3+index]*invDiv;
				//转换成对应的类型
				_image[i*_resolution.y+j]=*((T*)(texel));
				index+=4;//递增4个字节，因为PNG是按照RGBA 4Byte*8Bit的方式组织的
			}
		}
		//转移数据
		//_image=std::move(data);
	}
	virtual T Evaluate(const SurfaceInteraction & is) const override {
		Vector2f dstdx, dstdy;
		Point2f st = _mapping->Map(is, &dstdx, &dstdy);
		int w=std::min((int)(st.x*_resolution.x),_resolution.x);
		int h=std::min((int)(st.y*_resolution.y),_resolution.y);
		return _image[w*_resolution.y+h];
	}
	virtual ~ImageTexture(){}
};

ImageTexture<Spectrum> *CreateImageSpectrumTexture(const Transform &tex2world,
	const TextureParams &tp);