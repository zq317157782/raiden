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
	Float _widths[3]; //宽度每体素
	Float _invWidths[3]; //单位宽度下的体素个数
	int _voxelNum[3]; //每个轴下的体素个数
	int _totalVoxelNum;
	Voxel** _voxels; //体素集合
	MemoryArena _voxelArena;
private:
	int PosToVoxel(const Point3f& p, int axis) const {
		int ret = (int) (p[axis] - _worldBound.minPoint[axis])
				* _invWidths[axis];
		return Clamp(ret, 0, _voxelNum[axis] - 1);
	}

	Float VoxelToPos(int voxelPos, int axis) const {
		return _worldBound.minPoint[axis] + voxelPos * _widths[axis];
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

		Vector3f diagonal = _worldBound.Diagonal();
		int maxAxis = MaxDimension(diagonal);
		Float voxelPerUnit = (std::pow(_primitives.size(), 1.0f / 3.0f) * 3)
				/ diagonal[maxAxis];

		//计算每个轴下的体素个数
		for (int axis = 0; axis < 3; ++axis) {
			_voxelNum[axis] = (int) std::round(voxelPerUnit * diagonal[axis]);
			_voxelNum[axis] = Clamp(_voxelNum[axis], 1, 64); //裁剪在1~64的范围内
			_widths[axis] = diagonal[axis] / _voxelNum[axis];
			_invWidths[axis] =
					(_widths[axis] == 0) ? (0) : (1.0f / _widths[axis]);
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

			for (int z = minV[2]; z <= maxV[2]; ++z) {
				for (int y = minV[1]; y <= maxV[1]; ++y) {
					for (int x = minV[0]; x <= maxV[0]; ++x) {
						int o = Offset(x, y, z);
						if (_voxels[o] == nullptr) {
							_voxels[o] = _voxelArena.Alloc<Voxel>();
						}
						(*_voxels[o]).AddPrimitive(_primitives[i]);
					}
				}
			}
		}
	}
	~Grid() {
		_voxelArena.Reset();
		FreeAligned(_voxels);
	}

	bool Intersect(const Ray& r, SurfaceInteraction* ref) const override {
		Float hit;
		if (Inside(r.o, _worldBound)) {
			hit = 0;
		} else if (!_worldBound.IntersectP(r, &hit)) {
			return false;
		}
		Point3f hitPoint = r(hit); //和grid的相交点

		Float nextCrossingT[3];
		Float deltaT[3];
		int step[3], out[3], pos[3];

		//初始化
		for (int axis = 0; axis < 3; ++axis) {
			pos[axis] = PosToVoxel(hitPoint, axis); //获得当前voxel的坐标成分
			if (r.d[axis] >= 0) {
				//计算向正方向偏移到下个voxel和voxel相交处的射线参数
				nextCrossingT[axis] = hit
						+ (VoxelToPos(pos[axis] + 1, axis) - hitPoint[axis])
								/ r.d[axis];
				step[axis] = 1; //向正方向的标记
				out[axis] = _voxelNum[axis]; //到第几个voxel结束
				deltaT[axis] = _widths[axis] / r.d[axis];
			} else {
				nextCrossingT[axis] = hit
						+ (VoxelToPos(pos[axis], axis) - hitPoint[axis])
								/ r.d[axis];
				step[axis] = 1; //向负方向的标记
				out[axis] = -1;
				deltaT[axis] = -_widths[axis] / r.d[axis];
			}
		}

		bool isHit = false;
		for (;;) {
			//获取当前voxel
			Voxel* voxel = _voxels[Offset(pos[0], pos[1], pos[2])];
			if (voxel) {
				isHit |= voxel->Intersect(r, ref);
				int axis;
				if (nextCrossingT[0] < nextCrossingT[1]
						&& nextCrossingT[0] < nextCrossingT[2]) {
					axis = 0;
				} else if (nextCrossingT[1] < nextCrossingT[2]) {
					axis = 1;
				} else {
					axis = 2;
				}
				if (r.tMax < nextCrossingT[axis]) {
					break;
				}
				pos[axis] += step[axis]; //更新位置
				if (pos[axis] == out[axis]) {
					break;
				}
				nextCrossingT[axis] += deltaT[axis];
			}
			return isHit;
		}
		//开始遍历

	}
