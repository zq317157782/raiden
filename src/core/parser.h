/*
 * apilua.h
 *
 *  Created on: 2016年12月19日
 *      Author: zhuqian
 */

#ifndef SRC_CORE_APILUA_H_
#define SRC_CORE_APILUA_H_
#include "paramset.h"
#include "raiden.h"
#include "api.h"
#include "lua/lua.hpp"
extern "C" {
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

static lua_State *L;

#define PARAM_TYPR_WRONG(x) lua_getglobal(L, "debug");\
							lua_getfield(L, -1, "traceback");\
int iError = lua_pcall( L,0,1,0);\
Error("[wrong param]"<<x);\
Error(lua_tostring(L, -1));\
exit(1);

static void Init(lua_State *L) {
	Options options;
	if (lua_isinteger(L, 1)) {
		int nThreads = lua_tointeger(L, 1);
		Assert(nThreads >= 0);
		options.numThread = nThreads;
	} else {
		PARAM_TYPR_WRONG("")
	}
	if (lua_isstring(L, 2)) {
		options.imageFile = lua_tostring(L, 2);
	} else {
		PARAM_TYPR_WRONG("")
	}
	raidenInit(options);
}

static void CleanUp(lua_State *L) {
	raidenCleanup();
}

static void WorldBegin(lua_State *L) {
	raidenWorldBegin();
}

static void WorldEnd(lua_State *L) {
	raidenWorldEnd();
}

static void Translate(lua_State *L) {
	Float dx, dy, dz;
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
		dx = lua_tonumber(L, 1);
		dy = lua_tonumber(L, 2);
		dz = lua_tonumber(L, 3);
	} else {
		PARAM_TYPR_WRONG("")
	}
	raidenTranslate(dx, dy, dz);
}

static void Rotate(lua_State *L) {
	Float angle, dx, dy, dz;
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)
			&& lua_isnumber(L, 4)) {
		angle = lua_tonumber(L, 1);
		dx = lua_tonumber(L, 2);
		dy = lua_tonumber(L, 3);
		dz = lua_tonumber(L, 4);
	} else {
		PARAM_TYPR_WRONG("")
	}
	raidenRotate(angle, dx, dy, dz);
}

static void Scale(lua_State *L) {
	Float sx, sy, sz;
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3)) {
		sx = lua_tonumber(L, 1);
		sy = lua_tonumber(L, 2);
		sz = lua_tonumber(L, 3);
	} else {
		PARAM_TYPR_WRONG("")
	}
	raidenScale(sx, sy, sz);
}

static int CoordinateSystem(lua_State *L) {
	std::string name;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L, 1);
	} else {
		PARAM_TYPR_WRONG("")
	}
	raidenCoordinateSystem(name);
}

static int CoordSysTransform(lua_State *L) {
	std::string name;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L, 1);
	} else {
		PARAM_TYPR_WRONG("")
	}
	raidenCoordSysTransform(name);
}
static int ActiveTransformAll(lua_State *L) {
	raidenActiveTransformAll();
	return LUA_OK;
}

static int ActiveTransformEndTime(lua_State *L) {
	raidenActiveTransformEndTime();
	return LUA_OK;
}

static int ActiveTransformStartTime(lua_State *L) {
	raidenActiveTransformStartTime();
	return LUA_OK;
}

static int TransformTimes(lua_State *L) {
	Float start, end;
	if (lua_isnumber(L, 1) && lua_isnumber(L, 2)) {
		start = lua_tonumber(L, 1);
		end = lua_tonumber(L, 2);
	} else {
		PARAM_TYPR_WRONG("")
	}
	raidenTransformTimes(start, end);
}

