#include "chunk.h"

#include "config.h"
#include "procedure_geometry.h"

Chunk::Chunk(float x, float z) : x(x), z(z) {
	glm::vec4 position(x, kFloorY, z, 1.0);
	create_floor(
		position, 
		geom_verts, 
		geom_faces, 
		geom_normals, 
		geom_uv);
}