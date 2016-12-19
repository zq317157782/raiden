/*
 * api.cpp
 *
 *  Created on: 2016年12月16日
 *      Author: zhuqian
 */
#include "api.h"
#include <map>
#include "transform.h"
#include "paramset.h"
#include "spectrum.h"
#include "parallel.h"
#include "shapes/sphere.h"
#include "cameras/pinhole.h"
#include "samplers/random.h"
#include "filters/box.h"
#include "accelerators/iteration.h"
#include "lights/point.h"
#include "integrators/normal.h"
#include "film.h"
#include "scene.h"
#include "memory.h"
//transform相关参数
constexpr int MaxTransforms = 2;
constexpr int StartTransformBits = 1 << 0;	//0x01
constexpr int EndTransformBits = 1 << 1;	//0x10
constexpr int AllTransformsBits = (1 << MaxTransforms) - 1;	//0x11
//transform集合
struct TransformSet {
private:
	Transform _t[MaxTransforms];
public:
	Transform& operator[](int i) {
		Assert(i >= 0 && i < MaxTransforms);
		return _t[i];
	}
	const Transform& operator[](int i) const {
		Assert(i >= 0 && i < MaxTransforms);
		return _t[i];
	}
	//判断是否包含不同的transform
	bool IsAnimated() const {
		for (int i = 0; i < MaxTransforms - 1; ++i) {
			if (_t[i] != _t[i + 1]) {
				return true;
			}
		}
		return false;
	}
	//返回transformSet的逆
	friend TransformSet Inverse(const TransformSet& ts) {
		TransformSet invSet;
		for (int i = 0; i < MaxTransforms; ++i) {
			invSet._t[i] = Inverse(ts._t[i]);
		}
		return invSet;
	}
};

//Transform缓存
class TransformCache {
private:
	std::map<Transform, std::pair<Transform*/*common*/, Transform*/*inv*/>> _cache;
	MemoryArena _arena;
public:
	//寻找缓存，没找到就创建缓存
	void Lookup(const Transform&t, Transform **tCached,
			Transform **tCachedInv) {
		//先尝试寻找是否已经存在缓存
		auto iter = _cache.find(t);
		//没找到，缓存新的
		if (iter == _cache.end()) {
			Transform *tt = _arena.Alloc<Transform>(1);
			*tt = t;
			Transform *tInv = _arena.Alloc<Transform>(1);
			*tInv = Inverse(t);
			_cache[t] = std::make_pair(tt, tInv);
			iter = _cache.find(t);
		}
		//这里改变的是指针的值
		if (tCached) {
			*tCached = iter->second.first;
		}
		if (tCachedInv) {
			*tCachedInv = iter->second.second;
		}
	}

	void Clear() {
		_arena.Reset();
		_cache.erase(_cache.begin(), _cache.end());
	}
};
//渲染参数
struct RenderOptions {
	Float transformStartTime = 0;
	Float transformEndTime = 1;
	//过滤器名字  默认 box
	std::string FilterName = "box";
	ParamSet FilterParams;
	//Film的名字
	std::string FilmName = "image";
	ParamSet FilmParams;
	//采样器的名字
	std::string SamplerName = "random";
	ParamSet SamplerParams;
	//加速结构的名字
	std::string AcceleratorName = "iteration";
	ParamSet AcceleratorParams;
	//积分器的名字
	std::string IntegratorName = "normal";
	ParamSet IntegratorParams;
	//相机的名字
	std::string CameraName = "pinhole";
	ParamSet CameraParams;
	TransformSet CameraToWorld;
	//光源
	std::vector<std::shared_ptr<Light>> lights;
	//图元
	std::vector<std::shared_ptr<Primitive>> primitives;

	//创建一个scene
	Scene *MakeScene();
	//创建一个Camera
	Camera *MakeCamera() const;
	//创建一个积分器
	Integrator *MakeIntegrator() const;
};

struct GraphicsState {
	bool reverseOrientation = false;	//是否要翻转法线
};

