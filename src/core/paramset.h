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
public:
	ParamSet() {}
	//bool类型相关操作
	void AddBool(const std::string& name,std::unique_ptr<bool[]> values,int nValues);
	bool EraseBool(const std::string& name);
	const bool* FindBool(const std::string& name,int *nValues) const;
	const bool FindOneBool(const std::string& name,int defaultValue) const;
	//Float类型相关操作
	void AddFloat(const std::string& name,std::unique_ptr<Float[]> values,int nValues);
	bool EraseFloat(const std::string& name);
	const Float* FindFloat(const std::string& name,int *nValues) const;
	const Float FindOneFloat(const std::string& name,int defaultValue) const;
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
};

#endif /* SRC_CORE_PARAMSET_H_ */
