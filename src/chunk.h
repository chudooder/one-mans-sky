#ifndef CHUNK_H
#define CHUNK_H

#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include "config.h"

class Chunk {
public:
	float x;
	float z;

	std::vector<glm::vec4> geom_verts;
	std::vector<glm::vec2> geom_uv;

	Chunk(float x, float z);

};

void stitch_chunks(
	std::vector<Chunk>& chunks,
	std::vector<glm::vec4>& floor_verts,
	std::vector<glm::uvec3>& floor_faces,
	std::vector<glm::vec4>& floor_normals,
	std::vector<glm::vec2>& floor_uv);

#endif