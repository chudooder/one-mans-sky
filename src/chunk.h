#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <glm/glm.hpp>
#include <iostream>

class Chunk {
public:
	float x;
	float z;

	std::vector<glm::vec4> geom_verts;
	std::vector<glm::uvec3> geom_faces;
	std::vector<glm::vec4> geom_normals;
	std::vector<glm::vec2> geom_uv;

	Chunk(float x, float z);

};

#endif