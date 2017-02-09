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

//代表在内存中的一个线性BVH节点
class LinearBVHNode {
public:
	Bound3f bound;//包围盒				24
	uint16_t numPrimitive;//图元的个数	26
	uint8_t axis;//                     27
	union {
		int primitiveOffset;//指向图元
		int secondChildOffset;
	};	//31
	uint8_t pad[1];
public:
};

//用于SAH的结构
struct BucketInfo {
	int count=0;//包含的个数
	Bound3f bound;
};

class BVHAccelerator:public Aggregate{
public:
	enum SplitMethod { MIDDLE, EQUAL_COUNT,SAH};
private:
	std::vector<std::shared_ptr<Primitive>> _primitives;
	LinearBVHNode* _nodes;
	SplitMethod _splitMethod;
	int _maxPrimitivesInNode;

	//递归生成BVHBuildNode
	BVHBuildNode* RecursiveBuild(MemoryArena& arena, std::vector<BVHPrimitiveInfo>& primitiveInfos,int start,int end,int* totalNodes, std::vector<std::shared_ptr<Primitive>>& orderedPrimitives) const {
		BVHBuildNode* node = arena.Alloc<BVHBuildNode>();
		(*totalNodes)++;
		Bound3f bound;
		for (int i = start; i < end; ++i) {
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
			for (int i = start; i < end; ++i) {
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
			
			switch (_splitMethod)
			{
			case SplitMethod::MIDDLE: 
			{
				Float cMid = (cBound.maxPoint[dim] + cBound.minPoint[dim])*0.5f;
				BVHPrimitiveInfo* midPtr = std::partition(&primitiveInfos[start], &primitiveInfos[end - 1] + 1, [cMid, dim](const BVHPrimitiveInfo& info) {
					return info.centroid[dim] < cMid;
				});
				mid = midPtr - &primitiveInfos[0];
				if (mid != start&&mid != end) {
					break;
				}
			}
			case SplitMethod::EQUAL_COUNT: {
				mid = (start + end)*0.5f;
				std::nth_element(&primitiveInfos[start], &primitiveInfos[mid], &primitiveInfos[end - 1] + 1, [dim](const BVHPrimitiveInfo& i1, const BVHPrimitiveInfo& i2) {
					return i1.centroid[dim] < i2.centroid[dim];
				});
				break;
			}

			case SplitMethod::SAH: {
				if (numPrimitive<=2) {
					mid = (start + end)*0.5f;
					std::nth_element(&primitiveInfos[start], &primitiveInfos[mid], &primitiveInfos[end - 1] + 1, [dim](const BVHPrimitiveInfo& i1, const BVHPrimitiveInfo& i2) {
						return i1.centroid[dim] < i2.centroid[dim];
					});
				}
				else {
					constexpr int numBucket = 12;
					BucketInfo buckets[numBucket];//PBRT使用了12个bucket
												  //初始化buckets
					for (int i = start; i < end; ++i) {
						int bucketIndex = numBucket*cBound.Offset(primitiveInfos[i].centroid)[dim];
						if (bucketIndex == numBucket) {
							bucketIndex = bucketIndex - 1;
						}
						buckets[bucketIndex].count++;
						buckets[bucketIndex].bound = Union(buckets[bucketIndex].bound, primitiveInfos[i].bound);
					}

					//计算每一次分割策略所花费的开销
					Float costs[numBucket - 1];
					for (int i = 0; i < numBucket - 1; ++i) {
						int count0 = 0, count1 = 0;
						Bound3f b0, b1;
						for (int j = 0; j < i + 1; ++j) {
							count0 += buckets[j].count;
							b0 = Union(b0, buckets[j].bound);
						}
						for (int j = i + 1; j < numBucket; ++j) {
							count1 += buckets[j].count;
							b1 = Union(b1, buckets[j].bound);
						}
						//计算SAH系数
						//pbrt的最新代码中已经把0.125的近似改成了1
						costs[i] =/*0.125f*/1.0f + (count0*b0.SurfaceArea() + count1*b1.SurfaceArea()) / bound.SurfaceArea();
					}

					//寻找最小的SAH系数
					Float minCost = costs[0];
					int minCostIndex = 0;
					for (int i = 1; i < numBucket - 1; ++i) {
						if (costs[i] < minCost) {
							minCost = costs[i];
							minCostIndex = i;
						}
					}

					Float leafCost = numPrimitive;//近似
					if (numPrimitive > _maxPrimitivesInNode || minCost < leafCost) {
						//继续分割
						BVHPrimitiveInfo* midPtr = std::partition(&primitiveInfos[start], &primitiveInfos[end - 1] + 1, [=](const BVHPrimitiveInfo& info) {
							//按照SAH系数进行分割
							int bucketIndex = numBucket*cBound.Offset(info.centroid)[dim];
							if (bucketIndex == numBucket) {
								bucketIndex = bucketIndex - 1;
							}
							return bucketIndex <= minCostIndex;
						});
						mid = midPtr - &primitiveInfos[0];
					}
					else {
						//创建子节点
						int firstOffset = orderedPrimitives.size();
						for (int i = start; i < end; ++i) {
							int index = primitiveInfos[i].index;
							orderedPrimitives.push_back(_primitives[index]);
						}
						node->InitLeaf(firstOffset, numPrimitive, bound);
						return node;
					}
				}
				break;
			} 

			}
			node->InitInterior(RecursiveBuild(arena, primitiveInfos, start, mid, totalNodes, orderedPrimitives), RecursiveBuild(arena, primitiveInfos, mid, end, totalNodes, orderedPrimitives), dim);

		}

		return node;
	}

	//让tree在内存中线性分布
	int FlattenBVHTree(BVHBuildNode* node,int *offset) {
		LinearBVHNode* linearNode = &_nodes[*offset];
		linearNode->bound = node->bound;
		int myOffset = (*offset)++;
		if (node->numPrimitives > 0) {
			linearNode->primitiveOffset = node->firstPrimitiveOffset;
			linearNode->numPrimitive = node->numPrimitives;
		}
		else {
			linearNode->numPrimitive = 0;
			linearNode->axis = node->splitAxis;
			FlattenBVHTree(node->children[0], offset);
			linearNode->secondChildOffset= FlattenBVHTree(node->children[1], offset);
		}
		return myOffset;
	}
public:
	

	BVHAccelerator(const std::vector<std::shared_ptr<Primitive>>& primitives,SplitMethod sm=SplitMethod::SAH, int mpin=1):_primitives(primitives),_splitMethod(sm), _maxPrimitivesInNode(mpin){
		//生成图元生成需要的部分信息
		std::vector<BVHPrimitiveInfo> primitiveInfos(_primitives.size());
		for(int i=0;i<_primitives.size();++i){
			primitiveInfos[i]=BVHPrimitiveInfo(i,_primitives[i]->WorldBound());
		}
		//生成build tree
		BVHBuildNode * root;
		int totalNodes=0;
		std::vector<std::shared_ptr<Primitive>> orderedPrimitives;
		MemoryArena arena(1024 * 1024);//用于为中间节点提供内存空间
		root=RecursiveBuild(arena, primitiveInfos, 0, primitiveInfos.size(), &totalNodes, orderedPrimitives);
		_primitives.swap(orderedPrimitives);
		//为实际线性树分配空间
		_nodes = AllocAligned<LinearBVHNode>(totalNodes);
		int offset = 0;
		FlattenBVHTree(root, &offset);
		arena.Reset();
	}

	bool Intersect(const Ray& r, SurfaceInteraction* ref) const override {
		bool hit = false;
		Vector3f invDir(1.0f / r.d.x, 1.0f / r.d.y, 1.0f / r.d.z);
		int dirIsNeg[3] = { invDir.x < 0,invDir.y < 0 ,invDir.z < 0 };

		int curNodeIndex = 0;//当前需要访问的顶点
		int nextNodeToVisitOffset = 0;
		int nextNodeToVisit[64];
		while (true) {
			LinearBVHNode* node = &_nodes[curNodeIndex];
			if (node->bound.IntersectP(r, invDir, dirIsNeg)) {
				//叶子节点
				if (node->numPrimitive > 0) {
					for (int i = 0; i < node->numPrimitive; ++i) {
						if (_primitives[node->primitiveOffset + i]->Intersect(r, ref)) {
							hit = true;
						}
					}
					if (nextNodeToVisitOffset == 0) {
						break;
					}
					curNodeIndex = nextNodeToVisit[--nextNodeToVisitOffset];
				}
				//中间节点
				else {
					if (dirIsNeg[node->axis]) {
						//先访问第二个节点
						nextNodeToVisit[nextNodeToVisitOffset++] = curNodeIndex + 1;
						curNodeIndex = node->secondChildOffset;
					}
					else {
						//先访问第一个节点
						nextNodeToVisit[nextNodeToVisitOffset++]= node->secondChildOffset;
						curNodeIndex = curNodeIndex + 1;
					}
				}
			}
			else {
				//没有需要继续测试的node
				if (nextNodeToVisitOffset == 0) {
					break;
				}
				curNodeIndex = nextNodeToVisit[--nextNodeToVisitOffset];
			}
		}
		return hit;
	}
	
	Bound3f WorldBound() const override {
		return _nodes[0].bound;
	}
	bool IntersectP(const Ray& r) const override {
		Vector3f invDir(1.0f / r.d.x, 1.0f / r.d.y, 1.0f / r.d.z);
		int dirIsNeg[3] = { invDir.x < 0,invDir.y < 0 ,invDir.z < 0 };

		int curNodeIndex = 0;//当前需要访问的顶点
		int nextNodeToVisitOffset = 0;
		int nextNodeToVisit[64];

		while (true) {
			LinearBVHNode* node = &_nodes[curNodeIndex];
			if (node->bound.IntersectP(r, invDir, dirIsNeg)) {
				//叶子节点
				if (node->numPrimitive > 0) {
					for (int i = 0; i < node->numPrimitive; ++i) {
						if (_primitives[node->primitiveOffset + i]->IntersectP(r)) {
							return true;
						}
					}
					if (nextNodeToVisitOffset == 0) {
						break;
					}
					curNodeIndex = nextNodeToVisit[--nextNodeToVisitOffset];
				}
				//中间节点
				else {
					if (dirIsNeg[node->axis]) {
						//先访问第二个节点
						nextNodeToVisit[nextNodeToVisitOffset++] = curNodeIndex + 1;
						curNodeIndex = node->secondChildOffset;
					}
					else {
						//先访问第一个节点
						nextNodeToVisit[nextNodeToVisitOffset++] = node->secondChildOffset;
						curNodeIndex = curNodeIndex + 1;
					}
				}
			}
			else {
				//没有需要继续测试的node
				if (nextNodeToVisitOffset == 0) {
					break;
				}
				curNodeIndex = nextNodeToVisit[--nextNodeToVisitOffset];
			}
		}
		return false;
	}

	~BVHAccelerator() {

	}
};


std::shared_ptr<BVHAccelerator> CreateBVHAccelerator(
	const std::vector<std::shared_ptr<Primitive>> &prims,
	const ParamSet &ps);
#endif /* SRC_ACCELERATORS_BVH_H_ */
