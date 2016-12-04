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

class Film {
public:
	//像素结构
	struct Pixel {
		Float xyz[3]; //这里使用的是xyz，xyz和显示器的色彩曲线无关
		Float filterWeightSum;//所有样本的filter权重之和
		Float splatXYZ[3]; //不经过filter
		//到这里是 (float)3*4字节+4字节+3*4字节==28个字节/(double)3*8字节+8字节+3*8字节==56个字节
		Float pad; //用来补充(float)4个字节/(double)8个字节
		//到这里是(float)32字节/(double)64字节
		//in cache line
	};
public:
	const Point2i fullResolution; //完整分辨率
	Bound2i croppedPixelBound; //实际渲染大小,因为不一定渲染整个分辨率所有的情况，可能只渲染部分情况
	const std::string fileName; //输出文件名
public:
	Film(const Point2i& res/*分辨率*/, const Bound2f& cropped/*实际渲染窗口比例*/,
			const std::string& fileName/*输出文件名*/) :
			fullResolution(res), fileName(fileName) {
		Point2i minCropped = Point2i(
				std::ceil(fullResolution.x * cropped.minPoint.x),
				std::ceil(fullResolution.y * cropped.minPoint.y));
		Point2i maxCropped = Point2i(
				std::floor(fullResolution.x * cropped.maxPoint.x),
				std::floor(fullResolution.y * cropped.maxPoint.y));
		croppedPixelBound = Bound2i(minCropped,maxCropped);
	}

	void WriteImage() {

	}
};

#endif /* SRC_CORE_FILM_H_ */
