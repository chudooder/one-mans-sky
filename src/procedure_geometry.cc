#include "procedure_geometry.h"
#include "config.h"

void create_floor(
	std::vector<glm::vec4>& floor_vertices, 
	std::vector<glm::uvec3>& floor_faces,
	std::vector<glm::vec2>& floor_uv)
{
	int width = pow(2, kFloorSize);
	auto terrain = perlin_noise(0, kFloorSize, kFloorDepth);

	float sep = (kFloorXMax - kFloorXMin) / (float) width;
	float uv_sep = 1.0 / (float) width;

	for(int i = 0; i < width; i++) {	// row
		for(int j = 0; j < width; j++) {	// col
			float val = terrain[i][j];
			floor_vertices.push_back(glm::vec4(
				kFloorXMin + sep * j,
				val * 100.0f - 30.0f,
				kFloorZMin + sep * i,
				1.0f));
			floor_uv.push_back(glm::vec2(uv_sep * j, uv_sep * i));
		}
	}

	for(int i = 1; i < width; i++) {
		for(int j = 1; j < width; j++) {
			// draw face for square (i-1, j-1) -> (i, j)
			floor_faces.push_back(glm::uvec3(
				i * width + j,
				(i - 1) * width + j,
				i * width + (j - 1)));
			floor_faces.push_back(glm::uvec3(
				i * width + (j - 1),
				(i - 1) * width + j,
				(i - 1) * width + (j - 1)));
		}
	}


	// floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMax, 1.0f));
	// floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMax, 1.0f));
	// floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMin, 1.0f));
	// floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMin, 1.0f));
	// floor_uv.push_back(glm::vec2(0.0, 0.0));
	// floor_uv.push_back(glm::vec2(0.0, 1.0));
	// floor_uv.push_back(glm::vec2(1.0, 1.0));
	// floor_uv.push_back(glm::vec2(1.0, 0.0));
	// floor_faces.push_back(glm::uvec3(0, 1, 2));
	// floor_faces.push_back(glm::uvec3(2, 3, 0));
}

void create_water(
	std::vector<glm::vec4>& water_vertices, 
	std::vector<glm::uvec3>& water_faces,
	std::vector<glm::vec2>& water_uv)
{
	int width = pow(2, kFloorSize);
	float sep = (kFloorXMax - kFloorXMin) / (float) width;
	float uv_sep = 1.0 / (float) width;
	for(int i = 0; i < width; i++) {	// row
		for(int j = 0; j < width; j++) {	// col
			water_vertices.push_back(glm::vec4(
				kFloorXMin + sep * j,
				0,
				kFloorZMin + sep * i,
				1.0f));
			water_uv.push_back(glm::vec2(uv_sep * j, uv_sep * i));
		}
	}

	for(int i = 1; i < width; i++) {
		for(int j = 1; j < width; j++) {
			// draw face for square (i-1, j-1) -> (i, j)
			water_faces.push_back(glm::uvec3(
				i * width + j,
				(i - 1) * width + j,
				i * width + (j - 1)));
			water_faces.push_back(glm::uvec3(
				i * width + (j - 1),
				(i - 1) * width + j,
				(i - 1) * width + (j - 1)));
		}
	}
}

template <typename T>
T clamp(T low, T high, T val) {
	return min(high, max(low, val));
}

vector<vector<float>> perlin_noise(int seed, int size, int depth) {

	srand(seed);

	int init_width = pow(2, size - depth);
	vector<vector<float>> noise(init_width, vector<float>(init_width, 0.0f));
	for(int i = 0; i < init_width; ++i) {
		for(int j = 0; j < init_width; ++j) {
			noise[i][j] += ((float) rand() / RAND_MAX);
		}
	}

	for (int cur_depth = 0; cur_depth < depth; ++cur_depth) {
		int width = pow(2, size - depth + cur_depth + 1);
		// upsample noise
		vector<vector<float>> upsampled(width, vector<float>(width, 0.0f));
		for(int i = 0; i < width; ++i) {
			for(int j = 0; j < width; ++j) {
				// ~*.~~*.~* MAGIC *.*~.*~~ //
				int i2 = clamp(0, width/2 - 1, i/2 - 1 + 2 * (i % 2));
				int j2 = clamp(0, width/2 - 1, j/2 - 1 + 2 * (j % 2));
				float a, b, c, d;
				a = noise[i/2][j/2];
				b = noise[i/2][j2];
				c = noise[i2][j/2];
				d = noise[i2][j2];

				upsampled[i][j] = clamp(0.0f, 1.0f, (9 * a + 3 * (b + c) + d) / 16.0f +
					((float) rand() / RAND_MAX - 0.5f) * (float) pow(0.5f, cur_depth));
			}
		}

		noise = move(upsampled);
	}

	return noise;
}