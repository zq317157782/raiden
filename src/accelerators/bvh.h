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
class PrimitiveInfo{
public:
	int index;
	Bound3f bound;
	Point3f  centroid;//bound的中心
public:
	PrimitiveInfo(){}
	PrimitiveInfo(int i,const Bound3f& b):index(i),bound(b),centroid(b.minPoint*0.5+b.maxPoint*0.5){
	}
};

class BVHAccelerator:public Aggregate{
private:
	std::vector<std::shared_ptr<Primitive>> _primitives;

public:
	BVHAccelerator(const std::vector<std::shared_ptr<Primitive>>& primitives):_primitives(primitives){
		//生成图元生成需要的部分信息
		std::vector<PrimitiveInfo> primitiveInfos(_primitives.size());
		for(int i=0;i<_primitives.size();++i){
			primitiveInfos[i]=PrimitiveInfo(i,_primitives[i]->WorldBound());
		}
	}
};


#endif /* SRC_ACCELERATORS_BVH_H_ */
