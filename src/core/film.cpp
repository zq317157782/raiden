/*
 * film.cpp
 *
 *  Created on: 2016年12月2日
 *      Author: zhuqian
 */
#include "film.h"
#include <fstream>


Film::Film(const Point2i& res/*分辨率*/, const Bound2f& cropped/*实际渲染窗口比例*/,
			std::unique_ptr<Filter> filter,
			const std::string& fileName/*输出文件名*/,Float maxSampleLuminance) :
			fullResolution(res), filter(std::move(filter)), fileName(fileName),_maxSampleLuminance(maxSampleLuminance){
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
		//预计算filterTable 只计算1/4
		int offset=0;
		for(int y=0;y<filterTableWidth;++y){
			for(int x=0;x<filterTableWidth;++x,++offset){
				Point2f point;
				point.x=(x+0.5f)*(filter->radius.x/filterTableWidth);
				point.y=(y+0.5f)*(filter->radius.y/filterTableWidth);
				_filterTable[offset]=filter->Evaluate(point);
			}
		}
	}


void Film::WriteImage() {
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
