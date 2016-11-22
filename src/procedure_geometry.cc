#include "procedure_geometry.h"
#include "config.h"
#include <bitmap.h>
#include <image.h>
#include "jpegio.h"
#include <debuggl.h>

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

float noise_to_height(float val) {
	return kFloorY + kFloorHeight * val * val * val;
}

template <typename T>
T clamp(T low, T high, T val) {
	return min(high, max(low, val));
}

void create_floor(
	std::vector<glm::vec4>& floor_vertices, 
	std::vector<glm::uvec3>& floor_faces,
	std::vector<glm::vec4>& floor_normals,
	std::vector<glm::vec2>& floor_uv)
{
	int width = pow(2, kFloorSize);
	auto terrain = perlin_noise(kFloorSeed, kFloorSize, kFloorDepth);

	float sep = (kFloorXMax - kFloorXMin) / (float) width;
	float uv_sep = 1.0 / (float) width;

	// vertices
	for(int i = 0; i < width; i++) {	// row
		for(int j = 0; j < width; j++) {	// col
			float val = terrain[i][j];
			floor_vertices.push_back(glm::vec4(
				kFloorXMin + sep * j,
				noise_to_height(val),
				kFloorZMin + sep * i,
				1.0f));
			floor_uv.push_back(glm::vec2(uv_sep * j, uv_sep * i));
		}
	}

	// normals
	for(int i = 0; i < width; i++) {	// row
		for(int j = 0; j < width; j++) {	// col
			if(i == 0 || i == width-1 || j == 0 || j == width-1) {
				//TODO: some interpolation of these
				floor_normals.push_back(glm::vec4(0.0, 1.0, 0.0, 1.0));
				continue;
			}

			floor_normals.push_back(compute_vertex_normal(i, j, width, floor_vertices));
		}
	}

	// faces
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

	// erosion

	/*
	for(int t = 0; t < 5; t++) {
		std::vector<glm::vec4> eroded = floor_vertices;
		for(int i = 1; i < width - 1; i++) {
			for(int j = 1; j < width - 1; j++) {
				// check the normal at this point. If the angle from the y axis
				// is more than 45 degrees, then shift the point down and
				// recompute normals.

				glm::vec4 normal = floor_normals[i * width + j];
				float angle = std::acos(glm::dot(glm::vec3(normal), glm::vec3(0, 1, 0)));

				if (angle > 0.6) {	// 0.785 = pi / 4
					// erode to lowest point out of neighboring points
					float minY = floor_vertices[i * width + j].y;
					for(int r = i-1; r < i+2; r++) {
						for(int c = j-1; c < j+2; c++) {
							if(floor_vertices[r * width + c].y < minY) {
								minY = floor_vertices[r * width + c].y;
							}
						}
					}

					eroded[i * width + j].y -= 0.8 * (floor_vertices[i * width + j].y - minY);
				}
			}
		}
		floor_vertices = eroded;

		// recompute normals
		for(int i = 0; i < width; i++) {	// row
			for(int j = 0; j < width; j++) {	// col
				if(i == 0 || i == width-1 || j == 0 || j == width-1) {
					//TODO: some interpolation of these
					floor_normals[i * width + j] = glm::vec4(0.0, 1.0, 0.0, 1.0);
					continue;
				}

				floor_normals[i * width + j] = compute_vertex_normal(i, j, width, floor_vertices);
			}
		}
	}
	*/
	
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

void create_skybox(
	std::vector<glm::vec4>& sky_vertices, 
	std::vector<glm::uvec3>& sky_faces,
	const glm::vec3 center)
{
	glm::vec4 c = glm::vec4(center, 1.0);
	sky_vertices.push_back(c + glm::vec4(-kSkySize / 2, -kSkySize / 2, -kSkySize / 2, 0.0));
	sky_vertices.push_back(c + glm::vec4(-kSkySize / 2, -kSkySize / 2, kSkySize / 2, 0.0));
	sky_vertices.push_back(c + glm::vec4(-kSkySize / 2, kSkySize / 2, -kSkySize / 2, 0.0));
	sky_vertices.push_back(c + glm::vec4(-kSkySize / 2, kSkySize / 2, kSkySize / 2, 0.0));
	sky_vertices.push_back(c + glm::vec4(kSkySize / 2, -kSkySize / 2, -kSkySize / 2, 0.0));
	sky_vertices.push_back(c + glm::vec4(kSkySize / 2, -kSkySize / 2, kSkySize / 2, 0.0));
	sky_vertices.push_back(c + glm::vec4(kSkySize / 2, kSkySize / 2, -kSkySize / 2, 0.0));
	sky_vertices.push_back(c + glm::vec4(kSkySize / 2, kSkySize / 2, kSkySize / 2, 0.0));

	sky_faces.push_back(glm::uvec3(1, 0, 3));
	sky_faces.push_back(glm::uvec3(2, 3, 0));
	sky_faces.push_back(glm::uvec3(7, 4, 5));
	sky_faces.push_back(glm::uvec3(6, 4, 7));
	sky_faces.push_back(glm::uvec3(6, 7, 2));
	sky_faces.push_back(glm::uvec3(2, 7, 3));
	sky_faces.push_back(glm::uvec3(0, 5, 4));
	sky_faces.push_back(glm::uvec3(1, 5, 0));
	sky_faces.push_back(glm::uvec3(7, 5, 3));
	sky_faces.push_back(glm::uvec3(3, 5, 1));
	sky_faces.push_back(glm::uvec3(0, 4, 2));
	sky_faces.push_back(glm::uvec3(2, 4, 6));
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
					((float) rand() / RAND_MAX - 0.5f) * (float) pow(0.4f, cur_depth));
			}
		}

		noise = move(upsampled);
	}

	return noise;
}

int create_skybox_tex() {
	// Load skybox image
	std::vector<Image> images(6);
	LoadJPEG("assets/lake1_rt.JPG", &images[0]);
	LoadJPEG("assets/lake1_lf.JPG", &images[1]);
	LoadJPEG("assets/lake1_up.JPG", &images[2]);
	LoadJPEG("assets/lake1_dn.JPG", &images[3]);
	LoadJPEG("assets/lake1_bk.JPG", &images[4]);
	LoadJPEG("assets/lake1_ft.JPG", &images[5]);

	// Bind skybox textures
	GLuint skybox_tex;
	glGenTextures(1, &skybox_tex);
	std::cout << skybox_tex << std::endl;
	glActiveTexture(GL_TEXTURE0);


	std::cout << "Ready to bind textures" << std::endl;

	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);
	for(int i = 0; i < 6; i++) {
		Image& img = images[i];
		CHECK_GL_ERROR(glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.bytes.data()
		));
	}

	std::cout << "Bound textures" << std::endl;

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return skybox_tex;
}
