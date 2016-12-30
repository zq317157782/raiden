/*
 * paramset.cpp
 *
 *  Created on: 2016年12月15日
 *      Author: zhuqian
 */
#include "paramset.h"
#include "spectrum.h"
#include "textures/constant.h"
//定义一个宏定义用来添加参数
//这里要求name,values,nValues这三个形参的名字要固定
#define ADD_PARAM_TYPE(T,_list) \
(_list).emplace_back(new ParamSetItem<T>(name,std::move(values),nValues));

//查找指向相应参数值得指针
//这里要求name,values,nValues这三个形参的名字要固定
#define LOOKUP_PTR(_list)\
for(auto &v:_list){\
		if(v->name==name){\
			*nValues=v->nValues;\
			v->lookUp=true;\
			return v->values.get();\
		}\
}\
return nullptr;

//这里是寻找1个参数值得宏定义
#define LOOKUP_ONE(_list)\
for(auto &v:_list){\
		if(v->name==name&&v->nValues==1){\
			v->lookUp=true;\
			return v->values[0];\
		}\
	}\
	return defaultValue;

void ParamSet::AddBool(const std::string& name, std::unique_ptr<bool[]> values,
		int nValues) {
	EraseBool(name);
	ADD_PARAM_TYPE(bool, _bools)
}
void ParamSet::AddFloat(const std::string& name,
		std::unique_ptr<Float[]> values, int nValues) {
	EraseFloat(name);
	ADD_PARAM_TYPE(Float, _floats);
}

void ParamSet::AddInt(const std::string& name, std::unique_ptr<int[]> values,
		int nValues) {
	EraseInt(name);
	ADD_PARAM_TYPE(int, _ints);
}

bool ParamSet::EraseBool(const std::string& name) {
	for (int i = 0; i < _bools.size(); ++i) {
		if (_bools[i]->name == name) {
			_bools.erase(_bools.begin() + i);
			return true;
		}
	}
	return false;
}

bool ParamSet::EraseFloat(const std::string& name) {
	for (int i = 0; i < _floats.size(); ++i) {
		if (_floats[i]->name == name) {
			_floats.erase(_floats.begin() + i);
			return true;
		}
	}
	return false;
}

bool ParamSet::EraseInt(const std::string& name) {
	for (int i = 0; i < _ints.size(); ++i) {
		if (_ints[i]->name == name) {
			_ints.erase(_ints.begin() + i);
			return true;
		}
	}
	return false;
}

const bool* ParamSet::FindBool(const std::string& name, int *nValues) const {
	LOOKUP_PTR(_bools)
}

const Float* ParamSet::FindFloat(const std::string& name, int *nValues) const {
	LOOKUP_PTR(_floats)
}

const int* ParamSet::FindInt(const std::string& name, int *nValues) const {
	LOOKUP_PTR(_ints);
}

const bool ParamSet::FindOneBool(const std::string& name,
		bool defaultValue) const {
	LOOKUP_ONE(_bools)
}

const Float ParamSet::FindOneFloat(const std::string& name,
		Float defaultValue) const {
	LOOKUP_ONE(_floats)
}

const int ParamSet::FindOneInt(const std::string& name,
		int defaultValue) const {
	LOOKUP_ONE(_ints)
}

void ParamSet::AddPoint2f(const std::string &name,
		std::unique_ptr<Point2f[]> values, int nValues) {
	ErasePoint2f(name);
	ADD_PARAM_TYPE(Point2f, _point2fs);
}

void ParamSet::AddVector2f(const std::string &name,
		std::unique_ptr<Vector2f[]> values, int nValues) {
	EraseVector2f(name);
	ADD_PARAM_TYPE(Vector2f, _vector2fs);
}

void ParamSet::AddPoint3f(const std::string &name,
		std::unique_ptr<Point3f[]> values, int nValues) {
	ErasePoint3f(name);
	ADD_PARAM_TYPE(Point3f, _point3fs);
}

void ParamSet::AddVector3f(const std::string &name,
		std::unique_ptr<Vector3f[]> values, int nValues) {
	EraseVector3f(name);
	ADD_PARAM_TYPE(Vector3f, _vector3fs);
}

void ParamSet::AddNormal3f(const std::string &name,
		std::unique_ptr<Normal3f[]> values, int nValues) {
	EraseNormal3f(name);
	ADD_PARAM_TYPE(Normal3f, _normals);
}

void ParamSet::AddString(const std::string &name,
		std::unique_ptr<std::string[]> values, int nValues) {
	EraseString(name);
	ADD_PARAM_TYPE(std::string, _strings);
}

void ParamSet::AddTexture(const std::string &name, const std::string &value) {
	EraseTexture(name);
	std::unique_ptr<std::string[]> str(new std::string[1]);
	str[0] = value;
	std::shared_ptr<ParamSetItem<std::string>> psi(
			new ParamSetItem<std::string>(name, std::move(str), 1));
	_textures.push_back(psi);
}

void ParamSet::AddRGBSpectrum(const std::string &name,
		std::unique_ptr<Float[]> values, int nValues) {
	EraseSpectrum(name);
	Assert(nValues % 3 == 0);
	nValues /= 3;
	std::unique_ptr<Spectrum[]> s(new Spectrum[nValues]);
	for (int i = 0; i < nValues; ++i)
		s[i] = Spectrum::FromRGB(&values[3 * i]);
	std::shared_ptr<ParamSetItem<Spectrum>> psi(
			new ParamSetItem<Spectrum>(name, std::move(s), nValues));
	_spectra.push_back(psi);
}

bool ParamSet::ErasePoint2f(const std::string &n) {
	for (size_t i = 0; i < _point2fs.size(); ++i)
		if (_point2fs[i]->name == n) {
			_point2fs.erase(_point2fs.begin() + i);
			return true;
		}
	return false;
}

