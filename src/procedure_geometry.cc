#include "procedure_geometry.h"
#include "config.h"
#include <bitmap.h>
#include <image.h>
#include "jpegio.h"
#include <debuggl.h>
#include <string>
#include <list>

float noise_to_height(float val) {
	float mod_val;
	if(val < 0.5)
		mod_val = 0.5 / (1 + pow(2.818, -7 * val)) - 0.25;
	else
		mod_val = pow(2.818, val - 0.5) - 0.763;
	return kFloorY + kFloorHeight * mod_val;
	// return kFloorY + kFloorHeight * val;
}

template <typename T>
T clamp(T low, T high, T val) {
	return min(high, max(low, val));
}

void create_floor(
	glm::vec4 position,
	std::vector<glm::vec4>& floor_vertices, 
	std::vector<glm::vec2>& floor_uv)
{
	int width = pow(2, kFloorSize);
	auto terrain = perlin_noise((int) position.x, (int) position.z, kFloorSize, kFloorDepth);

	float sep = (kFloorWidth) / (float) width;
	float uv_sep = 1.0 / (float) width;

	// vertices
	for(int i = 0; i < width; i++) {	// row
		for(int j = 0; j < width; j++) {	// col
			float val = terrain[i][j];
			floor_vertices.push_back(glm::vec4(
				position.x + sep * j,
				noise_to_height(val),
				position.z + sep * i,
				1.0f));
			floor_uv.push_back(glm::vec2(uv_sep * j, uv_sep * i));
		}
	}
}

