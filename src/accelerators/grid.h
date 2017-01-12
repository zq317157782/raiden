/*
 * grid.h
 *
 *  Created on: 2017年1月12日
 *      Author: zhuqian
 */

#ifndef SRC_ACCELERATORS_GRID_H_
#define SRC_ACCELERATORS_GRID_H_
#include "raiden.h"
#include "primitive.h"
#include "geometry.h"
#include "memory.h"

//体素结构
struct Voxel {
private:
	std::vector<std::shared_ptr<Primitive>> _primitives;
public:
	void AddPrimitive(const std::shared_ptr<Primitive>& p) {
		_primitives.push_back(p);
	}
	bool Intersect(const Ray& r, SurfaceInteraction* ref) const {
		bool hit = false;
		for (int i = 0; i < _primitives.size(); ++i) {
			if (_primitives[i]->Intersect(r, ref)) {
				hit = true;
			}
		}
		return hit;
	}
	bool IntersectP(const Ray& r) const {
		for (int i = 0; i < _primitives.size(); ++i) {
			if (_primitives[i]->IntersectP(r)) {
				return true;
			}
		}
	}

};

class Grid: public Aggregate {
private:
	std::vector<std::shared_ptr<Primitive>> _primitives;
	Bound3f _worldBound;
	Float _voxelPerUnit; //每个空间单位下体素个数
	Float _unitPerVoxel;
	int _voxelNum[3]; //每个轴下的体素个数
	int _totalVoxelNum;
	Voxel** _voxels; //体素集合
	MemoryArena _voxelArena;
private:
	int PosToVoxel(const Point3f& p, int axis) const {
		int ret = (int) (p[axis] - _worldBound.minPoint[axis]) * _voxelPerUnit;
		return Clamp(ret, 0, _voxelNum[axis] - 1);
	}

	Float VoxelToPos(int voxelPos, int axis) const {
		return _worldBound.minPoint[axis] + voxelPos * _unitPerVoxel;
	}
	//偏移到相应的voxel
	int Offset(int x, int y, int z) const {
		return z * _voxelNum[0] * _voxelNum[1] + y * _voxelNum[0] + x;
	}
public:
	Grid(const std::vector<std::shared_ptr<Primitive>>& primitives,
			int maxWidth = 64) :
			_primitives(primitives) {



		//获取整个grid的空间AABB大小
		for (auto& p : _primitives) {
			_worldBound = Union(_worldBound, p->WorldBound());
		}

		maxWidth = std::min((int)std::pow(_primitives.size(), 1.0f / 3.0f),maxWidth);

		Vector3f diagonal = _worldBound.Diagonal();
		int maxAxis = MaxDimension(diagonal);
		_voxelPerUnit = maxWidth / diagonal[maxAxis];
		_unitPerVoxel = 1 / _voxelPerUnit;
		//计算每个轴下的体素个数
		for (int axis = 0; axis < 3; ++axis) {
			_voxelNum[axis] =  std::ceil(_voxelPerUnit * diagonal[axis]);
		}
		//总共的体素个数
		_totalVoxelNum = _voxelNum[0] * _voxelNum[1] * _voxelNum[2];
		//为体素分配空间
		_voxels = AllocAligned<Voxel*>(_totalVoxelNum);
		memset(_voxels, 0, _totalVoxelNum * sizeof(Voxel*));
		for (int i = 0; i < _primitives.size(); ++i) {
			const Bound3f& b = _primitives[i]->WorldBound();
			int minV[3];
			int maxV[3];
			//计算得到图元覆盖的voxel范围
			for (int j = 0; j < 3; ++j) {
				minV[j] = PosToVoxel(b.minPoint, j);
				maxV[j] = PosToVoxel(b.maxPoint, j);
			}

			for (int x = minV[0]; x <= maxV[0]; ++x) {
				for (int y = minV[1]; y <= maxV[1]; ++y) {
					for (int z = minV[2]; z <= maxV[2]; ++z) {
						int o = Offset(x, y, z);
						if (_voxels[o] == nullptr) {
							_voxels[o] = _voxelArena.Alloc<Voxel>();
						}
						(*_voxels[o]).AddPrimitive(_primitives[i]);
						
					}
				}
			}
		}
//		for(int i=0;i<_totalVoxelNum;++i){
//			if(_voxels[i]){
//				Debug("id:"<<i<<" num:"<<_voxels[i]->primitives.size());
//			}
//		}
	}
	~Grid() {
		_voxelArena.Reset();
		FreeAligned(_voxels);
	}
	bool Intersect(const Ray& r, SurfaceInteraction* ref) const override {
		Float hit;
		Float hitFar;
		if (Inside(r.o, _worldBound)) {
			hit = 0;
		} else if (!_worldBound.IntersectP(r, &hit)) {
			return false;
		}

		Point3f hitPoint = r(hit); //和grid的相交点
		int vp[3];
		for (int axis = 0; axis < 3; ++axis) {
			vp[axis] = PosToVoxel(hitPoint, axis);
		}
		//获取当前相交点所在voxel;
		for (;;) {
			//Debug("start");
			Voxel* voxel = _voxels[Offset(vp[0], vp[1], vp[2])];
			if (voxel&&voxel->Intersect(r, ref)) {
				return true;
			} else {

				//return false;
				Point3f minPoint = Point3f(VoxelToPos(vp[0], 0),
						VoxelToPos(vp[1], 1), VoxelToPos(vp[2], 2));
				Point3f maxPoint = Point3f(VoxelToPos(vp[0] + 1, 0),
						VoxelToPos(vp[1] + 1, 1), VoxelToPos(vp[2] + 1, 2));
				Bound3f vb(minPoint, maxPoint);
				vb.IntersectP(r, &hit, &hitFar);
				hitPoint = r(hitFar); //更新新的voxel点
				int vpTemp[3];
				for (int axis = 0; axis < 3; ++axis) {
					vpTemp[axis] = PosToVoxel(hitPoint, axis);
				}
				if (vpTemp[0] == vp[0] && vpTemp[1] == vp[1]
						&& vpTemp[2] == vp[2]) {
					return false;
				}
				for (int axis = 0; axis < 3; ++axis) {
					vp[axis]=vpTemp[axis];
					//Debug(axis<<":"<<vp[axis])
				}
			}
		}
	}
	Bound3f WorldBound() const override {
		return _worldBound;
	}
	bool IntersectP(const Ray& r) const override {
		return false;
	}
};

std::shared_ptr<Grid> CreateGridAccelerator(
		const std::vector<std::shared_ptr<Primitive>> &prims,
		const ParamSet &ps);

#endif /* SRC_ACCELERATORS_GRID_H_ */