bool ParamSet::EraseVector2f(const std::string &n) {
	for (size_t i = 0; i < _vector2fs.size(); ++i)
		if (_vector2fs[i]->name == n) {
			_vector2fs.erase(_vector2fs.begin() + i);
			return true;
		}
	return false;
}

bool ParamSet::ErasePoint3f(const std::string &n) {
	for (size_t i = 0; i < _point3fs.size(); ++i)
		if (_point3fs[i]->name == n) {
			_point3fs.erase(_point3fs.begin() + i);
			return true;
		}
	return false;
}

bool ParamSet::EraseVector3f(const std::string &n) {
	for (size_t i = 0; i < _vector3fs.size(); ++i)
		if (_vector3fs[i]->name == n) {
			_vector3fs.erase(_vector3fs.begin() + i);
			return true;
		}
	return false;
}

bool ParamSet::EraseNormal3f(const std::string &n) {
	for (size_t i = 0; i < _normals.size(); ++i)
		if (_normals[i]->name == n) {
			_normals.erase(_normals.begin() + i);
			return true;
		}
	return false;
}

bool ParamSet::EraseSpectrum(const std::string &n) {
	for (size_t i = 0; i < _spectra.size(); ++i)
		if (_spectra[i]->name == n) {
			_spectra.erase(_spectra.begin() + i);
			return true;
		}
	return false;
}

bool ParamSet::EraseString(const std::string &n) {
	for (size_t i = 0; i < _strings.size(); ++i)
		if (_strings[i]->name == n) {
			_strings.erase(_strings.begin() + i);
			return true;
		}
	return false;
}

bool ParamSet::EraseTexture(const std::string &n) {
	for (size_t i = 0; i < _textures.size(); ++i)
		if (_textures[i]->name == n) {
			_textures.erase(_textures.begin() + i);
			return true;
		}
	return false;
}

std::string ParamSet::FindTexture(const std::string &name) const {
    std::string defaultValue = "";
    LOOKUP_ONE(_textures);
}

const Point2f *ParamSet::FindPoint2f(const std::string &name,
		int *nValues) const {
	LOOKUP_PTR(_point2fs);
}

Point2f ParamSet::FindOnePoint2f(const std::string &name,
		const Point2f &defaultValue) const {
	LOOKUP_ONE(_point2fs);
}

const Vector2f *ParamSet::FindVector2f(const std::string &name,
		int *nValues) const {
	LOOKUP_PTR(_vector2fs);
}

Vector2f ParamSet::FindOneVector2f(const std::string &name,
		const Vector2f &defaultValue) const {
	LOOKUP_ONE(_vector2fs);
}

const Point3f *ParamSet::FindPoint3f(const std::string &name,
		int *nValues) const {
	LOOKUP_PTR(_point3fs);
}

Point3f ParamSet::FindOnePoint3f(const std::string &name,
		const Point3f &defaultValue) const {
	LOOKUP_ONE(_point3fs);
}

const Vector3f *ParamSet::FindVector3f(const std::string &name,
		int *nValues) const {
	LOOKUP_PTR(_vector3fs);
}

Vector3f ParamSet::FindOneVector3f(const std::string &name,
		const Vector3f &defaultValue) const {
	LOOKUP_ONE(_vector3fs);
}

const Normal3f *ParamSet::FindNormal3f(const std::string &name,
		int *nValues) const {
	LOOKUP_PTR(_normals);
}

Normal3f ParamSet::FindOneNormal3f(const std::string &name,
		const Normal3f &defaultValue) const {
	LOOKUP_ONE(_normals);
}

const Spectrum *ParamSet::FindSpectrum(const std::string &name,
		int *nValues) const {
	LOOKUP_PTR(_spectra);
}

Spectrum ParamSet::FindOneSpectrum(const std::string &name,
		const Spectrum &defaultValue) const {
	LOOKUP_ONE(_spectra);
}

const std::string *ParamSet::FindString(const std::string &name,
		int *nValues) const {
	LOOKUP_PTR(_strings);
}

std::string ParamSet::FindOneString(const std::string &name,
		const std::string &defaultValue) const {
	LOOKUP_ONE(_strings);
}

Spectrum TextureParams::FindSpectrum(const std::string& name,
		const Spectrum& defaultValue) const {
	return _geomParams.FindOneSpectrum(name,
			_materialParams.FindOneSpectrum(name, defaultValue));
}

Float TextureParams::FindFloat(const std::string& name,
		Float defaultValue) const {
	return _geomParams.FindOneFloat(name,
			_materialParams.FindOneFloat(name, defaultValue));
}


std::string TextureParams::FindString(const std::string& name, const std::string& defaultValue) const {
	return _geomParams.FindOneString(name,
		_materialParams.FindOneString(name, defaultValue));
}


std::shared_ptr<Texture<Spectrum>> TextureParams::GetSpectrumTexture(
		const std::string &n, const Spectrum &def) const {
	std::string name=_geomParams.FindTexture(n);
	if(name==""){
		name=_materialParams.FindTexture(n);
	}
	if(name!=""){
		if(_spectrumTextures.find(name)!=_spectrumTextures.end()){
			return _spectrumTextures[name];
		}
		else{
			Error("Couldn't find spectrum texture named \""<<name<<"\""
	                "for parameter \""<<n<<"\"");
		}
	}
	Spectrum value=_materialParams.FindOneSpectrum(n,def);
	value=_geomParams.FindOneSpectrum(n,value);
	return std::shared_ptr<Texture<Spectrum>>(new ConstantTexture<Spectrum>(value));
}

