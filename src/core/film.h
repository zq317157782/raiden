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
#include <fstream>
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
	std::unique_ptr<Pixel[]> _pixels;
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
			const std::string& fileName/*输出文件名*/) :
			fullResolution(res), filter(std::move(filter)), fileName(fileName) {
		Point2i minCropped = Point2i(
				std::ceil(fullResolution.x * cropped.minPoint.x),
				std::ceil(fullResolution.y * cropped.minPoint.y));
		Point2i maxCropped = Point2i(
				std::floor(fullResolution.x * cropped.maxPoint.x),
				std::floor(fullResolution.y * cropped.maxPoint.y));
		croppedPixelBound = Bound2i(minCropped, maxCropped);
		//分配储存像素所需要的空间
		_pixels = std::unique_ptr<Pixel[]>(
				new Pixel[croppedPixelBound.SurfaceArea()]);
	}
	void WriteImage() {
		std::ofstream out(fileName);
		Vector2i res = croppedPixelBound[1] - croppedPixelBound[0];
		out << "P3\n" << (res.x) << " " << res.y << "\n255\n";
		for (int j = croppedPixelBound[1].y - 1; j >= croppedPixelBound[0].y;
				--j) {
			for (int i = croppedPixelBound[0].x; i < croppedPixelBound[1].x;
					++i) {
				Pixel p = GetPixel(Point2i(i, j));
				Float rgb[3];
				XYZToRGB(p.xyz, rgb);
				Float invWeight = 1.0 / p.filterWeightSum;
				rgb[0] *= invWeight;
				rgb[1] *= invWeight;
				rgb[2] *= invWeight;
				out << (int) (rgb[0] * 255) << " "
						<< (int) (rgb[1] * 255) << " "
						<< (int) (rgb[2] * 255) << " ";
			}
		}
		out.close();
	}
};

#endif /* SRC_CORE_FILM_H_ */
