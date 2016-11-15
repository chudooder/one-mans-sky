#ifndef PROCEDURE_GEOMETRY_H
#define PROCEDURE_GEOMETRY_H

#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <utility>

using namespace std;

void create_floor(
	std::vector<glm::vec4>& floor_vertices, 
	std::vector<glm::uvec3>& floor_faces,
	std::vector<glm::vec2>& floor_uv);

void create_water(
	std::vector<glm::vec4>& water_vertices, 
	std::vector<glm::uvec3>& water_faces,
	std::vector<glm::vec2>& water_uv);

vector<vector<float>> perlin_noise(int seed, int size, int depth);



#endif
