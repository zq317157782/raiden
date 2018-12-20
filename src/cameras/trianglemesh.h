/*
* cameras/shape.h
*
*  Created on: 2018年12月19日
*      Author: zhuqian
*/
#pragma once
#ifndef SRC_CAMERAS_SHAPE_H_
#define SRC_CAMERAS_SHAPE_H_

#include "raiden.h"
#include "camera.h"
#include "film.h"
#include "shapes/trianglemesh.h"
#include "geometry.h"

class UVQuadTreeNode{
public:
	bool isLeaf;//是否是叶节点
	Bound2f rect;
	std::shared_ptr<UVQuadTreeNode> childs[4];
	std::vector<std::shared_ptr<Triangle>> data;

	UVQuadTreeNode(const Bound2f& rect){
        this->rect=rect;
        isLeaf=true;
    }
};

class UVQuadTree{
	private:
		std::shared_ptr<UVQuadTreeNode> _root;
		uint8_t _capacity;
        uint8_t _maxDepth;
	public:
		UVQuadTree(const Bound2f& rect,uint8_t capacity,uint8_t maxDepth):_capacity(capacity),_maxDepth(maxDepth){
         Assert(capacity>=1);
         Assert(maxDepth>=1);
         _root=std::make_shared<UVQuadTreeNode>(rect);
     }
	 
     std::vector<std::shared_ptr<Triangle>> Lookup(const Point2f& uv) const{
		  std::vector<std::shared_ptr<Triangle>> list;
		  Lookup(uv,_root,list);
		  return list;
	 }
     void Lookup(const Point2f& uv,const std::shared_ptr<UVQuadTreeNode>& node,std::vector<std::shared_ptr<Triangle>>& list) const{
		 if(!Inside(uv,node->rect)){
			 return;
		 }
		 if(node->isLeaf==true){
			 for(size_t i=0;i<node->data.size();++i){
				 list.push_back(node->data[i]);
				// LInfo<<list.size();
			 }
			
		 }
		 else{
			    
				Lookup(uv,node->childs[0],list);
				Lookup(uv,node->childs[1],list);
				Lookup(uv,node->childs[2],list);
				Lookup(uv,node->childs[3],list);
		}
	 }


	 bool Build(const std::vector<std::shared_ptr<Triangle>>& meshs){
		  _root->isLeaf=true;
		  _root->data.clear();
		  bool flag=false;
		  for(size_t i=0;i<meshs.size();++i){
			  flag|=Insert(meshs[i],_root,0);
		  }
		  return flag;
	 }

	 bool Insert(const std::shared_ptr<Triangle>& mesh,const std::shared_ptr<UVQuadTreeNode>& node,int depth){
		 //首先判断是否是叶子节点
		 if(node->isLeaf==true){
			 //再判断叶子中的数据是否到达最大值
			 //或者深度超过最大深度的话，直接加入数据到节点
			 if((node->data.size()<_capacity)||(depth>_maxDepth)){
				 if(Overlap(mesh->UVBound(),node->rect)){
					 	node->data.push_back(mesh);
						//LInfo<<depth;
						return true;
				 }
				 return false;
			 }else{
				 //转化成中间节点
				 node->isLeaf=false;
				 Point2f midPoint=(node->rect.minPoint+node->rect.maxPoint)*0.5;
				 //| b2|b3 |
				 //+---+---+
				 //| b0|b1 |
				 Bound2f b0(node->rect.Corner(0),midPoint);
				 Bound2f b1(node->rect.Corner(1),midPoint);
				 Bound2f b2(node->rect.Corner(2),midPoint);
				 Bound2f b3(node->rect.Corner(3),midPoint);

				 node->childs[0]=std::make_shared<UVQuadTreeNode>(b0);
				 node->childs[1]=std::make_shared<UVQuadTreeNode>(b1);
				 node->childs[2]=std::make_shared<UVQuadTreeNode>(b2);
				 node->childs[3]=std::make_shared<UVQuadTreeNode>(b3);
				 
				 //把数据插入到子节点中
				 for(size_t i=0;i<node->data.size();++i){
					 Insert(node->data[i],node->childs[0],depth+1);
					 Insert(node->data[i],node->childs[1],depth+1);
					 Insert(node->data[i],node->childs[2],depth+1);
					 Insert(node->data[i],node->childs[3],depth+1);
				 }
				 //清除中间节点的数据
				 node->data.clear();
			 }
		 }

		 //往子节点加数据
		 bool flag=false;
		 flag|=Insert(mesh,node->childs[0],depth+1);
		 flag|=Insert(mesh,node->childs[1],depth+1);
		 flag|=Insert(mesh,node->childs[2],depth+1);
		 flag|=Insert(mesh,node->childs[3],depth+1);
		 return flag; 
	 }
};

class TriangeMeshCamera :public Camera {
private:
	//相机空间和光栅化空间的差分
	std::vector<std::shared_ptr<Triangle>> _mesh;
	std::shared_ptr<UVQuadTree> _tree;
public:
	TriangeMeshCamera(const Transform& c2w, bool reverseOrientation,
	int nTriangles, const int *vertexIndices, int nVertices, const Point3f *p,
	const Vector3f *s, const Normal3f *n, const Point2f *uv,Float shutterOpen, Float shutterEnd, Film * f, const Medium* medium):Camera(c2w, shutterOpen, shutterEnd,f, medium){
		_mesh=CreateTriangleMesh2(&c2w,&Inverse(c2w),reverseOrientation,nTriangles,vertexIndices,nVertices,p,s,n,uv);
		_tree=std::make_shared<UVQuadTree>(Bound2f(Point2f(0,0),Point2f(1,1)),4,6);
		_tree->Build(_mesh);
	}
	
    virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const override {
		//计算uv坐标
        Float v =  sample.pFilm.y / (film->fullResolution.y);
		Float u =  sample.pFilm.x / (film->fullResolution.x);
		Point2f uv(u,v);
		UVInteraction hit;
		bool flag=false;

		auto meshList=_tree->Lookup(uv);
	
	 	for(size_t i=0;i<meshList.size();++i){
			flag=meshList[i]->UVToWorld(uv,&hit);
			if(flag==true){
				break;
			}
		}
		if(flag==true){
			Vector3f dir=(Vector3f)Normalize(hit.n);
			*ray = Ray(hit.p,dir,Infinity, Lerp(sample.time, shutterOpen, shutterEnd));
			ray->medium = medium;
			return 1;
		}
		else {
			Vector3f dir=Vector3f(0,0,0);
			*ray = Ray(Point3f(0,0,0),dir,Infinity, Lerp(sample.time, shutterOpen, shutterEnd));
			ray->medium = medium;
			return 0;
		}
	}
};


TriangeMeshCamera *CreateTriangleMeshCamera(const ParamSet &params,
	const Transform &cam2world,
	Film *film, const Medium *medium);
#endif /* SRC_CAMERAS_ENVERONMENT_H_ */
