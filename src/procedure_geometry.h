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
	std::vector<glm::vec2>& floor_uv);

void create_water(
	glm::vec4 position,
	std::vector<glm::vec4>& water_vertices, 
	std::vector<glm::uvec3>& water_faces,
	std::vector<glm::vec2>& water_uv);

void create_skybox(
	std::vector<glm::vec4>& sky_vertices, 
	std::vector<glm::uvec3>& sky_faces,
	const glm::vec3 center);

int create_looping_noise_tex();

int create_skybox_tex();

vector<vector<float>> random_noise(int size);

vector<vector<float>> perlin_noise(float x, float z, int size, int depth);



#endif
