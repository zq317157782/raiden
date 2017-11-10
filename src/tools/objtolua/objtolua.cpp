/*
 * main.cpp
 *
 *  Created on: 2017年1月10日
 *      Author: zhuqian
 */
#include <iostream>
#include <fstream>
#include "tiny_obj_loader.h"
#include <map>
#include <memory>
#include <vector>


struct Float2 {
	Float2(float xx, float yy) :x(xx), y(yy){}
	float x;
	float y;
};

struct Float3 {
	Float3(float xx, float yy, float zz) :x(xx), y(yy), z(zz) {}
	float x;
	float y;
	float z;
};


struct Vertex {
	Vertex(const tinyobj::index_t i, Float3 pos, Float3 n, Float2 u) :id(i), position(pos), normal(n), uv(u) {}

	tinyobj::index_t id;
	Float3 position;
	Float3 normal;
	Float2 uv;

	
};

bool operator == (const Vertex& A,const Vertex& B) {
	if (A.id.vertex_index != B.id.vertex_index) {
		return false;
	}
	if (A.id.normal_index != B.id.normal_index) {
		return false;
	}
	if (A.id.texcoord_index != B.id.texcoord_index) {
		return false;
	}
	return true;
}

struct Mesh {
	std::string name;
	//[key:value value:index] 
//	std::map<Vertex,int> vertices;
	std::vector<Vertex> verticesKeys;
	std::vector<int> verticesValues;
	//index
	std::vector<int> indices;
};


//载入obj文件
bool LoadObjFile(const std::string& filename,tinyobj::attrib_t* attrib, std::vector<tinyobj::shape_t>* shapes, std::vector<tinyobj::material_t>*materials) {
	std::string err;
	bool ret = tinyobj::LoadObj(attrib, shapes, materials, &err,
		filename.c_str());
	if (!err.empty()) { // `err` may contain warning message.
		std::cerr << err << std::endl;
	}
	return ret;
}

void PrintMesh(const Mesh& mesh) {
	/*std::cout << "Mesh Name: "<<mesh.name<<std::endl;
	for (auto v = mesh.vertices.begin(); v != mesh.vertices.end(); ++v) {
		std::cout << "Index[" << v->second << "]" << std::endl;
		std::cout << "Vertex[" << v->first.position.x<<" " << v->first.position.y << " " << v->first.position.z << "]" << std::endl;
		std::cout << "Normal[" << v->first.normal.x << " " << v->first.normal.y << " " << v->first.normal.z << "]" << std::endl;
		std::cout << "UV[" << v->first.uv.x << " " << v->first.uv.y << "]" << std::endl;
	}*/
}

