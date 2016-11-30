#include <GL/glew.h>
#include <dirent.h>

#include "procedure_geometry.h"
#include "render_pass.h"
#include "config.h"
#include "chunk.h"
#include "gui.h"
#include "aircraft.h"
#include "hud.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <cmath>

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>

#include <ctime>
#include <thread>

int window_width = 800, window_height = 600;
int buffer_width, buffer_height;
const std::string window_title = "One Man's Sky";

const char* vertex_shader =
#include "shaders/default.vert"
;

const char* geometry_shader =
#include "shaders/default.geom"
;

const char* fragment_shader =
#include "shaders/default.frag"
;

const char* floor_fragment_shader =
#include "shaders/floor.frag"
;

const char* floor_ref_fragment_shader =
#include "shaders/floor_ref.frag"
;

const char* skybox_vertex_shader = 
#include "shaders/skybox.vert"
;

const char* skybox_geometry_shader = 
#include "shaders/skybox.geom"
;

const char* skybox_fragment_shader =
#include "shaders/skybox.frag"
;

const char* water_vertex_shader =
#include "shaders/water.vert"
;

const char* water_fragment_shader =
#include "shaders/water.frag"
;

struct TerrainGeom {
	std::vector<glm::vec4> verts;
	std::vector<glm::uvec3> faces;
	std::vector<glm::vec4> normals;
	std::vector<glm::vec2> uv;
};

// FIXME: Add more shaders here.

void ErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << "\n";
}

void init_refl_buffer(GLuint& reflection_buffer, GLuint& reflection_texture) {
	reflection_buffer = 0;
	glGenFramebuffers(1, &reflection_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, reflection_buffer);
	glGenTextures(1, &reflection_texture);
	glBindTexture(GL_TEXTURE_2D, reflection_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buffer_width, buffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, buffer_width, buffer_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, reflection_texture, 0);
	GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Reflection framebuffer setup went wrong!" << std::endl;
}

