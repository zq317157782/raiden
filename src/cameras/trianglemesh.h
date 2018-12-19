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
class TriangeMeshCamera :public Camera {
private:
	//相机空间和光栅化空间的差分
	std::vector<std::shared_ptr<Shape>> _mesh;
public:
	TriangeMeshCamera(const Transform& c2w, bool reverseOrientation,
	int nTriangles, const int *vertexIndices, int nVertices, const Point3f *p,
	const Vector3f *s, const Normal3f *n, const Point2f *uv, Float shutterOpen, Float shutterEnd, Film * f, const Medium* medium):Camera(c2w, shutterOpen, shutterEnd,f, medium){
		_mesh=CreateTriangleMesh(&c2w,&Inverse(c2w),reverseOrientation,nTriangles,vertexIndices,nVertices,p,s,n,uv);
	}
	
    virtual Float GenerateRay(const CameraSample &sample, Ray *ray) const override {
		//计算uv坐标
        Float v =  sample.pFilm.y / (film->fullResolution.y);
		Float u =  sample.pFilm.x / (film->fullResolution.x);
		Point2f uv(u,v);
		UVInteraction hit;
		bool flag=false;
	 	for(int i=0;i<_mesh.size();++i){
			 flag=_mesh[i]->UVToWorld(uv,&hit);
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