//系统的三个状态
enum class APIState {
	Uninitialized, OptionsBlock, WorldBlock
};
static APIState currentApiState = APIState::Uninitialized;	//当前状态
static TransformSet curTransform;	//当前的TransformSet
static uint32_t activeTransformBits = AllTransformsBits; //当前的TransformSet Bit
static std::map<std::string, TransformSet> namedCoordinateSystems; //保存坐标系的map
static std::unique_ptr<RenderOptions> renderOptions; //当前渲染参数
static GraphicsState graphicsState; //当前图形状态
static std::vector<GraphicsState> pushedGraphicsStates;
static std::vector<TransformSet> pushedTransforms;
static std::vector<uint32_t> pushedActiveTransformBits;
static TransformCache transformCache;
//创建一个shape
std::vector<std::shared_ptr<Shape>> MakeShapes(const std::string &name,
		const Transform *object2world, const Transform *world2object,
		bool reverseOrientation, const ParamSet &paramSet) {
	std::vector<std::shared_ptr<Shape>> shapes;
	std::shared_ptr<Shape> s;
	if (name == "sphere") {
		s = CreateSphereShape(object2world, world2object, reverseOrientation,
				paramSet);
	} else {
		Error("shape \""<<name.c_str()<<"\" unknown.");
	}
	if (s != nullptr) {
		shapes.push_back(s);
	}
	return shapes;
}

//创建加速结构
std::shared_ptr<Primitive> MakeAccelerator(const std::string &name,
		const std::vector<std::shared_ptr<Primitive>> &prims,
		const ParamSet &paramSet) {
	std::shared_ptr<Primitive> accel;
	if (name == "iteration") {
		accel = CreateIterationAccelerator(prims, paramSet);
	} else {
		Error("accelerator \""<<name.c_str()<<"\" unknown.");
	}
	return accel;
}

//创建相机
Camera *MakeCamera(const std::string &name, const ParamSet &paramSet,
		const TransformSet &cam2worldSet, Film *film) {
	Camera *camera = nullptr;
	Transform *cam2world[1];
	transformCache.Lookup(cam2worldSet[0], &cam2world[0], nullptr);
	if (name == "pinhole") {
		camera = CreatePinholeCamera(paramSet, *cam2world[0], film);
	} else {
		Error("camera \""<<name.c_str()<<"\" unknown.");
	}
	return camera;
}
//创建film
Film *MakeFilm(const std::string &name, const ParamSet &paramSet,
		std::unique_ptr<Filter> filter) {
	Film *film = nullptr;
	if (name == "image") {
		film = CreateFilm(paramSet, std::move(filter));
		Debug(
				"[make film: "<<name<<" ,res:"<< film->fullResolution<<" ,croppedPixelBound:"<< film->croppedPixelBound<<".]");
	} else {
		Error("film \""<<name.c_str()<<"\" unknown.");
	}
	return film;
}

std::shared_ptr<Sampler> MakeSampler(const std::string &name,
		const ParamSet &paramSet, const Film *film) {
	Sampler *sampler = nullptr;
	if (name == "random") {
		sampler = CreateRandomSampler(paramSet);
	} else {
		Error("sampler \""<<name.c_str()<<"\" unknown.");
	}
	return std::shared_ptr<Sampler>(sampler);
}

std::unique_ptr<Filter> MakeFilter(const std::string &name,
		const ParamSet &paramSet) {
	Filter *filter = nullptr;
	if (name == "box") {
		filter = CreateBoxFilter(paramSet);
	} else {
		Error("filter \""<<name.c_str()<<"\" unknown.");
		exit(1);
	}
	return std::unique_ptr<Filter>(filter);
}

std::shared_ptr<Light> MakeLight(const std::string &name,
		const ParamSet &paramSet, const Transform &light2world) {
	std::shared_ptr<Light> light;
	if (name == "point") {
		light = CreatePointLight(light2world, paramSet);
	} else {
		Error("light \""<<name.c_str()<<"\" unknown.");
	}
	return light;
}

