#include "chunk.h"

#include "config.h"
#include "procedure_geometry.h"

Chunk::Chunk(float x, float z) : x(x), z(z) {
	glm::vec4 position(x, kFloorY, z, 1.0);
	create_floor(
		position, 
		geom_verts, 
		geom_uv);
}

glm::vec4 compute_vertex_normal(
	const int i, 
	const int j, 
	const int width, 
	const std::vector<glm::vec4>& floor_vertices)
{
	glm::vec3 p = glm::vec3(floor_vertices[i * width + j]);
	glm::vec3 n0 = glm::normalize(glm::cross(
		-p + glm::vec3(floor_vertices[(i - 1) * width + (j - 1)]),
		-p + glm::vec3(floor_vertices[i * width + (j - 1)])));
	glm::vec3 n1 = glm::normalize(glm::cross(
		-p + glm::vec3(floor_vertices[(i - 1) * width + j]),
		-p + glm::vec3(floor_vertices[(i - 1) * width + (j - 1)])));
	glm::vec3 n2 = glm::normalize(glm::cross(
		-p + glm::vec3(floor_vertices[i * width + (j + 1)]),
		-p + glm::vec3(floor_vertices[(i - 1) * width + j])));
	glm::vec3 n3 = glm::normalize(glm::cross(
		-p + glm::vec3(floor_vertices[(i + 1) * width + (j + 1)]),
		-p + glm::vec3(floor_vertices[i * width + (j + 1)])));
	glm::vec3 n4 = glm::normalize(glm::cross(
		-p + glm::vec3(floor_vertices[(i + 1) * width + j]),
		-p + glm::vec3(floor_vertices[(i + 1) * width + (j + 1)])));
	glm::vec3 n5 = glm::normalize(glm::cross(
		-p + glm::vec3(floor_vertices[i * width + (j - 1)]),
		-p + glm::vec3(floor_vertices[(i + 1) * width + j])));
	return glm::vec4((n0 + n1 + n2 + n3 + n4 + n5) / 6.0f, 1.0f);
}

void stitch_chunks(
	std::vector<Chunk>& chunks,
	std::vector<glm::vec4>& floor_verts,
	std::vector<glm::uvec3>& floor_faces,
	std::vector<glm::vec4>& floor_normals,
	std::vector<glm::vec2>& floor_uv) 
{
	int offset = floor_verts.size();
	// interleave vertices
	int floor_width = pow(2, kFloorSize);
	int total_width = kChunkDraw * floor_width;
	for(int i = 0; i < total_width; i++) {
		for(int j = 0; j < total_width; j++) {
			int chunk_index = (i / floor_width) * kChunkDraw + j / floor_width;
			int val_index = (i % floor_width) * floor_width + j % floor_width;
			floor_verts.push_back(chunks[chunk_index].geom_verts[val_index]);
			floor_uv.push_back(chunks[chunk_index].geom_uv[val_index]);
		}
	}


	// faces
	for(int i = 1; i < total_width; i++) {
		for(int j = 1; j < total_width; j++) {
			// draw face for square (i-1, j-1) -> (i, j)
			floor_faces.push_back(glm::uvec3(
				i * total_width + j,
				(i - 1) * total_width + j,
				i * total_width + (j - 1)));
			floor_faces.push_back(glm::uvec3(
				i * total_width + (j - 1),
				(i - 1) * total_width + j,
				(i - 1) * total_width + (j - 1)));
		}
	}


	// normals
	for(int i = 0; i < total_width; i++) {	// row
		for(int j = 0; j < total_width; j++) {	// col
			if(i == 0 || i == total_width-1 || j == 0 || j == total_width-1) {
				//TODO: some interpolation of these
				floor_normals.push_back(glm::vec4(0.0, 1.0, 0.0, 1.0));
				continue;
			}

			floor_normals.push_back(compute_vertex_normal(i, j, total_width, floor_verts));
		}
	}
}