/*
 * film.cpp
 *
 *  Created on: 2016年12月2日
 *      Author: zhuqian
 */
#include "film.h"
#include "paramset.h"
#include "imageio.h"


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

void Film::SetImage(const Spectrum* img) {
	int numPixel = croppedPixelBound.Area();
	for (int i = 0; i < numPixel; ++i) {
		Pixel& p = _pixels[i];
		img[i].ToXYZ(p.xyz);
		p.filterWeightSum = 1;
		p.splatXYZ[0] = p.splatXYZ[1] = p.splatXYZ[2] = 0;
	}
}

void Film::WriteImage(Float splatScale) {
	std::vector<Float> image;
	for (int j = croppedPixelBound[0].y; j < croppedPixelBound[1].y; ++j) {
			for (int i = croppedPixelBound[0].x; i < croppedPixelBound[1].x; ++i) {
				Pixel &p = GetPixel(Point2i(i, j));
				Float rgb[3];
				XYZToRGB(p.xyz, rgb);
				if(p.filterWeightSum!=0){
					Float invWeight = 1.0 / p.filterWeightSum;
					rgb[0] *= invWeight;
					rgb[1] *= invWeight;
					rgb[2] *= invWeight;
				}
				
				
				//添加splatXYZ内的能量
				Float splatRGB[3];
				Float splatXYZ[3]={p.splatXYZ[0],p.splatXYZ[1] ,p.splatXYZ[2] };
				XYZToRGB(splatXYZ, splatRGB);

				rgb[0] += splatRGB[0] * splatScale;
				rgb[1] += splatRGB[1] * splatScale;
				rgb[2] += splatRGB[2] * splatScale;

				image.push_back(rgb[0]);
				image.push_back(rgb[1]);
				image.push_back(rgb[2]);
			}
		}
	Vector2i resolution=croppedPixelBound.Diagonal();
	WriteImageToFile(fileName.c_str(), &image[0], resolution.x, resolution.y);
}

void Film::AddSplat(const Point2f& p, Spectrum L) {
	if (L.HasNaNs()) {
		LWarning << "Ignoring splatted spectrum with NaN values at (" << p.x << "," << p.y << ")";
		return;
	}
	else if (L.y() < 0) {
		LWarning<<"Ignoring splatted spectrum with negative luminance"<<L.y()<<"at (" << p.x << "," << p.y << ")";
		return;
	}
	else if (std::isinf(L.y())) {
		LWarning << "Ignoring splatted spectrum with infinite luminance at (" << p.x << "," << p.y << ")";
		return;
	}

	if(!InsideExclusive((Point2i)p, croppedPixelBound)) {
		return;
	}

	if (L.y() > _maxSampleLuminance){
		L = (_maxSampleLuminance / L.y())*L;
	}

	auto& pixel=GetPixel((Point2i)p);
	Float xyz[3];
	L.ToXYZ(xyz);
	for (int i = 0; i < 3; ++i) {
		pixel.splatXYZ[i].Add(xyz[i]);
	}
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
	Point2i pMin = Point2i(Ceil(pFilmDiscrete - _filterRadius));
	Point2i pMax = Point2i(Floor(pFilmDiscrete + _filterRadius))+Point2i(1,1);
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

//{filename:string,xresolution:int,yresolution:int,cropwindow:Float[4],maxsampleluminance:Float}
Film *CreateFilm(const ParamSet &params, std::unique_ptr<Filter> filter) {
    std::string filename = params.FindOneString("filename", "");
    if (RaidenOptions.imageFile != "") {
        if (filename != "") {
        	LWarning<<"ingore commandline filename:"<<RaidenOptions.imageFile.c_str()<<",use descriptionfile filename:"<<filename.c_str();
        } else{
            filename = RaidenOptions.imageFile;
        }
    }
    if (filename == "") {filename = "raiden.png";}

    int xres = params.FindOneInt("xresolution", 1280);
    int yres = params.FindOneInt("yresolution", 720);
    Bound2f crop(Point2f(0, 0), Point2f(1, 1));
    int cwi;
    const Float *cr = params.FindFloat("cropwindow", &cwi);
    if (cr && cwi == 4) {
        crop.minPoint.x = Clamp(std::min(cr[0], cr[1]), 0.0f, 1.0f);
        crop.maxPoint.x = Clamp(std::max(cr[0], cr[1]), 0.0f, 1.0f);
        crop.minPoint.y = Clamp(std::min(cr[2], cr[3]), 0.0f, 1.0f);
        crop.maxPoint.y = Clamp(std::max(cr[2], cr[3]), 0.0f, 1.0f);
    } else if (cr){
    	LError<<"cropwindow need four values, and "<<cwi<<" for now.";
    }
    Float maxSampleLuminance = params.FindOneFloat("maxsampleluminance",
                                                   Infinity);
	Debug("[CreateFilm][ res:" << Point2i(xres, yres) << ",croppedPixelBound:" << crop << ".]");

    return new Film(Point2i(xres, yres), crop, std::move(filter),
                    filename,maxSampleLuminance);
}