//确认现在的系统是否已经初始完毕
#define VERIFY_INITIALIZED(func)                           \
    if (currentApiState == APIState::Uninitialized) {      \
    	Error("need call raidenInit() before call "<<func<<"()");	\
        return;                                            \
    } else

#define VERIFY_OPTIONS(func)                           \
    if (currentApiState == APIState::WorldBlock) {      \
    	Error("can't call "<<func<<"() inside WorlBlock.");	\
        return;                                          \
    } else

#define VERIFY_WORLD(func)                           \
    if (currentApiState == APIState::OptionsBlock) {      \
    	Error("must call "<<func<<"() inside WorlBlock.");	\
        return;                                          \
    } else

//为每个transform 执行 expr表达式
#define FOR_ACTIVE_TRANSFORMS(expr)           \
    for (int i = 0; i < MaxTransforms; ++i)   \
        if (activeTransformBits & (1 << i)) { \
            expr                              \
        }

void raidenInit(const Options &opt) {
	RaidenOptions = opt; //初始化系统参数
	if (currentApiState != APIState::Uninitialized) {
		Error("raidenInit() has been called.");
	}
	currentApiState = APIState::OptionsBlock;
	renderOptions.reset(new RenderOptions);
	graphicsState = GraphicsState();
	//一些初始化函数
	SampledSpectrum::Init(); //初始化一些光谱相关的数据
	ParallelInit(); //初始化并行工作队列
}

void raidenCleanup() {
	if (currentApiState == APIState::Uninitialized) {
		Error("raidenInit() must be called befor call raidenCleanup().");
	} else if (currentApiState == APIState::WorldBlock) {
		printf("raidenCleanup() can't be called inside WorldBlock.");
	}
	currentApiState = APIState::Uninitialized;
	ParallelCleanup();
	renderOptions.reset(nullptr);
}
//单位化每个矩阵变换
void raidenIdentity() {
	VERIFY_INITIALIZED("Identity");
	FOR_ACTIVE_TRANSFORMS(curTransform[i] = Transform()
	;
)
}

//translate
void raidenTranslate(Float dx, Float dy, Float dz) {
VERIFY_INITIALIZED("Translate");
FOR_ACTIVE_TRANSFORMS(
		curTransform[i] = curTransform[i] * Translate(Vector3f(dx, dy, dz))
		;
)
}

void raidenTransform(Float tr[16]) {
VERIFY_INITIALIZED("Transform");
FOR_ACTIVE_TRANSFORMS(
	curTransform[i] = Transform(
			Matrix4x4(tr[0], tr[4], tr[8], tr[12], tr[1], tr[5], tr[9], tr[13],
					tr[2], tr[6], tr[10], tr[14], tr[3], tr[7], tr[11], tr[15]))
	;
)
}

void raidenConcatTransform(Float tr[16]) {
VERIFY_INITIALIZED("ConcatTransform");
FOR_ACTIVE_TRANSFORMS(
curTransform[i] = curTransform[i]
* Transform(
Matrix4x4(tr[0], tr[4], tr[8], tr[12], tr[1], tr[5], tr[9], tr[13], tr[2],
tr[6], tr[10], tr[14], tr[3], tr[7], tr[11], tr[15]))
;
)
}

void raidenRotate(Float angle, Float dx, Float dy, Float dz) {
VERIFY_INITIALIZED("Rotate");
FOR_ACTIVE_TRANSFORMS(
curTransform[i] = curTransform[i] * Rotate(angle, Vector3f(dx, dy, dz))
;
)
}

void raidenScale(Float sx, Float sy, Float sz) {
VERIFY_INITIALIZED("Scale");
FOR_ACTIVE_TRANSFORMS(curTransform[i] = curTransform[i] * Scale(sx, sy, sz)
;
)
}