void create_water(
	glm::vec4 position,
	std::vector<glm::vec4>& water_vertices, 
	std::vector<glm::uvec3>& water_faces,
	std::vector<glm::vec2>& water_uv)
{
	int width = pow(2, kFloorSize) / 4;
	float sep = (kFloorXMax - kFloorXMin) / (float) width;
	float uv_sep = 1.0 / (float) width;
	for(int i = 0; i < width; i++) {	// row
		for(int j = 0; j < width; j++) {	// col
			water_vertices.push_back(glm::vec4(
				position.x + sep * j,
				0,
				position.z + sep * i,
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

float blerp(float a, float b, float c, float d, float u, float v) {
	/*
	a b

	c d
	*/
	return (1.0f - u) * ((1.0f - v) * a + v * b)
		+ u * ((1.0f - v) * c + v * d);
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

vector<vector<float>> random_noise(int size) {
	int width = pow(2, size);
	vector<vector<float>> noise(width, vector<float>(width, 0.0f));
	for(int i = 0; i < width; i++) {
		for(int j = 0; j < width; j++) {
			noise[i][j] = ((float) rand() / RAND_MAX);
		}
	}
	return noise;
}

struct RandomPatch;
list<RandomPatch> patch_cache;

struct RandomPatch {

	int seed;
	vector<vector<vector<float>>> noise_levels;

	// depth == number of smaller patches to generate
	RandomPatch(int seed, int size, int depth) : seed(seed) {
		// scan cache for hit
		for(auto rp : patch_cache) {
			if(rp.seed == seed) {
				noise_levels = rp.noise_levels;
				// std::cout << "Cache hit!" << std::endl;
				return;
			}
		}

		// if no hit, generate and add to cache
		srand(seed);
		for(int cur_depth = 0; cur_depth < depth; ++cur_depth) {
			noise_levels.push_back(random_noise(size - depth + cur_depth + 1));
		}

		if(patch_cache.size() > 15) {
			patch_cache.pop_back();
		}
		patch_cache.push_front(*this);
		// std::cout << "Cache miss, size = " << patch_cache.size() << std::endl;
	}

	float val(int cur_depth, int i, int j) const {
		return noise_levels[cur_depth][i][j];
	}
};

float get_patch_value(
	const vector<RandomPatch>& patches, 
	int patch,
	int cur_depth, 
	int i, 
	int j)
{
	int width = patches[0].noise_levels[cur_depth].size();
	if(i < 0) {
		patch -= 3;
		i = width + i;
	} else if(i >= width) {
		patch += 3;
		i = i - width;
	}

	if(j < 0) {
		patch -= 1;
		j = width + j;
	} else if(j >= width) {
		patch += 1;
		j = j - width;
	}

	if(patch < 0 || patch >= patches.size()) {
		return 0;
	}

	return patches[patch].val(cur_depth, i, j);
};

vector<vector<float>> upsample_patches(vector<RandomPatch>& patches, int size, int depth) {
	vector<vector<vector<float>>> octaves;

	int width = pow(2, size); // final upsampled size
	for(int cur_depth = 0; cur_depth < depth - 1; ++cur_depth) {
		int factor = pow(2, depth - cur_depth - 1);
		vector<vector<float>> upsampled(width, vector<float>(width, 0.0f));
		for(int i = 0; i < width; ++i) {
			for(int j = 0; j < width; ++j) {
				// ~*.~~*.~* MAGIC *.*~.*~~ //
				int i2 = i/factor + 1;
				int j2 = j/factor + 1;
				float a, b, c, d;
				a = get_patch_value(patches, 4, cur_depth, i/factor, j/factor);
				b = get_patch_value(patches, 4, cur_depth, i/factor, j2);
				c = get_patch_value(patches, 4, cur_depth, i2, j/factor);
				d = get_patch_value(patches, 4, cur_depth, i2, j2);

				float u = (float) (i % factor) / factor;
				float v = (float) (j % factor) / factor;

				float val = blerp(a, b, c, d, u, v);

				upsampled[i][j] = clamp(0.0f, 1.0f, val);
			}
		}
		octaves.push_back(upsampled);
	}

	vector<vector<float>> noise(width, vector<float>(width, 0.0f));
	for(int oct = 0; oct < octaves.size(); oct++) {
		vector<vector<float>>& octave = octaves[oct];
		for(int i=0; i<width; ++i) {
			for(int j=0; j<width; ++j) {
				if(oct == 0) {
					noise[i][j] += octave[i][j];
				} else {
					noise[i][j] += pow(0.4, oct) * (octave[i][j] - 0.5f);
				}
			}
		}
	}

	return noise;
}

/**
 * Tiling perlin noise
 */
vector<vector<float>> perlin_noise(float x, float z, int size, int depth) {
	vector<RandomPatch> patches;
	/*
	 0 1 2
	 3 4 5
	 6 7 8
	*/
	for(int i = 0; i < 9; i++) {
		int xpos = (int)(x + ((i % 3) - 1) * kFloorWidth);
		int zpos = (int)(z + ((i / 3) - 1) * kFloorWidth);
		int seed = xpos * 15485863 + zpos;
		patches.push_back(RandomPatch(seed, size, depth));
	}

	return upsample_patches(patches, size, depth);
};

/**
 * Generates a patch of perlin noise that loops on itself,
 * ideal for terrain textures or other random applications
 */
vector<vector<float>> looping_perlin_noise(int seed, int size, int depth) {
	vector<RandomPatch> patches;
	for(int i = 0; i < 9; i++) {
		patches.push_back(RandomPatch(seed, size, depth));
	}

	return upsample_patches(patches, size, depth);
}

int create_looping_noise_tex() {
	vector<vector<float>> noise = looping_perlin_noise(1, kFloorSize, kFloorDepth);
	Image img;
	for(auto r : noise) {
		for(float val : r) {
			unsigned char v = (unsigned char) (clamp(0.0f, 1.0f, val) * 255);
			img.bytes.push_back(val);
			img.bytes.push_back(val);
			img.bytes.push_back(val);
		}
	}
	img.width = noise[0].size();
	img.height = noise.size();

	GLuint tex;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height,
		0, GL_RGB, GL_UNSIGNED_BYTE, img.bytes.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;

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
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex);
	for(int i = 0; i < 6; i++) {
		Image& img = images[i];
		CHECK_GL_ERROR(glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			GL_RGB, img.width, img.height, 0, GL_RGB, GL_UNSIGNED_BYTE, img.bytes.data()
		));
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return skybox_tex;
}
