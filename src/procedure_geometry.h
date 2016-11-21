#ifndef PROCEDURE_GEOMETRY_H
#define PROCEDURE_GEOMETRY_H

#include <GL/glew.h>

#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <utility>

using namespace std;

void create_floor(
	glm::vec4 position,
	std::vector<glm::vec4>& floor_vertices, 
	std::vector<glm::uvec3>& floor_faces,
	std::vector<glm::vec4>& floor_normals,
	std::vector<glm::vec2>& floor_uv);

void create_water(
	std::vector<glm::vec4>& water_vertices, 
	std::vector<glm::uvec3>& water_faces,
	std::vector<glm::vec2>& water_uv);

void create_skybox(
	std::vector<glm::vec4>& sky_vertices, 
	std::vector<glm::uvec3>& sky_faces,
	const glm::vec3 center);

int create_skybox_tex();

vector<vector<float>> perlin_noise(int seed, int size, int depth);



#endif
