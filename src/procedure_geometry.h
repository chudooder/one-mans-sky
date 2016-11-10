#ifndef PROCEDURE_GEOMETRY_H
#define PROCEDURE_GEOMETRY_H

#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>

class LineMesh;

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces);

void create_bone(std::vector<glm::vec4>& verts, std::vector<glm::uvec3>& faces,
	glm::vec4 pt0, glm::vec4 pt2);

#endif
