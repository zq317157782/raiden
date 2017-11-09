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

struct Vertex {
	float x, y, z;
};

struct UV {
	float u, v;
};

struct Shape {
	std::string name;
	std::map<int, Vertex> vertices;
	std::map<int, Vertex> normals;
	std::map<int, UV> uvs;
	std::vector<int> indices;
};

int main(int argc, char**argv) {

	std::string inputfile = argv[1];
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err,
			inputfile.c_str());

	if (!err.empty()) { // `err` may contain warning message.
		std::cerr << err << std::endl;
	}

	if (!ret) {
		exit(1);
	}

	std::cout << "start pharse obj" << std::endl;
	std::ofstream out(inputfile.substr(0, inputfile.size() - 4) + ".lua");
	out.setf(std::iostream::fixed, std::iostream::floatfield);
//	out << inputfile.substr(0, inputfile.size() - 4)
//			<< "_vertices={type=\"point3f\",\n value={";
//	for (size_t i = 0; i < attrib.vertices.size(); ++i) {
//		out << attrib.vertices[i];
//		if (i != (attrib.vertices.size() - 1)) {
//			out << ",";
//		}
//		if (i % 9 == 8) {
//			out << "\n";
//		}
//	}
//	out << "}}\n";
//
//	out << inputfile.substr(0, inputfile.size() - 4)
//			<< "_normals={type=\"normal3f\",\n value={";
//	for (size_t i = 0; i < attrib.normals.size(); ++i) {
//		out << attrib.normals[i];
//		if (i != (attrib.normals.size() - 1)) {
//			out << ",";
//		}
//		if (i % 9 == 8) {
//			out << "\n";
//		}
//	}
//	out << "}}\n";
//
//	out << inputfile.substr(0, inputfile.size() - 4)
//			<< "_uvs={type=\"point2f\",\n value={";
//	for (size_t i = 0; i < attrib.texcoords.size(); ++i) {
//		out << attrib.texcoords[i];
//		if (i != (attrib.texcoords.size() - 1)) {
//			out << ",";
//		}
//		if (i % 6 == 5) {
//			out << "\n";
//		}
//	}
//	out << "}}\n";

	Shape *ss = new Shape[shapes.size()];

	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		ss[s].name = shapes[s].name;
		int index = 0;
		std::map<int, int> indexMap;
		std::cout << "shape:" << ss[s].name << std::endl;
		std::cout <<"-->triangle num:"<<shapes[s].mesh.num_face_vertices.size()<< std::endl;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

			int fv = shapes[s].mesh.num_face_vertices[f];
			for (size_t v = 0; v < fv; v++) {
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

				if (indexMap.find(idx.vertex_index) == indexMap.end()) {
					indexMap[idx.vertex_index] = index;
					index++;
					//std::cout<<index<<" ";
				}

				ss[s].indices.push_back(indexMap[idx.vertex_index]);

				if (attrib.vertices.size() > 0) {
					Vertex vertex;
					vertex.x = attrib.vertices[3 * idx.vertex_index];
					vertex.y = attrib.vertices[3 * idx.vertex_index + 1];
					vertex.z = attrib.vertices[3 * idx.vertex_index + 2];
					ss[s].vertices[indexMap[idx.vertex_index]] = vertex;
				}
				if (attrib.normals.size() > 0) {
					Vertex normal;
					normal.x = attrib.normals[3 * idx.normal_index];
					normal.y = attrib.normals[3 * idx.normal_index + 1];
					normal.z = attrib.normals[3 * idx.normal_index + 2];
					ss[s].normals[indexMap[idx.vertex_index]] = normal;
				}
				if (attrib.texcoords.size() > 0) {
					UV uv;
					uv.u = attrib.texcoords[2 * idx.texcoord_index];
					uv.v = attrib.texcoords[2 * idx.texcoord_index + 1];
					ss[s].uvs[indexMap[idx.vertex_index]] = uv;
				}

			}
			index_offset += fv;
		}
		std::cout << "-->vertex num:" << ss[s].vertices.size() << std::endl;
		std::cout << "-->normal num:" << ss[s].normals.size() << std::endl;
		std::cout << "--uv num:" << ss[s].uvs.size() << std::endl;

	}
	out <<"local mesh={}\n";
	out <<"mesh[\"shapes\"]={}\n";

	//开始写文件
	for (int i = 0; i < shapes.size(); ++i) {
		out <<"mesh.shapes[" << i << "]={\n  ";

		out << "indices={ type=\"int[]\" ,value={";
		for (int v = 0; v < ss[i].indices.size(); ++v) {
			out << ss[i].indices[v];
			if (v != ss[i].indices.size() - 1) {
				out << ",";
			}
		}
		out << "}},\n";

		out << "vertices={ type=\"point3f\" ,value={";
		for (int v = 0; v < ss[i].vertices.size(); ++v) {
			out << ss[i].vertices[v].x << ",";
			out << ss[i].vertices[v].y << ",";
			out << ss[i].vertices[v].z;
			if (v != ss[i].vertices.size() - 1) {
				out << ",";
			}
			if (v % 3 == 2) {
				out << "\n";
			}
		}
		out << "}},"; //vertex end
		out << "\n  ";

		out << "normals={ type=\"normal3f\" ,value={";
		for (int v = 0; v < ss[i].normals.size(); ++v) {
			out << ss[i].normals[v].x << ",";
			out << ss[i].normals[v].y << ",";
			out << ss[i].normals[v].z;
			if (v != ss[i].normals.size() - 1) {
				out << ",";
			}
			if (v % 3 == 2) {
				out << "\n";
			}
		}
		out << "}},"; //normal end
		out << "\n";

		out << "uvs={ type=\"point2f\" ,value={";
		for (int v = 0; v < ss[i].uvs.size(); ++v) {
			out << ss[i].uvs[v].u << ",";
			out << ss[i].uvs[v].v;
			if (v != ss[i].uvs.size() - 1) {
				out << ",";
			}
			if (v % 3 == 2) {
				out << "\n";
			}
		}
		out << "}}"; //uv end
		out << "\n";

		out << "}\n";
	}
	out << "return mesh\n";
	return 0;
}