void ConvertToLuaFile(const std::string& filename,const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes, const std::vector<tinyobj::material_t>& materials) {
	std::string luaName = filename.substr(0, filename.size() - 4) + ".lua";
	std::cout << "Convert "<< filename << " to " << luaName << std::endl;
	//设置lua文件的输出流以及相应的设置
	std::ofstream lua(luaName);
	lua.setf(std::iostream::fixed, std::iostream::floatfield);
	//这个变量是记录真正的Index的
	uint32_t index = 0;
	
	uint32_t index_offset = 0;
	//创建存放shape的数组
	std::vector<Mesh> meshs(shapes.size());
	//遍历所有的Shape
	for (size_t s = 0; s < shapes.size(); s++) {

		//遍历当前Shape下的Face
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			
			//获得当前的Face所拥有的Vertex个数
			int fv = shapes[s].mesh.num_face_vertices[f];
			if (fv != 3) {
				std::cerr<<"Only Support Triangle Mesh! But has "<<fv<<" Vertex's mesh!" << std::endl;
				exit(-1);
			}

			//遍历当前Face下的Vertex
			for (size_t v = 0; v < fv; v++) {
				//获得当前的Index结构
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				
				//获得position
				Float3 position(0,0,0);
				if (attrib.vertices.size() > 0) {
					position.x = attrib.vertices[3 * idx.vertex_index];
					position.y = attrib.vertices[3 * idx.vertex_index + 1];
					position.z = attrib.vertices[3 * idx.vertex_index + 2];
				}
				//获得法线
				Float3 normal(0, 0, 0);
				if (attrib.normals.size() > 0) {
					normal.x = attrib.normals[3 * idx.normal_index];
					normal.y = attrib.normals[3 * idx.normal_index + 1];
					normal.z = attrib.normals[3 * idx.normal_index + 2];
				}
				//获得UV
				Float2 uv(0, 0);
				if (attrib.texcoords.size() > 0) {
					uv.x = attrib.texcoords[2 * idx.texcoord_index];
					uv.y = attrib.texcoords[2 * idx.texcoord_index + 1];			
				}
				//创建一个顶点
				Vertex vertex(idx,position,normal,uv);

				bool hasVertex = false;
				int  hasVertexIndex = 0;
				//判断顶点是否存在
				for (auto v =0; v != meshs[s].verticesKeys.size(); ++v) {
					if (meshs[s].verticesKeys[v]== vertex) {
						hasVertex = true;
						hasVertexIndex = meshs[s].verticesValues[v];
						break;
					}
				}
				if (hasVertex) {
					meshs[s].indices.push_back(hasVertexIndex);
				}
				else {
					meshs[s].verticesKeys.push_back(vertex);
					meshs[s].verticesValues.push_back(index);
					meshs[s].indices.push_back(index);
					index++;
				}

			
			}
			index_offset += fv;
		}
	}

	//开始写lua文件
	lua << "local mesh={}\n";
	lua <<"mesh[\"shapes\"]={}\n";

	for (int m = 0; m < meshs.size(); ++m) {
		lua << "mesh.shapes[" << m << "]={\n";
		PrintMesh(meshs[m]);
		auto verticesKeys=meshs[m].verticesKeys;
		auto verticesValues = meshs[m].verticesValues;


		

		//排序整个顶点顺序
		std::vector<Vertex> orderedVertices;

		for (int i = 0; i < verticesKeys.size(); ++i) {
			for (int j = 0; j < verticesValues.size(); ++j) {
				if (verticesValues[j] == i) {
					orderedVertices.push_back(verticesKeys[i]);
				}
			}
		}


		lua << "  vertices={ type=\"point3f\" ,value={";
		int count = 0;
		for (int i = 0; i < orderedVertices.size();++i) {
			
			lua << orderedVertices[i].position.x << ",";
			lua << orderedVertices[i].position.y << ",";
			lua << orderedVertices[i].position.z;
			if (count != (orderedVertices.size()-1)) {
				lua << ",";
			}
			count++;
		}
		lua << "}},\n";


		lua << "  normals={ type=\"normal3f\" ,value={";
		count = 0;

		for (auto v : orderedVertices) {

			lua << v.normal.x << ",";
			lua << v.normal.y << ",";
			lua << v.normal.z;
			if (count != (orderedVertices.size() - 1)) {
				lua << ",";
			}
			count++;
		}
		lua << "}},\n";


		lua << "  uvs={ type=\"point2f\" ,value={";
		count = 0;

		for (auto v : orderedVertices) {

			lua << v.uv.x << ",";
			lua << v.uv.y;
			if (count != (orderedVertices.size() - 1)) {
				lua << ",";
			}
			count++;
		}
		lua << "}},\n";

		lua << "  indices={ type=\"int[]\" ,value={";
		for (int i = 0; i < meshs[m].indices.size(); ++i) {
			lua << meshs[m].indices[i];
			if (i != (meshs[m].indices.size() - 1)) {
				lua << ",";
			}
		}
		lua << "}}\n";

		lua << "}\n";
	}

	lua << "return mesh\n";
	lua.clear();
	lua.close();
}

//主函数入口
int main(int argc, char**argv) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	//1. 载入obj文件
	bool ret=LoadObjFile(argv[1],&attrib,&shapes,&materials);
	if (!ret) {
		exit(1);
	}

	//2. 转换到lua文件
	ConvertToLuaFile(argv[1],attrib,shapes,materials);
}


