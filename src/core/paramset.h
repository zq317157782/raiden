/*
 * paramset.h
 *
 *  Created on: 2016年12月15日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_PARAMSET_H_
#define SRC_CORE_PARAMSET_H_
#include "raiden.h"
#include "geometry.h"
#include <map>
//代表一个参数以及它的值
template<typename T>
struct ParamSetItem {
	const std::string name; //参数名
	const std::unique_ptr<T[]> values; //参数的值
	const int nValues; //参数的长度大于1代表数组
	mutable bool lookUp = false; //判断是否已经查询过这个变量
	ParamSetItem(const std::string& name, std::unique_ptr<T[]> values,
			int nValues = 1) :
			name(name), values(std::move(values)), nValues(nValues) {
	}
};

//代表一个参数集
class ParamSet {
private:
	std::vector<std::shared_ptr<ParamSetItem<bool>>>_bools;
	std::vector<std::shared_ptr<ParamSetItem<int>>> _ints;
	std::vector<std::shared_ptr<ParamSetItem<Float>>> _floats;
	std::vector<std::shared_ptr<ParamSetItem<Point2f>>> _point2fs;
	std::vector<std::shared_ptr<ParamSetItem<Vector2f>>> _vector2fs;
	std::vector<std::shared_ptr<ParamSetItem<Point3f>>> _point3fs;
	std::vector<std::shared_ptr<ParamSetItem<Vector3f>>> _vector3fs;
	std::vector<std::shared_ptr<ParamSetItem<Normal3f>>> _normals;
	std::vector<std::shared_ptr<ParamSetItem<Spectrum>>> _spectra;
	std::vector<std::shared_ptr<ParamSetItem<std::string>>> _strings;
	std::vector<std::shared_ptr<ParamSetItem<std::string>>> _textures;
public:
	ParamSet() {}
	//bool类型相关操作
	void AddBool(const std::string& name,std::unique_ptr<bool[]> values,int nValues);
	bool EraseBool(const std::string& name);
	const bool* FindBool(const std::string& name,int *nValues) const;
	const bool FindOneBool(const std::string& name,bool defaultValue) const;
	//Float类型相关操作
	void AddFloat(const std::string& name,std::unique_ptr<Float[]> values,int nValues);
	bool EraseFloat(const std::string& name);
	const Float* FindFloat(const std::string& name,int *nValues) const;
	const Float FindOneFloat(const std::string& name,Float defaultValue) const;
	//int类型相关操作
	void AddInt(const std::string& name,std::unique_ptr<int[]> values,int nValues);
	bool EraseInt(const std::string& name);
	const int* FindInt(const std::string& name,int *nValues) const;
	const int FindOneInt(const std::string& name,int defaultValue) const;
	//point2i
	void AddPoint2f(const std::string& name,std::unique_ptr<Point2f[]> values,int nValues);
	bool ErasePoint2f(const std::string& name);
	const int FindOnePoint2f(const std::string& name,int defaultValue) const;

	void AddVector2f(const std::string &, std::unique_ptr<Vector2f[]> v,
			int nValues);
	void AddPoint3f(const std::string &, std::unique_ptr<Point3f[]> v,
			int nValues);
	void AddVector3f(const std::string &, std::unique_ptr<Vector3f[]> v,
			int nValues);
	void AddNormal3f(const std::string &, std::unique_ptr<Normal3f[]> v,
			int nValues);
	void AddString(const std::string &, std::unique_ptr<std::string[]> v,
			int nValues);
	void AddTexture(const std::string &, const std::string &);
	void AddRGBSpectrum(const std::string &, std::unique_ptr<Float[]> v,
			int nValues);

	bool EraseVector2f(const std::string &);
	bool ErasePoint3f(const std::string &);
	bool EraseVector3f(const std::string &);
	bool EraseNormal3f(const std::string &);
	bool EraseSpectrum(const std::string &);
	bool EraseString(const std::string &);
	bool EraseTexture(const std::string &);

	Point2f FindOnePoint2f(const std::string &, const Point2f &d) const;
	Vector2f FindOneVector2f(const std::string &, const Vector2f &d) const;
	Point3f FindOnePoint3f(const std::string &, const Point3f &d) const;
	Vector3f FindOneVector3f(const std::string &, const Vector3f &d) const;
	Normal3f FindOneNormal3f(const std::string &, const Normal3f &d) const;
	Spectrum FindOneSpectrum(const std::string &, const Spectrum &d) const;
	std::string FindOneString(const std::string &, const std::string &d) const;

	const Point2f *FindPoint2f(const std::string &, int *nValues) const;
	const Vector2f *FindVector2f(const std::string &, int *nValues) const;
	const Point3f *FindPoint3f(const std::string &, int *nValues) const;
	const Vector3f *FindVector3f(const std::string &, int *nValues) const;
	const Normal3f *FindNormal3f(const std::string &, int *nValues) const;
	const Spectrum *FindSpectrum(const std::string &, int *nValues) const;
	const std::string *FindString(const std::string &, int *nValues) const;
	std::string FindTexture(const std::string &) const;

	//报告为使用的参数
	void ReportUnused() const{
#define CHECK_UNUSED(v) for(i=0;i<(v).size();++i){if(!(v)[i]->lookUp){ Warning("Parameter:\'"<<(v)[i]->name<<"\' not used"); }}
		size_t i = 0;
		CHECK_UNUSED(_bools);
		CHECK_UNUSED(_ints);
		CHECK_UNUSED(_floats);
		CHECK_UNUSED(_point2fs);
		CHECK_UNUSED(_vector2fs);
		CHECK_UNUSED(_point3fs);
		CHECK_UNUSED(_vector3fs);
		CHECK_UNUSED(_normals);
		CHECK_UNUSED(_spectra);
		CHECK_UNUSED(_strings);
		CHECK_UNUSED(_textures);
	}
};

//纹理参数
//包含纹理相关的参数集
class TextureParams {
private:
	const ParamSet& _geomParams; //几何相关的参数集
	const ParamSet& _materialParams; //材质相关的参数集
	std::map<std::string, std::shared_ptr<Texture<Float>>>& _floatTextures;
	std::map<std::string,std::shared_ptr<Texture<Spectrum>>>& _spectrumTextures;
public:
	TextureParams(const ParamSet& geomParams,const ParamSet& materialParams,
			std::map<std::string,std::shared_ptr<Texture<Float>>>& floatTextures,
			std::map<std::string,std::shared_ptr<Texture<Spectrum>>>& spectrumTextures):_geomParams(geomParams),_materialParams(materialParams),
	_floatTextures(floatTextures),_spectrumTextures(spectrumTextures) {

	}
	//寻找一个Float变量
	Float FindFloat(const std::string& name,Float defaultValue) const;
	//寻找一个Spectrum变量
	Spectrum FindSpectrum(const std::string& name,const Spectrum& defaultValue) const;
	//寻找一个string
	std::string FindString(const std::string& name, const std::string& defaultValue="") const;
	
	//寻找一个Vector3f
	Vector3f FindVector3f(const std::string& name, const Vector3f& defaultValue) const;

	//获取一个Spectrum类型的纹理
	std::shared_ptr<Texture<Spectrum>> GetSpectrumTexture( const std::string &n, const Spectrum &def) const;
	//获取一个Float类型的纹理
	std::shared_ptr<Texture<Float>> GetFloatTexture(const std::string &n, Float def) const;

	void ReportUnused() const {
		_geomParams.ReportUnused();
		_materialParams.ReportUnused();
	}
};
#endif /* SRC_CORE_PARAMSET_H_ */
