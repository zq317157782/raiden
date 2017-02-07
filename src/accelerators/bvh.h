/*
 * bvh.h
 *
 *  Created on: 2017年2月6日
 *      Author: zhuqian
 */

#ifndef SRC_ACCELERATORS_BVH_H_
#define SRC_ACCELERATORS_BVH_H_
#include "raiden.h"
#include "primitive.h"
#include "geometry.h"
#include "memory.h"
#include <algorithm>

//构建BVH树的过程中使用的节点结构
class BVHBuildNode{
public:
	Bound3f bound;//包含所有子节点包围盒的包围盒
	//如果节点是中间节点
	BVHBuildNode* children[2];//指向两个子节点
	int splitAxis;//分割子节点的坐标轴
	//如果节点是个叶子的情况
	int firstPrimitiveOffset;//指向primitives数组中的第一个图元
	int numPrimitives;//叶子包含的图元数
public:
	//初始化叶子
	void InitLeaf(int first,int num,const Bound3f& bound){
		firstPrimitiveOffset=first;
		numPrimitives=num;
		this->bound=bound;
		children[0]=children[1]=nullptr;
	}

	//初始化中间节点
	void InitInterior(BVHBuildNode* c1,BVHBuildNode* c2,int axis){
		children[0]=c1;
		children[1]=c2;
		bound=Union(c1->bound,c2->bound);
		splitAxis=axis;
	}
};

//代表一个图元的部分信息
class BVHPrimitiveInfo{
public:
	int index;
	Bound3f bound;
	Point3f  centroid;//bound的中心
public:
	BVHPrimitiveInfo(){}
	BVHPrimitiveInfo(int i,const Bound3f& b):index(i),bound(b),centroid(b.minPoint*0.5+b.maxPoint*0.5){
	}
};

class BVHAccelerator:public Aggregate{
private:
	std::vector<std::shared_ptr<Primitive>> _primitives;
	//递归生成BVHBuildNode
	BVHBuildNode* RecursiveBuild(MemoryArena& arena, std::vector<BVHPrimitiveInfo>& primitiveInfos,int start,int end,int* totalNodes, std::vector<std::shared_ptr<Primitive>>& orderedPrimitives) const {
		BVHBuildNode* node = arena.Alloc<BVHBuildNode>();
		*totalNodes++;
		Bound3f bound;
		for (int i = 0; i < primitiveInfos.size(); ++i) {
			bound = Union(bound, primitiveInfos[i].bound);
		}
		
		int numPrimitive = end - start;
		//生成叶子节点
		if (numPrimitive == 1) {
			int firstOffset = orderedPrimitives.size();
			for (int i = start; i < end; ++i) {
				int index = primitiveInfos[i].index;
				orderedPrimitives.push_back(_primitives[index]);
			}
			node->InitLeaf(firstOffset, numPrimitive, bound);
			return node;
		}
		//生成中间节点
		else {
			Bound3f cBound;
			for (int i = 0; i < primitiveInfos.size(); ++i) {
				cBound = Union(cBound, primitiveInfos[i].centroid);
			}
			//获取最大坐标轴
			int dim = cBound.MaximumExtent();
			int mid = (end - start)*0.5f;
			if (cBound.maxPoint[dim] == cBound.minPoint[dim]) {
				int firstOffset = orderedPrimitives.size();
				for (int i = start; i < end; ++i) {
					int index = primitiveInfos[i].index;
					orderedPrimitives.push_back(_primitives[index]);
				}
				node->InitLeaf(firstOffset, numPrimitive, bound);
				return node;
			}
			//这里确定是要生成中间节点了
			//这里默认使用middle,其他模式，以后会加入
			Float cMid = (cBound.maxPoint[dim] + cBound.minPoint[dim])*0.5f;
			BVHPrimitiveInfo* midPtr = std::partition(&primitiveInfos[start], &primitiveInfos[end - 1] + 1, [cMid,dim](const BVHPrimitiveInfo& info) {
				return info.centroid[dim] < cMid;
			});
			mid = midPtr - &primitiveInfos[0];
			if (mid == start||mid == end) {
				mid = (start + end) / 2;
			}
				node->InitInterior(RecursiveBuild(arena, primitiveInfos, start, mid, totalNodes, orderedPrimitives), RecursiveBuild(arena, primitiveInfos, mid, end, totalNodes, orderedPrimitives), dim);
			
		}

		return node;
	}
public:
	BVHAccelerator(const std::vector<std::shared_ptr<Primitive>>& primitives):_primitives(primitives){
		//生成图元生成需要的部分信息
		std::vector<BVHPrimitiveInfo> primitiveInfos(_primitives.size());
		for(int i=0;i<_primitives.size();++i){
			primitiveInfos[i]=BVHPrimitiveInfo(i,_primitives[i]->WorldBound());
		}
		//生成build tree
		BVHBuildNode * root;
		int totalNodes;
		std::vector<std::shared_ptr<Primitive>> orderedPrimitives;
		MemoryArena arena(1024 * 1024);//用于为中间节点提供内存空间
		root=RecursiveBuild(arena, primitiveInfos, 0, primitiveInfos.size(), &totalNodes, orderedPrimitives);
	}

	bool Intersect(const Ray& r, SurfaceInteraction* ref) const override {
		
		return false;
	}
	
	Bound3f WorldBound() const override {
		return Bound3f();
	}
	bool IntersectP(const Ray& r) const override {
		return false;
	}
};


std::shared_ptr<BVHAccelerator> CreateBVHAccelerator(
	const std::vector<std::shared_ptr<Primitive>> &prims,
	const ParamSet &ps);
#endif /* SRC_ACCELERATORS_BVH_H_ */
