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
#include <mutex>
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
		Pixel(){
			xyz[0] = 0.0;
			xyz[1] = 0.0;
			xyz[2] = 0.0;
			filterWeightSum = 0.0;
			splatXYZ[0] = 0.0;
			splatXYZ[1] = 0.0;
			splatXYZ[2] = 0.0;
		}
	};
	std::unique_ptr<Pixel[]> _pixels;		//像素值数组，最终要写入image
	const Float _maxSampleLuminance;		//最大的样本能量值
	static constexpr int filterTableWidth = 16;		//默认filterTable宽度为16
	Float _filterTable[filterTableWidth * filterTableWidth];
	std::mutex _mutex;
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
			const std::string& fileName/*输出文件名*/, Float maxSampleLuminance =
					Infinity);
	void WriteImage();
	//获取一个tile
	std::unique_ptr<FilmTile> GetFilmTile(const Bound2i &sampleBounds);
	//合并1个tile
	void MergeFilmTile(std::unique_ptr<FilmTile> tile);

	//考虑了filter范围
	Bound2i GetSampleBounds() const{
		Point2f minP=Floor(Point2f(croppedPixelBound.minPoint)+Vector2f(0.5f,0.5f)-filter->radius);
		Point2f maxP=Ceil(Point2f(croppedPixelBound.maxPoint)-Vector2f(0.5f,0.5f)+filter->radius);
		return Bound2i((Point2i)minP, (Point2i)maxP);
	}
	//直接设置image数据
	void SetImage(const Spectrum* img);
};

struct FilmTilePixel {
	Spectrum contribSum = 0.0f;
	Float filterWeightSum = 0.0f;
};

//代表Film上的一个Tile
class FilmTile {
private:
	const Bound2i _pixelBound;
	const Vector2f _filterRadius, _invFilterRadius;
	const Float* _filterTable;
	const int _filterTableWidth;
	const Float _maxSampleLuminance;
	std::vector<FilmTilePixel> _pixels;
	friend class Film;
public:
	FilmTile(const Bound2i& pixelBound, const Vector2f& filterRadius,
			const Float* filterTable, int filterTableWidth,
			Float maxSampleLuminance) :
			_pixelBound(pixelBound), _filterRadius(filterRadius), _invFilterRadius(
					filterRadius.x, filterRadius.y), _filterTable(filterTable), _filterTableWidth(
					filterTableWidth), _maxSampleLuminance(maxSampleLuminance) {
		_pixels = std::vector<FilmTilePixel>(std::max(0, pixelBound.Area()));
	}

	FilmTilePixel& GetPixel(const Point2i& pos) {
		Assert(InsideExclusive(pos, _pixelBound));
		int width = _pixelBound.maxPoint.x - _pixelBound.minPoint.x;
		int index = pos.x - _pixelBound.minPoint.x
				+ (pos.y - _pixelBound.minPoint.y) * width;
		return _pixels[index];
	}

	FilmTilePixel GetPixel(const Point2i& pos) const {
		Assert(InsideExclusive(pos, _pixelBound));
		int width = _pixelBound.maxPoint.x - _pixelBound.minPoint.x;
		int index = pos.x - _pixelBound.minPoint.x
				+ (pos.y - _pixelBound.minPoint.y) * width;
		return _pixels[index];
	}
	//向tile中贡献样本
	void AddSample(const Point2f& pFilm, Spectrum L, Float weight);

	const Bound2i& GetPixelBound() const {
		return _pixelBound;
	}
};
//{filename:string,xresolution:int,yresolution:int,cropwindow:Float[4],maxsampleluminance:Float}
Film *CreateFilm(const ParamSet &params, std::unique_ptr<Filter> filter);

#endif /* SRC_CORE_FILM_H_ */
