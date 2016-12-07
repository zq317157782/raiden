/*
 * film.h
 *
 *  Created on: 2016年12月2日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_FILM_H_
#define SRC_CORE_FILM_H_
#include "raiden.h"
#include "geometry.h"
#include "filter.h"
#include "spectrum.h"
class Film {
public:

public:
	const Point2i fullResolution; //完整分辨率
	Bound2i croppedPixelBound; //实际渲染大小,因为不一定渲染整个分辨率所有的情况，可能只渲染部分情况
	const std::string fileName; //输出文件名
	const std::unique_ptr<Filter> filter;
private:
	//像素结构
	struct Pixel {
		Float xyz[3]; //这里使用的是xyz，xyz和显示器的色彩曲线无关
		Float filterWeightSum; //所有样本的filter权重之和
		Float splatXYZ[3]; //不经过filter
		//到这里是 (float)3*4字节+4字节+3*4字节==28个字节/(double)3*8字节+8字节+3*8字节==56个字节
		Float pad; //用来补充(float)4个字节/(double)8个字节
		//到这里是(float)32字节/(double)64字节
		//in cache line
	};
	std::unique_ptr<Pixel[]> _pixels;//像素值数组，最终要写入image
	const Float _maxSampleLuminance;//最大的样本能量值
	static constexpr int filterTableWidth = 16;//默认filterTable宽度为16
	Float _filterTable[filterTableWidth*filterTableWidth];

	//根据位置获取像素的引用
#ifdef DEBUG_BUILD
public:
#else
private:
#endif
	Pixel& GetPixel(const Point2i& pos) {
		Assert(InsideExclusive(pos, croppedPixelBound));
		int width = croppedPixelBound.maxPoint.x - croppedPixelBound.minPoint.x;
		int index = pos.x - croppedPixelBound.minPoint.x
				+ (pos.y - croppedPixelBound.minPoint.y) * width;
		return _pixels[index];
	}
public:
	Film(const Point2i& res/*分辨率*/, const Bound2f& cropped/*实际渲染窗口比例*/,
			std::unique_ptr<Filter> filter,
			const std::string& fileName/*输出文件名*/,Float maxSampleLuminance=Infinity);
	void WriteImage();
};

//代表Film上的一个Tile
class FilmTile{
private:
	Bound2i _pixelBound;

};

#endif /* SRC_CORE_FILM_H_ */