//void raidenLookAt(Float ex, Float ey, Float ez, Float lx, Float ly, Float lz,
//                Float ux, Float uy, Float uz) {
//    VERIFY_INITIALIZED("LookAt");
//    Transform lookAt =
//        LookAt(Point3f(ex, ey, ez), Point3f(lx, ly, lz), Vector3f(ux, uy, uz));
//    FOR_ACTIVE_TRANSFORMS(curTransform[i] = curTransform[i] * lookAt;);
//}

void raidenCoordinateSystem(const std::string &name) {
VERIFY_INITIALIZED("CoordinateSystem");
namedCoordinateSystems[name] = curTransform;
}

void raidenCoordSysTransform(const std::string &name) {
VERIFY_INITIALIZED("CoordSysTransform");
if (namedCoordinateSystems.find(name) != namedCoordinateSystems.end()) {
curTransform = namedCoordinateSystems[name];
} else {
Error("cant find CoordinateSystem: \""<< name.c_str()<<"\"");
}
}

void raidenActiveTransformAll() {
activeTransformBits = AllTransformsBits;
}

void raidenActiveTransformEndTime() {
activeTransformBits = EndTransformBits;
}

void raidenActiveTransformStartTime() {
activeTransformBits = StartTransformBits;
}

void raidenTransformTimes(Float start, Float end) {
VERIFY_OPTIONS("TransformTimes");
renderOptions->transformStartTime = start;
renderOptions->transformEndTime = end;
}

void raidenPixelFilter(const std::string &name, const ParamSet &params) {
VERIFY_OPTIONS("PixelFilter");
renderOptions->FilterName = name;
renderOptions->FilterParams = params;
}

void raidenFilm(const std::string &type, const ParamSet &params) {
VERIFY_OPTIONS("Film");
renderOptions->FilmParams = params;
renderOptions->FilmName = type;
}

void raidenSampler(const std::string &name, const ParamSet &params) {
VERIFY_OPTIONS("Sampler");
renderOptions->SamplerName = name;
renderOptions->SamplerParams = params;
}

void raidenAccelerator(const std::string &name, const ParamSet &params) {
VERIFY_OPTIONS("Accelerator");
renderOptions->AcceleratorName = name;
renderOptions->AcceleratorParams = params;
}

void raidenIntegrator(const std::string &name, const ParamSet &params) {
VERIFY_OPTIONS("Integrator");
renderOptions->IntegratorName = name;
renderOptions->IntegratorParams = params;
}

void raidenCamera(const std::string &name, const ParamSet &params) {
VERIFY_OPTIONS("Camera");
renderOptions->CameraName = name;
renderOptions->CameraParams = params;
renderOptions->CameraToWorld = Inverse(curTransform);
namedCoordinateSystems["camera"] = renderOptions->CameraToWorld;
}

void raidenShape(const std::string &name, const ParamSet &params) {
VERIFY_WORLD("Shape");
std::vector<std::shared_ptr<Primitive>> prims;
if (!curTransform.IsAnimated()) {
Transform *ObjToWorld, *WorldToObj;
transformCache.Lookup(curTransform[0], &ObjToWorld, &WorldToObj);
std::vector<std::shared_ptr<Shape>> shapes = MakeShapes(name, ObjToWorld,
WorldToObj, graphicsState.reverseOrientation, params);
if (shapes.size() == 0) {
return;
}
for (auto s : shapes) {
prims.push_back(std::make_shared<GeomPrimitive>(s));
}
}
//把创建的shape 插入到renderOption中的容器中
renderOptions->primitives.insert(renderOptions->primitives.end(), prims.begin(),
prims.end());
}

void raidenWorldBegin() {
VERIFY_OPTIONS("WorldBegin");
currentApiState = APIState::WorldBlock;
for (int i = 0; i < MaxTransforms; ++i) {
curTransform[i] = Transform();
}
activeTransformBits = AllTransformsBits;
namedCoordinateSystems["world"] = curTransform;
}