static void ParsePoint2f(ParamSet& set, const std::string& name) {
	lua_len(L, -1); //长度入栈
	int size = lua_tointeger(L, -1);
	if (size % 2 != 0) {
		PARAM_TYPR_WRONG("point2f[]'s size need align 2");
	}
	size = size / 2;
	lua_pop(L, 1); //长度出栈
	std::unique_ptr<Point2f[]> points(new Point2f[size]);
	for (int i = 0; i < size; ++i) {
		lua_geti(L, -1, 2 * i + 1);
		points[i].x = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_geti(L, -1, 2 * i + 2);
		points[i].y = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	set.AddPoint2f(name, std::move(points), size);
}

static void ParseVector2f(ParamSet& set, const std::string& name) {
	lua_len(L, -1); //长度入栈
	int size = lua_tointeger(L, -1);
	if (size % 2 != 0) {
		PARAM_TYPR_WRONG("Vector2f[]'s size need align 2");
	}
	size = size / 2;
	lua_pop(L, 1); //长度出栈
	std::unique_ptr<Vector2f[]> vectors(new Vector2f[size]);
	for (int i = 0; i < size; ++i) {
		lua_geti(L, -1, 2 * i + 1);
		vectors[i].x = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_geti(L, -1, 2 * i + 2);
		vectors[i].y = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	set.AddVector2f(name, std::move(vectors), size);
}

static void ParsePoint3f(ParamSet& set, const std::string& name) {
	lua_len(L, -1); //长度入栈
	int size = lua_tointeger(L, -1);
	if (size % 3 != 0) {
		PARAM_TYPR_WRONG("point3f[]'s size need align 3");
	}
	size = size / 3;
	lua_pop(L, 1); //长度出栈
	std::unique_ptr<Point3f[]> points(new Point3f[size]);
	for (int i = 0; i < size; ++i) {
		lua_geti(L, -1, 3 * i + 1);
		points[i].x = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_geti(L, -1, 3 * i + 2);
		points[i].y = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_geti(L, -1, 3 * i + 3);
		points[i].z = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	set.AddPoint3f(name, std::move(points), size);
}

static void ParseVector3f(ParamSet& set, const std::string& name) {
	lua_len(L, -1); //长度入栈
	int size = lua_tointeger(L, -1);
	if (size % 3 != 0) {
		PARAM_TYPR_WRONG("Vector3f[]'s size need align 3");
	}
	size = size / 3;
	lua_pop(L, 1); //长度出栈
	std::unique_ptr<Vector3f[]> vectors(new Vector3f[size]);
	for (int i = 0; i < size; ++i) {
		lua_geti(L, -1, 3 * i + 1);
		vectors[i].x = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_geti(L, -1, 3 * i + 2);
		vectors[i].y = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_geti(L, -1, 3 * i + 3);
		vectors[i].z = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	set.AddVector3f(name, std::move(vectors), size);
}

static void ParseNormal3f(ParamSet& set, const std::string& name) {
	lua_len(L, -1); //长度入栈
	int size = lua_tointeger(L, -1);
	if (size % 3 != 0) {
		PARAM_TYPR_WRONG("Normal3f[]'s size need align 3");
	}
	size = size / 3;
	lua_pop(L, 1); //长度出栈
	std::unique_ptr<Normal3f[]> normals(new Normal3f[size]);
	for (int i = 0; i < size; ++i) {
		lua_geti(L, -1, 3 * i + 1);
		normals[i].x = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_geti(L, -1, 3 * i + 2);
		normals[i].y = lua_tonumber(L, -1);
		lua_pop(L, 1);
		lua_geti(L, -1, 3 * i + 3);
		normals[i].z = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	set.AddNormal3f(name, std::move(normals), size);
}

static void ParseRGB(ParamSet& set, const std::string& name) {
	lua_len(L, -1); //长度入栈
	int size = lua_tointeger(L, -1);
	if (size % 3 != 0) {
		PARAM_TYPR_WRONG("RGB[]'s size need align 3");
	}
	lua_pop(L, 1); //长度出栈
	std::unique_ptr<Float[]> rgbs(new Float[size]);
	for (int i = 0; i < size; i = i + 3) {
		lua_geti(L, -1, i + 1);
		rgbs[i] = lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_geti(L, -1, i + 2);
		rgbs[i + 1] = lua_tonumber(L, -1);
		lua_pop(L, 1);

		lua_geti(L, -1, i + 3);
		rgbs[i + 2] = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	set.AddRGBSpectrum(name, std::move(rgbs), size);
}

static void ParseString(ParamSet& set, const std::string& name) {
	lua_len(L, -1); //长度入栈
	int size = lua_tointeger(L, -1);
	lua_pop(L, 1); //长度出栈
	std::unique_ptr<std::string[]> strings(new std::string[size]);
	for (int i = 0; i < size; ++i) {
		lua_geti(L, -1, i + 1);
		strings[i] = lua_tostring(L, -1);
		lua_pop(L, 1);
	}
	set.AddString(name, std::move(strings), size);
}

static void ParseFloatArray(ParamSet& set, const std::string& name) {
	lua_len(L, -1); //长度入栈
	int size = lua_tointeger(L, -1);
	lua_pop(L, 1); //长度出栈
	std::unique_ptr<Float[]> floats(new Float[size]);
	for (int i = 0; i < size; ++i) {
		lua_geti(L, -1, i + 1);
		floats[i] = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	set.AddFloat(name, std::move(floats), size);
}

//获取paramset
static ParamSet GetParamSet(lua_State* L, int index) {
	ParamSet set;
	int t = index;
	if (!lua_istable(L, t)) {
		PARAM_TYPR_WRONG("paramset must be a table");
	}
	lua_pushnil(L); //push 第一个key
	//遍历表
	while (lua_next(L, t) != 0) {
		std::string key = lua_tostring(L, -2);	//获取参数名字
		switch (lua_type(L, -1)) {
		case LUA_TNUMBER: {
			//int 类型
			if (lua_isinteger(L, -1)) {
				int v = lua_tointeger(L, -1);
				set.AddInt(key, std::unique_ptr<int[]>(new int[1] { v }), 1);
			} else {
				Float v = lua_tonumber(L, -1);
				set.AddFloat(key, std::unique_ptr<Float[]>(new Float[1] { v }),
						1);
			}
		}
			break;
			//bool
		case LUA_TBOOLEAN: {
			bool v = lua_toboolean(L, -1);
			set.AddBool(key, std::unique_ptr<bool[]>(new bool[1] { v }), 1);
		}
			break;

			//处理类型为表的情况，比较复杂
		case LUA_TTABLE: {
			lua_getfield(L, t, key.c_str());	//把子table压入栈顶!
			lua_getfield(L, -1, "type");	//type入栈
			std::string type = lua_tostring(L, -1);
			lua_pop(L, 1);	//type出栈
			lua_getfield(L, -1, "value"); //value入栈
			if (type == "point2f") {
				ParsePoint2f(set, key);
			} else if (type == "point3f") {
				ParsePoint3f(set, key);
			} else if (type == "vector2f") {
				ParseVector2f(set, key);
			} else if (type == "vector3f") {
				ParseVector3f(set, key);
			} else if (type == "normal3f") {
				ParseNormal3f(set, key);
			} else if (type == "string") {
				ParseString(set, key);
			} else if (type == "rgb") {
				ParseRGB(set, key);
			} else if (type == "float[]") {
				ParseFloatArray(set, key);
			} else {
				PARAM_TYPR_WRONG("unknow type");
			}
			lua_pop(L, 1); //value 出站
			lua_pop(L, 1); //把子table出栈!
		}
			break;
		}
		lua_pop(L, 1);
	}
	return set;
}

static int PixelFilter(lua_State* L) {
	std::string name;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L, 1);
	} else {
		PARAM_TYPR_WRONG("")
	}
	ParamSet params = GetParamSet(L, 2);
	raidenPixelFilter(name, params);
	return LUA_OK;
}
static int Film(lua_State* L) {
	std::string name;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L, 1);
	} else {
		PARAM_TYPR_WRONG("")
	}
	ParamSet params = GetParamSet(L, 2);
	raidenFilm(name, params);
	return LUA_OK;
}

static int Sampler(lua_State* L) {
	std::string name;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L, 1);
	} else {
		PARAM_TYPR_WRONG("")
	}
	ParamSet params = GetParamSet(L, 2);
	raidenSampler(name, params);
	return LUA_OK;
}
static int Accelerator(lua_State* L) {
	std::string name;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L, 1);
	} else {
		PARAM_TYPR_WRONG("")
	}
	ParamSet params = GetParamSet(L, 2);
	raidenAccelerator(name, params);
	return LUA_OK;
}
static int Integrator(lua_State* L) {
	std::string name;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L, 1);
	} else {
		PARAM_TYPR_WRONG("")
	}
	ParamSet params = GetParamSet(L, 2);
	raidenIntegrator(name, params);
	return LUA_OK;
}
static int Camera(lua_State* L) {
	std::string name;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L, 1);
	} else {
		PARAM_TYPR_WRONG("")
	}
	ParamSet params = GetParamSet(L, 2);
	raidenCamera(name, params);
	return LUA_OK;
}
static int Shape(lua_State* L) {
	std::string name;
	if (lua_isstring(L, 1)) {
		name = lua_tostring(L, 1);
	} else {
		PARAM_TYPR_WRONG("")
	}
	ParamSet params = GetParamSet(L, 2);
	raidenShape(name, params);
	return LUA_OK;
}

