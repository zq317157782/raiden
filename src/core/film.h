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
	};
public:
	const Point2i fullResolution; //完整分辨率
	Bound2i croppedPixelBound; //实际渲染大小
	const std::string fileName;
public:
	Film(const Point2i& res/*分辨率*/, const Bound2f& cropped,
			const std::string& fileName) :
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