void raidenAttributeBegin() {
VERIFY_WORLD("AttributeBegin");
pushedGraphicsStates.push_back(graphicsState);
pushedTransforms.push_back(curTransform);
pushedActiveTransformBits.push_back(activeTransformBits);
}

void raidenAttributeEnd() {
VERIFY_WORLD("AttributeEnd");
if (!pushedGraphicsStates.size()) {
printf("raidenAttributeEnd()不匹配.");
return;
}
graphicsState = pushedGraphicsStates.back();
pushedGraphicsStates.pop_back();
curTransform = pushedTransforms.back();
pushedTransforms.pop_back();
activeTransformBits = pushedActiveTransformBits.back();
pushedActiveTransformBits.pop_back();
}

void raidenTransformBegin() {
VERIFY_WORLD("TransformBegin");
pushedTransforms.push_back(curTransform);
pushedActiveTransformBits.push_back(activeTransformBits);
}

void raidenTransformEnd() {
VERIFY_WORLD("TransformEnd");
if (!pushedTransforms.size()) {
Error("raidenTransformEnd() miss match");
return;
}
curTransform = pushedTransforms.back();
pushedTransforms.pop_back();
activeTransformBits = pushedActiveTransformBits.back();
pushedActiveTransformBits.pop_back();
}

Scene *RenderOptions::MakeScene() {
std::shared_ptr<Primitive> accelerator = MakeAccelerator(AcceleratorName,
primitives, AcceleratorParams);
if (!accelerator) {
 //默认使用的加速结构
accelerator = std::make_shared<Iteration>(primitives);
}
Scene *scene = new Scene(accelerator, lights);
//从randeroptions中删除primitives和lights
primitives.erase(primitives.begin(), primitives.end());
lights.erase(lights.begin(), lights.end());
return scene;
}

Camera *RenderOptions::MakeCamera() const {
std::unique_ptr<Filter> filter = MakeFilter(FilterName, FilterParams);
Film *film = MakeFilm(FilmName, FilmParams, std::move(filter));
if (!film) {
Error("film cant be made.");
return nullptr;
}
Debug("[make camera: "<<CameraName<<" .]");
Camera *camera = ::MakeCamera(CameraName, CameraParams, CameraToWorld, film);
return camera;
}

Integrator *RenderOptions::MakeIntegrator() const {
std::shared_ptr<const Camera> camera(MakeCamera());
if (!camera) {
Error("camera cant be made.");
return nullptr;
}
std::shared_ptr<Sampler> sampler = MakeSampler(SamplerName, SamplerParams,
camera->film);
if (!sampler) {
Error("sampler cant be made.");
return nullptr;
}

Integrator *integrator = nullptr;
if (IntegratorName == "normal") {
integrator = CreateNormalIntegrator(IntegratorParams, sampler, camera);
} else {
Error("integrator \""<<IntegratorName.c_str()<<"\" unkonwn.");
return nullptr;
}
return integrator;
}

void raidenWorldEnd() {
VERIFY_WORLD("WorldEnd");

while (pushedGraphicsStates.size()) {
Warning("raidenAttributeEnd() miss match. try to fix it auto");
pushedGraphicsStates.pop_back();
pushedTransforms.pop_back();
}
while (pushedTransforms.size()) {
Warning("raidenTransformEnd() miss match. try to fix it auto");
pushedTransforms.pop_back();
}

std::unique_ptr<Integrator> integrator(renderOptions->MakeIntegrator());
std::unique_ptr<Scene> scene(renderOptions->MakeScene());
if (scene && integrator) {
Debug("[start render scene.]");
integrator->RenderScene(*scene);
}

graphicsState = GraphicsState();
currentApiState = APIState::OptionsBlock;

for (int i = 0; i < MaxTransforms; ++i) {
curTransform[i] = Transform();
}
activeTransformBits = AllTransformsBits;
namedCoordinateSystems.erase(namedCoordinateSystems.begin(),
namedCoordinateSystems.end());
}