//	bool Intersect(const Ray& r, SurfaceInteraction* ref) const override {
//		Float hit;
//		Float hitFar;
//		if (Inside(r.o, _worldBound)) {
//			hit = 0;
//		} else if (!_worldBound.IntersectP(r, &hit)) {
//			return false;
//		}
//
//		Point3f hitPoint = r(hit); //和grid的相交点
//		int vp[3];
//		for (int axis = 0; axis < 3; ++axis) {
//			vp[axis] = PosToVoxel(hitPoint, axis);
//		}
//		//获取当前相交点所在voxel;
//		for (;;) {
//			//Debug("start");
//			Voxel* voxel = _voxels[Offset(vp[0], vp[1], vp[2])];
//			if (voxel&&voxel->Intersect(r, ref)) {
//				return true;
//			} else {
//
//				//return false;
//				Point3f minPoint = Point3f(VoxelToPos(vp[0], 0),
//						VoxelToPos(vp[1], 1), VoxelToPos(vp[2], 2));
//				Point3f maxPoint = Point3f(VoxelToPos(vp[0] + 1, 0),
//						VoxelToPos(vp[1] + 1, 1), VoxelToPos(vp[2] + 1, 2));
//				Bound3f vb(minPoint, maxPoint);
//				vb.IntersectP(r, &hit, &hitFar);
//				hitPoint = r(hitFar); //更新新的voxel点
//				int vpTemp[3];
//				for (int axis = 0; axis < 3; ++axis) {
//					vpTemp[axis] = PosToVoxel(hitPoint, axis);
//				}
//				if (vpTemp[0] == vp[0] && vpTemp[1] == vp[1]
//						&& vpTemp[2] == vp[2]) {
//					return false;
//				}
//				for (int axis = 0; axis < 3; ++axis) {
//					vp[axis]=vpTemp[axis];
//					//Debug(axis<<":"<<vp[axis])
//				}
//			}
//		}
//	}
	Bound3f WorldBound() const override {
		return _worldBound;
	}
	bool IntersectP(const Ray& r) const override {
		Float hit;
		if (Inside(r.o, _worldBound)) {
			hit = 0;
		} else if (!_worldBound.IntersectP(r, &hit)) {
			return false;
		}
		Point3f hitPoint = r(hit); //和grid的相交点

		Float nextCrossingT[3];
		Float deltaT[3];
		int step[3], out[3], pos[3];

		//初始化
		for (int axis = 0; axis < 3; ++axis) {
			pos[axis] = PosToVoxel(hitPoint, axis); //获得当前voxel的坐标成分
			if (r.d[axis] >= 0) {
				//计算向正方向偏移到下个voxel和voxel相交处的射线参数
				nextCrossingT[axis] = hit
						+ (VoxelToPos(pos[axis] + 1, axis) - hitPoint[axis])
								/ r.d[axis];
				step[axis] = 1; //向正方向的标记
				out[axis] = _voxelNum[axis]; //到第几个voxel结束
				deltaT[axis] = _widths[axis] / r.d[axis];
			} else {
				nextCrossingT[axis] = hit
						+ (VoxelToPos(pos[axis], axis) - hitPoint[axis])
								/ r.d[axis];
				step[axis] = 1; //向负方向的标记
				out[axis] = -1;
				deltaT[axis] = -_widths[axis] / r.d[axis];
			}
		}

		for (;;) {
			//获取当前voxel
			Voxel* voxel = _voxels[Offset(pos[0], pos[1], pos[2])];
			if (voxel) {
				if (voxel->IntersectP(r)) {
					return true;
				}
				int axis;
				if (nextCrossingT[0] < nextCrossingT[1]
						&& nextCrossingT[0] < nextCrossingT[2]) {
					axis = 0;
				} else if (nextCrossingT[1] < nextCrossingT[2]) {
					axis = 1;
				} else {
					axis = 2;
				}
				if (r.tMax < nextCrossingT[axis]) {
					break;
				}
				pos[axis] += step[axis]; //更新位置
				if (pos[axis] == out[axis]) {
					break;
				}
				nextCrossingT[axis] += deltaT[axis];
			}
			return false;
		}
	}
}
;

std::shared_ptr<Grid> CreateGridAccelerator(
		const std::vector<std::shared_ptr<Primitive>> &prims,
		const ParamSet &ps);

#endif /* SRC_ACCELERATORS_GRID_H_ */
