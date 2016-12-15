/*
 * film.cpp
 *
 *  Created on: 2016年12月2日
 *      Author: zhuqian
 */
#include "film.h"
#include "lodepng.h"
Film::Film(const Point2i& res/*分辨率*/, const Bound2f& cropped/*实际渲染窗口比例*/,
		std::unique_ptr<Filter> filt, const std::string& fileName/*输出文件名*/,
		Float maxSampleLuminance) :
		fullResolution(res), fileName(fileName), filter(std::move(filt)), _maxSampleLuminance(
				maxSampleLuminance) {
	Point2i minCropped = Point2i(
			std::ceil(fullResolution.x * cropped.minPoint.x),
			std::ceil(fullResolution.y * cropped.minPoint.y));
	Point2i maxCropped = Point2i(
			std::floor(fullResolution.x * cropped.maxPoint.x),
			std::floor(fullResolution.y * cropped.maxPoint.y));
	croppedPixelBound = Bound2i(minCropped, maxCropped);
	//分配储存像素所需要的空间
	_pixels = std::unique_ptr<Pixel[]>(new Pixel[croppedPixelBound.Area()]);
	//预计算filterTable 只计算1/4
	int offset = 0;
	for (int y = 0; y < filterTableWidth; ++y) {
		for (int x = 0; x < filterTableWidth; ++x, ++offset) {
			Point2f point;
			point.x = (x + 0.5f) * (filter->radius.x / filterTableWidth);
			point.y = (y + 0.5f) * (filter->radius.y / filterTableWidth);
			_filterTable[offset] = filter->Evaluate(point);
		}
	}

}

void Film::WriteImage() {
	std::vector<uint8_t> image;
	for (int j = croppedPixelBound[1].y - 1; j >= croppedPixelBound[0].y; --j) {
			for (int i = croppedPixelBound[0].x; i < croppedPixelBound[1].x; ++i) {
				Pixel p = GetPixel(Point2i(i, j));
				Float rgb[3];
				XYZToRGB(p.xyz, rgb);
				Float invWeight = 1.0 / p.filterWeightSum;
				rgb[0] *= invWeight;
				rgb[1] *= invWeight;
				rgb[2] *= invWeight;
				image.push_back(rgb[0]*255);//R
				image.push_back(rgb[1]*255);//G
				image.push_back(rgb[2]*255);//B
				image.push_back(255);		//A
			}
		}
	Vector2i resolution=croppedPixelBound.Diagonal();
	unsigned error = lodepng::encode(fileName, image, resolution.x,
			resolution.y);
	if (error)
			std::cerr << "encoder error " << error << ": "
					<< lodepng_error_text(error) << std::endl;
}

void FilmTile::AddSample(const Point2f& pFilm, Spectrum L, Float weight) {
	Assert(weight>0.0f);
	//Assert(InsideExclusive(pFilm,_pixelBound));
	//贡献值大于最大值，需要scale
	if (L.y() > _maxSampleLuminance) {
		L *= (_maxSampleLuminance / L.y());
	}

	//计算过滤范围
	Point2f pFilmDiscrete = pFilm - Vector2f(0.5f, 0.5f);
	Point2i pMin = (Point2i) Ceil(pFilmDiscrete - _filterRadius);
	Point2i pMax = (Point2i) Floor(pFilmDiscrete + _filterRadius);
	pMin = Max(_pixelBound.minPoint, pMin);
	pMax = Min(_pixelBound.maxPoint, pMax);

	//预计算row和col上的偏移
	int *ifx = ALLOCA(int, pMax.x - pMin.x);
	for (int x = pMin.x; x < pMax.x; ++x) {
		Float offsetX = std::abs(x - pFilmDiscrete.x) * _invFilterRadius.x
				* _filterTableWidth;
		ifx[x - pMin.x] = std::min((int) std::floor(offsetX),
				_filterTableWidth - 1);
	}
	int *ify = ALLOCA(int, pMax.y - pMin.y);
	for (int y = pMin.y; y < pMax.y; ++y) {
		Float offsetY = std::abs(y - pFilmDiscrete.y) * _invFilterRadius.y
				* _filterTableWidth;
		ify[y - pMin.y] = std::min((int) std::floor(offsetY),
				_filterTableWidth - 1);
	}

	for (int y = pMin.y; y < pMax.y; ++y) {
		for (int x = pMin.x; x < pMax.x; ++x) {
			int filterTableIndex = ify[y - pMin.y] * _filterTableWidth
					+ ifx[x - pMin.x];	//filter表索引
			Float fiterWeight = _filterTable[filterTableIndex];	//过滤器权重
			FilmTilePixel& pixel = GetPixel(Point2i(x, y));
			pixel.contribSum += L * weight * fiterWeight;
			pixel.filterWeightSum += fiterWeight;
		}
	}
}



std::unique_ptr<FilmTile> Film::GetFilmTile(const Bound2i &sampleBounds) {
	Vector2f half(0.5f, 0.5f);
	Bound2f floatBounds = (Bound2f) sampleBounds;
	Point2i p0 = (Point2i) Ceil(floatBounds.minPoint - half - filter->radius);
	Point2i p1 = (Point2i) Floor(floatBounds.maxPoint - half + filter->radius)
			+ Point2i(1, 1);
	Bound2i pixelBound=Intersect(Bound2i(p0,p1),croppedPixelBound);
	return std::unique_ptr<FilmTile>(new FilmTile(pixelBound,filter->radius,_filterTable,filterTableWidth,_maxSampleLuminance));
}

void Film::MergeFilmTile(std::unique_ptr<FilmTile> tile){
	//获取互斥锁
	std::lock_guard<std::mutex> lock(_mutex);
	for(Point2i pixelPos:tile->GetPixelBound()){
		const FilmTilePixel& tilePixel=tile->GetPixel(pixelPos);
		Pixel& pixel=GetPixel(pixelPos);
		Float xyz[3];
		tilePixel.contribSum.ToXYZ(xyz);
		for(int i=0;i<3;++i){
			pixel.xyz[i]+=xyz[i];
		}
		pixel.filterWeightSum+=tilePixel.filterWeightSum;
	}
}