static int CreateTexture(lua_State* L) {
	std::string name;
	std::string type;
	std::string texName;
	if (lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isstring(L, 3)) {
		name = lua_tostring(L, 1);
		type = lua_tostring(L, 2);
		texName = lua_tostring(L, 3);
	} else {
		PARAM_TYPR_WRONG("")
	}
	ParamSet params = GetParamSet(L, 4);
	raidenTexture(name, type, texName, params);
}

static int AttributeBegin() {
	raidenAttributeBegin();
	return LUA_OK;
}

static int AttributeEnd() {
	raidenAttributeEnd();
	return LUA_OK;
}

static int TransformBegin() {
	raidenTransformBegin();
	return LUA_OK;
}

static int TransformEnd() {
	raidenTransformEnd();
	return LUA_OK;
}

void parse(char* filename) {
	int status, result;
	L = luaL_newstate(); /* create state */
	luaL_openlibs(L);
	lua_register(L, "Init", (lua_CFunction )Init);
	lua_register(L, "CleanUp", (lua_CFunction )CleanUp);
	lua_register(L, "WorldBegin", (lua_CFunction )WorldBegin);
	lua_register(L, "WorldEnd", (lua_CFunction )WorldEnd);
	lua_register(L, "Translate", (lua_CFunction )Translate);
	lua_register(L, "Rotate", (lua_CFunction )Rotate);
	lua_register(L, "Scale", (lua_CFunction )Scale);
	lua_register(L, "CoordinateSystem", (lua_CFunction )CoordinateSystem);
	lua_register(L, "CoordSysTransform", (lua_CFunction )CoordSysTransform);
	lua_register(L, "ActiveTransformAll", (lua_CFunction )ActiveTransformAll);
	lua_register(L, "ActiveTransformEndTime",
			(lua_CFunction )ActiveTransformEndTime);
	lua_register(L, "ActiveTransformStartTime",
			(lua_CFunction )ActiveTransformStartTime);
	lua_register(L, "TransformTimes", (lua_CFunction )TransformTimes);
	lua_register(L, "PixelFilter", (lua_CFunction )PixelFilter);
	lua_register(L, "Film", (lua_CFunction )Film);
	lua_register(L, "Sampler", (lua_CFunction )Sampler);
	lua_register(L, "Accelerator", (lua_CFunction )Accelerator);
	lua_register(L, "Integrator", (lua_CFunction )Integrator);
	lua_register(L, "Camera", (lua_CFunction )Camera);
	lua_register(L, "Shape", (lua_CFunction )Shape);
	lua_register(L, "Texture", (lua_CFunction )CreateTexture);
	lua_register(L, "AttributeBegin", (lua_CFunction )AttributeBegin);
	lua_register(L, "AttributeEnd", (lua_CFunction )AttributeEnd);
	lua_register(L, "TransformBegin", (lua_CFunction )TransformBegin);
	lua_register(L, "TransformEnd", (lua_CFunction )TransformEnd);
	int ret = luaL_dofile(L, filename);
	if (ret != LUA_OK) {
		lua_error(L);
	}
}

#endif /* SRC_CORE_APILUA_H_ */
