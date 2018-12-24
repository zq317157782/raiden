#include "mesh.h"
#include "paramset.h"

MeshCamera *CreateMeshCamera(const ParamSet &params,
	const Transform &cam2world,
	Film *film, const Medium *medium){
	int nvi, npi, nuvi, nsi, nni;
	const int *vi = params.FindInt("indices", &nvi);
	const Point3f *P = params.FindPoint3f("P", &npi);
	const Point2f *uvs = params.FindPoint2f("uv", &nuvi);
	if (!uvs) uvs = params.FindPoint2f("st", &nuvi);
	std::vector<Point2f> tempUVs;
	if (!uvs) {
		const Float *fuv = params.FindFloat("uv", &nuvi);
		if (!fuv) fuv = params.FindFloat("st", &nuvi);
		if (fuv) {
			nuvi /= 2;
			tempUVs.reserve(nuvi);
			for (int i = 0; i < nuvi; ++i)
				tempUVs.push_back(Point2f(fuv[2 * i], fuv[2 * i + 1]));
			uvs = &tempUVs[0];
		}
	}
	if (uvs) {
		if (nuvi < npi) {
			LError<<
				"Not enough of \"uv\"s for triangle mesh.  Expected "<< npi <<", "
				"found "<< nuvi <<".  Discarding.";
			uvs = nullptr;
		}
		else if (nuvi > npi) {
			LWarning<<
				"More \"uv\"s provided than will be used for triangle "
				"mesh camera.  (" << npi << " expcted, " << nuvi << " found)";
		}
	}
	if (!vi) {
		LError<<
			"Vertex indices \"indices\" not provided with triangle mesh camera";
		return nullptr;
	}
	if (!P) {
		LError<<"Vertex positions \"P\" not provided with triangle mesh camera";
		return nullptr;
	}
	const Vector3f *S = params.FindVector3f("S", &nsi);
	if (S && nsi != npi) {
		LError<<"Number of \"S\"s for triangle mesh camera must match \"P\"s";
		S = nullptr;
	}
	const Normal3f *N = params.FindNormal3f("N", &nni);
	if (N && nni != npi) {
		LError<<"Number of \"N\"s for triangle mesh camera must match \"P\"s";
		N = nullptr;
	}
	for (int i = 0; i < nvi; ++i) {
		if (vi[i] >= npi) {
			LError<<
				"triangle mesh camera has out of-bounds vertex index " << vi[i] << " (" << npi << " \"P\" "
				"values were given)";
			return nullptr;
		}
	}
	
    Float shutteropen = params.FindOneFloat("shutteropen", 0.f);
	Float shutterclose = params.FindOneFloat("shutterclose", 1.f);
	if (shutterclose < shutteropen) {
		std::swap(shutterclose, shutteropen);
	}
	Float frame = params.FindOneFloat("frameaspectratio", Float(film->fullResolution.x) / Float(film->fullResolution.y));
	Bound2f screen;
	if (frame > 1.f) {
		screen.minPoint.x = -frame;
		screen.maxPoint.x = frame;
		screen.minPoint.y = -1.0f;
		screen.maxPoint.y = 1.0f;
	}
	else {
		screen.minPoint.x = -1.0f;
		screen.maxPoint.x = 1.0f;
		screen.minPoint.y = -1.0f / frame;
		screen.maxPoint.y = 1.0f / frame;
	}
	int swi;
	const Float *sw = params.FindFloat("screenwindow", &swi);
	if (sw) {
		if (swi == 4) {
			screen.minPoint.x = sw[0];
			screen.maxPoint.x = sw[1];
			screen.minPoint.y = sw[2];
			screen.maxPoint.y = sw[3];
		}
		else
			LError<<"\"screenwindow\" should have four values";
	}
    return new MeshCamera(cam2world,false, nvi / 3, vi, npi, P,
		S, N, uvs, shutteropen, shutterclose, film, medium);
}