GLFWwindow* init_glefw()
{
	if (!glfwInit())
		exit(EXIT_FAILURE);
	glfwSetErrorCallback(ErrorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	auto ret = glfwCreateWindow(window_width, window_height, window_title.data(), nullptr, nullptr);
	CHECK_SUCCESS(ret != nullptr);
	glfwMakeContextCurrent(ret);
	glewExperimental = GL_TRUE;
	CHECK_SUCCESS(glewInit() == GLEW_OK);
	glGetError();  // clear GLEW's error for it
	glfwSwapInterval(1);
	const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
	const GLubyte* version = glGetString(GL_VERSION);    // version as a string
	std::cout << "Renderer: " << renderer << "\n";
	std::cout << "OpenGL version supported:" << version << "\n";

	return ret;
}

void generate_chunks(std::vector<Chunk*>& chunks, float cx, float cz) {
	if(chunks.size() == 0){
		for(int i=0; i<kChunkDraw; i++) {
			for(int j=0; j<kChunkDraw; j++) {
				chunks.push_back(new Chunk(
					cx + kFloorWidth * (j - kChunkDraw / 2), 
					cz + kFloorWidth * (i - kChunkDraw / 2)));
			}
		}
		return;
	}
	Chunk* center = chunks[chunks.size() / 2];
	float cur_x = center->x;
	float cur_z = center->z;
	int dx = (int)((cx - cur_x) / kFloorWidth);
	int dz = (int)((cz - cur_z) / kFloorWidth);
	if(dx == 1) {
		for(int j = 0; j < kChunkDraw; j++) {
			for(int i = 0; i < kChunkDraw; i++) {
				Chunk* other = j == kChunkDraw - 1 ? 
					nullptr : 
					chunks[i * kChunkDraw + (j + 1)];
				if(j == 0) {
					delete chunks[i * kChunkDraw + j];
				}
				chunks[i * kChunkDraw + j] = other;
			}
		}
	} else if (dx == -1) {
		for(int j = kChunkDraw - 1; j >= 0; j--) {
			for(int i = 0; i < kChunkDraw; i++) {
				Chunk* other = j == 0 ? 
					nullptr : 
					chunks[i * kChunkDraw + (j - 1)];
				if(j == kChunkDraw - 1) {
					delete chunks[i * kChunkDraw + j];
				}
				chunks[i * kChunkDraw + j] = other;
			}
		}
	}

	if(dz == 1) {
		for(int i = 0; i < kChunkDraw; i++) {
			for(int j = 0; j < kChunkDraw; j++) {
				Chunk* other = i == kChunkDraw - 1 ? 
					nullptr : 
					chunks[(i + 1) * kChunkDraw + j];
				if(i == 0) {
					delete chunks[i * kChunkDraw + j];
				}
				chunks[i * kChunkDraw + j] = other;
			}
		}
	} else if (dz == -1) {
		for(int i = kChunkDraw - 1; i >= 0; i--) {
			for(int j = 0; j < kChunkDraw; j++) {
				Chunk* other = i == 0 ? 
					nullptr : 
					chunks[(i - 1) * kChunkDraw + j];
				if(i == kChunkDraw - 1) {
					delete chunks[i * kChunkDraw + j];
				}
				chunks[i * kChunkDraw + j] = other;
			}
		}
	}

	// Create floor data
	for(int i=0; i<kChunkDraw; i++) {
		for(int j=0; j<kChunkDraw; j++) {
			if(chunks[i * kChunkDraw + j] == nullptr) {
				chunks[i * kChunkDraw + j] = new Chunk(
					cx + kFloorWidth * (j - kChunkDraw / 2), 
					cz + kFloorWidth * (i - kChunkDraw / 2));
			}
		}
	}
}

void threaded_chunk_update(
	TerrainGeom& geom,
	std::vector<Chunk*>& chunks, 
	float cx, 
	float cz, 
	int& status) {

	std::cout << "- Generating chunks" << std::endl;
	generate_chunks(chunks, cx, cz);
	geom.verts.clear();
	geom.faces.clear();
	geom.normals.clear();
	geom.uv.clear();
	stitch_chunks(chunks, geom.verts, geom.faces, geom.normals, geom.uv);

	std::cout << "- Nice lah" << std::endl;
	status = 1;
}

int main(int argc, char* argv[])
{
	GLFWwindow *window = init_glefw();
	glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
	Aircraft aircraft(window);
	GUI gui(window, &aircraft);
	Altimeter altimeter(aircraft);
	Speedometer speedometer(aircraft);
	Throttometer throttometer(aircraft);
	Heading heading(aircraft);
	Pitch pitch(aircraft);

	// Sampler
	unsigned sampler;
	CHECK_GL_ERROR(glGenSamplers(1, &sampler));
	CHECK_GL_ERROR(glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT));
	CHECK_GL_ERROR(glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT));
	CHECK_GL_ERROR(glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	CHECK_GL_ERROR(glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	CHECK_GL_ERROR(glBindSampler(0, (GLuint)(long)sampler));


	// Get skybox geometry
	std::vector<glm::vec4> skybox_vertices;
	std::vector<glm::uvec3> skybox_faces;

	create_skybox(skybox_vertices, skybox_faces, glm::vec3(0, 0, 0));
	std::cout << "Created skybox vertices" << std::endl;

	GLuint skybox_tex = create_skybox_tex();
	std::cout << "Created skybox texture at " << skybox_tex << std::endl;

	GLuint looping_tex = create_looping_noise_tex();
	std::cout << "Created looping noise texture at " << looping_tex << std::endl;

	std::vector<Chunk*> chunks;

	generate_chunks(chunks, 0, 0);


	TerrainGeom terrain;
	TerrainGeom swapTerrain;

	stitch_chunks(chunks, terrain.verts, terrain.faces, terrain.normals, terrain.uv);
	
	std::cout << "Created floor" << std::endl;

	// Create water data
	std::vector<glm::vec4> water_vertices;
	std::vector<glm::uvec3> water_faces;
	std::vector<glm::vec2> water_uv;
	create_water(
		glm::vec4(
			0.0f - kFloorWidth * (kChunkDraw / 2),
			0.0f,
			0.0f - kFloorWidth * (kChunkDraw / 2),
			1.0f),
		water_vertices, water_faces, water_uv);


	double light_angle = 0.0f;
	glm::vec4 light_direction = glm::normalize(
		glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	MatrixPointers mats; // Define MatrixPointers here for lambda to capture
	GLuint reflection_buffer, reflection_texture;
	init_refl_buffer(reflection_buffer, reflection_texture);

	/*
	 * In the following we are going to define several lambda functions to bind Uniforms.
	 * 
	 * Introduction about lambda functions:
	 *      http://en.cppreference.com/w/cpp/language/lambda
	 *      http://www.stroustrup.com/C++11FAQ.html#lambda
	 */
	glm::mat4 floor_model_matrix = glm::mat4(1.0f);
    	auto floor_model_data = [&floor_model_matrix]() -> const void* {
    	return &floor_model_matrix[0][0];
    }; // This return model matrix for the floor.
    glm::mat4 skybox_model_matrix = glm::mat4(1.0f);
    	auto skybox_model_data = [&skybox_model_matrix]() -> const void* {
    	return &skybox_model_matrix[0][0];
    };
	auto matrix_binder = [](int loc, const void* data) {
		glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat*)data);
	};
	auto vector_binder = [](int loc, const void* data) {
		glUniform4fv(loc, 1, (const GLfloat*)data);
	};
	auto vector3_binder = [](int loc, const void* data) {
		glUniform3fv(loc, 1, (const GLfloat*)data);
	};
	auto float_binder = [](int loc, const void* data) {
		glUniform1fv(loc, 1, (const GLfloat*)data);
	};
	auto int_binder = [](int loc, const void* data) {
		glUniform1iv(loc, 1, (const GLint*)data);
	};
	/*
	 * These lambda functions below are used to retrieve data
	 */
	auto std_view_data = [&mats]() -> const void* {
		return mats.view;
	};
	glm::mat4 refl_view_mat;
	auto refl_view_data = [&refl_view_mat]() -> const void* {
		return &refl_view_mat[0][0];
	};
	auto std_camera_data  = [&aircraft]() -> const void* {
		return &aircraft.position;
	};
	auto std_proj_data = [&mats]() -> const void* {
		return mats.projection;
	};
	auto std_light_data = [&light_direction]() -> const void* {
		return &light_direction[0];
	};
	int time_millis = 0;
	auto time_data = [&time_millis]() -> const void* {
		return &time_millis;
	};

	ShaderUniform std_view = { "view", matrix_binder, std_view_data };
	ShaderUniform refl_view = { "view", matrix_binder, refl_view_data };
	ShaderUniform std_camera = { "camera_position", vector3_binder, std_camera_data };
	ShaderUniform std_proj = { "projection", matrix_binder, std_proj_data };
	ShaderUniform std_light = { "light_direction", vector_binder, std_light_data };
	ShaderUniform floor_model = { "model", matrix_binder, floor_model_data };
	ShaderUniform skybox_model = { "model", matrix_binder, std_camera_data };
	ShaderUniform std_time = { "time", int_binder, time_data };

	RenderDataInput floor_pass_input;
	floor_pass_input.assign(0, "vertex_position", terrain.verts.data(), terrain.verts.size(), 4, GL_FLOAT);
	floor_pass_input.assign(1, "normal", terrain.normals.data(), terrain.normals.size(), 4, GL_FLOAT);
	floor_pass_input.assign(2, "uv", terrain.uv.data(), terrain.uv.size(), 2, GL_FLOAT);
	floor_pass_input.assign_index(terrain.faces.data(), terrain.faces.size(), 3);
	RenderPass floor_pass(-1,
			floor_pass_input,
			{ vertex_shader, geometry_shader, floor_fragment_shader},
			{ floor_model, std_view, std_proj, std_light },
			{ "fragment_color" }
			);

	RenderDataInput floor_refl_pass_input;
	floor_refl_pass_input.assign(0, "vertex_position", terrain.verts.data(), terrain.verts.size(), 4, GL_FLOAT);
	floor_refl_pass_input.assign(1, "normal", terrain.normals.data(), terrain.normals.size(), 4, GL_FLOAT);
	floor_refl_pass_input.assign(2, "uv", terrain.uv.data(), terrain.uv.size(), 2, GL_FLOAT);
	floor_refl_pass_input.assign_index(terrain.faces.data(), terrain.faces.size(), 3);
	RenderPass floor_refl_pass(-1,
			floor_refl_pass_input,
			{ vertex_shader, geometry_shader, floor_ref_fragment_shader},
			{ floor_model, refl_view, std_proj, std_light },
			{ "fragment_color" }
			);

	RenderDataInput skybox_refl_pass_input;
	skybox_refl_pass_input.assign(0, "vertex_position", skybox_vertices.data(), skybox_vertices.size(), 4, GL_FLOAT);
	skybox_refl_pass_input.assign_index(skybox_faces.data(), skybox_faces.size(), 3);
	RenderPass skybox_refl_pass(-1,
			skybox_refl_pass_input,
			{ skybox_vertex_shader, skybox_geometry_shader, skybox_fragment_shader },
			{ refl_view, std_proj, std_light },
			{ "fragment_color" }
			);

	RenderDataInput water_pass_input;
	water_pass_input.assign(0, "vertex_position", water_vertices.data(), water_vertices.size(), 4, GL_FLOAT);
	water_pass_input.assign(1, "uv", water_uv.data(), water_uv.size(), 2, GL_FLOAT);
	water_pass_input.assign_index(water_faces.data(), water_faces.size(), 3);
	RenderPass water_pass(-1,
			water_pass_input,
			{ water_vertex_shader, geometry_shader, water_fragment_shader },
			{ std_time, floor_model, std_view, std_proj, std_light, std_camera },
			{ "fragment_color" }
			);

	RenderDataInput skybox_pass_input;
	skybox_pass_input.assign(0, "vertex_position", skybox_vertices.data(), skybox_vertices.size(), 4, GL_FLOAT);
	skybox_pass_input.assign_index(skybox_faces.data(), skybox_faces.size(), 3);
	RenderPass skybox_pass(-1,
			skybox_pass_input,
			{ skybox_vertex_shader, skybox_geometry_shader, skybox_fragment_shader },
			{ std_view, std_proj, std_light },
			{ "fragment_color" }
			);

	float aspect = 0.0f;

	bool draw_floor = true;
	bool draw_water = true;
	bool draw_skybox = true;

	auto curTime = std::chrono::system_clock::now();
	auto lastTime = curTime;

	int status = 0;

	while (!glfwWindowShouldClose(window)) {
		// Setup some basic window stuff.
		glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
		glViewport(0, 0, buffer_width, buffer_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCullFace(GL_BACK);

        std::chrono::duration<double> diff = curTime-lastTime;



		aircraft.physicsStep(diff.count());
		mats = aircraft.getMatrixPointers();

		glm::vec3 refl_pos = aircraft.position;
		refl_pos[1] *= -1;

		glm::vec3 refl_look = aircraft.look;
		refl_look[1] *= -1;

		glm::vec3 refl_up = aircraft.up;
		refl_up[1] *= -1;

		refl_view_mat = glm::lookAt(refl_pos, refl_pos + refl_look, refl_up);

		// light direction
		// light_angle += 3.14159 * 2.0 * diff.count() / kDayLength;
		light_direction = glm::normalize(glm::vec4(
				0.0f,
				std::cos(light_angle),
				std::sin(light_angle),
				0.0f
			));

		time_millis = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()
		).count() % 10000;

		if (draw_skybox) {
			CHECK_GL_ERROR(glDepthMask(GL_FALSE));
			skybox_pass.setup();
			CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex));
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, skybox_faces.size() * 3, GL_UNSIGNED_INT, 0));
			CHECK_GL_ERROR(glDepthMask(GL_TRUE));
		}
		
		// Then draw floor.
		if (draw_floor) {
			// do we need to regenerate the terrain?
			int cx, cz;
			cx = (int) floor(aircraft.position.x / kFloorWidth) * kFloorWidth;
			cz = (int) floor(aircraft.position.z / kFloorWidth) * kFloorWidth;
			Chunk* center = chunks[chunks.size() / 2];
			if(cx != center->x || cz != center->z) {
				if(status == 0) {
					// spawn a new generating thread
					status = -1;
					std::cout << "Spin up new thread" << std::endl;
					std::thread update(
						threaded_chunk_update, 
						std::ref(swapTerrain),
						std::ref(chunks), 
						(float) cx, 
						(float) cz, 
						std::ref(status));
					update.detach();
				}
			}
			if (status == 1) {
				// regenerate terrain once thread is done
				swap(terrain, swapTerrain);
				floor_pass.updateVBO(0, terrain.verts.data(), terrain.verts.size());
				floor_pass.updateVBO(1, terrain.normals.data(), terrain.normals.size());
				floor_pass.updateVBO(2, terrain.uv.data(), terrain.faces.size());
				floor_refl_pass.updateVBO(0, terrain.verts.data(), terrain.verts.size());
				floor_refl_pass.updateVBO(1, terrain.normals.data(), terrain.normals.size());
				floor_refl_pass.updateVBO(2, terrain.uv.data(), terrain.faces.size());

				// regenerate water
				water_vertices.clear();
				water_faces.clear();
				water_uv.clear();
				create_water(glm::vec4(
					cx - kFloorWidth * (kChunkDraw / 2),
					0.0f,
					cz - kFloorWidth * (kChunkDraw / 2),
					1.0f),
					water_vertices, water_faces, water_uv);
				water_pass.updateVBO(0, water_vertices.data(), water_vertices.size());
				water_pass.updateVBO(1, water_uv.data(), water_uv.size());
				status = 0;
			}
			floor_pass.setup();
			CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, looping_tex));
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, terrain.faces.size() * 3, GL_UNSIGNED_INT, 0));
			CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
		}

		if (draw_water) {
			// reflections
			glBindFramebuffer(GL_FRAMEBUFFER, reflection_buffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if(draw_skybox) {
				CHECK_GL_ERROR(glDepthMask(GL_FALSE));
				skybox_refl_pass.setup();
				CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_tex));
				CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, skybox_faces.size() * 3, GL_UNSIGNED_INT, 0));
				CHECK_GL_ERROR(glDepthMask(GL_TRUE));
				CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
			}

			if (draw_floor) {
				floor_refl_pass.setup();
				CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, looping_tex));
				CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, terrain.faces.size() * 3, GL_UNSIGNED_INT, 0));
				CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));

			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			water_pass.setup();
			CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, reflection_texture));
			CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buffer_width, buffer_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0));
	
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, water_faces.size() * 3, GL_UNSIGNED_INT, 0));
		}

		altimeter.render();
		speedometer.render();
		throttometer.render();
		heading.render();
		pitch.render();
		
		// Poll and swap.
		glfwPollEvents();
		glfwSwapBuffers(window);

		// std::chrono::milliseconds timespan(1);
		// std::this_thread::sleep_for(timespan);

		lastTime = curTime;
		curTime = std::chrono::system_clock::now();

	}
	glfwDestroyWindow(window);
	glfwTerminate();
#if 0
	for (size_t i = 0; i < images.size(); ++i)
		delete [] images[i].bytes;
#endif
	exit(EXIT_SUCCESS);
}
