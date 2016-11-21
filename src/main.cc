#include <GL/glew.h>
#include <dirent.h>

#include "procedure_geometry.h"
#include "render_pass.h"
#include "config.h"
#include "chunk.h"
#include "gui.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/io.hpp>
#include <debuggl.h>

int window_width = 800, window_height = 600;
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

// FIXME: Add more shaders here.

void ErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error: " << description << "\n";
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

int main(int argc, char* argv[])
{
	GLFWwindow *window = init_glefw();
	GUI gui(window);

	// Get skybox geometry
	std::vector<glm::vec4> skybox_vertices;
	std::vector<glm::uvec3> skybox_faces;

	create_skybox(skybox_vertices, skybox_faces, glm::vec3(0, 0, 0));
	std::cout << "Created skybox vertices" << std::endl;

	GLuint skybox_tex = create_skybox_tex();
	std::cout << "Created skybox texture" << std::endl;

	std::vector<Chunk> chunks;

	// Create floor data
	for(int i=0; i<3; i++) {
		for(int j=0; j<3; j++) {
			chunks.push_back(Chunk(-1024.0f + kFloorWidth * i, -1024.0f + kFloorWidth * j));
		}
	}

	std::vector<glm::vec4> floor_verts;
	std::vector<glm::uvec3> floor_faces;
	std::vector<glm::vec4> floor_normals;
	std::vector<glm::vec2> floor_uv;

	for(int i=0; i<chunks.size(); i++) {
		Chunk& c = chunks[i];
		int offset = floor_verts.size();
		floor_verts.insert(floor_verts.end(), c.geom_verts.begin(), c.geom_verts.end());
		// manually do the insert for faces cause indices
		auto gf = c.geom_faces.begin();
		std::cout << offset << std::endl;
		while(gf != c.geom_faces.end()) {
			glm::uvec3 face = *gf;
			face.x += offset;
			face.y += offset;
			face.z += offset;
			floor_faces.push_back(face);
			++gf;
		}

		// insert extra faces to connect to the previous chunk
		int floorWidth = pow(2, kFloorSize);
		if(i > 2) {
			int l_offset = (i - 3) * floorWidth * floorWidth;
			std::cout << "interp " << i << " " << l_offset << std::endl;
			// faces bordering tile to the negative x
			for(int z = 0; z < floorWidth - 1; z++) {
				int a = l_offset + floorWidth * z + (floorWidth - 1);
				int b = l_offset + floorWidth * (z + 1) + (floorWidth - 1);
				int c = offset + floorWidth * z;
				int d = offset + floorWidth * (z + 1);

				floor_faces.push_back(glm::uvec3(a, b, c));
				floor_faces.push_back(glm::uvec3(b, d, c));
			}
		}

		floor_normals.insert(floor_normals.end(), c.geom_normals.begin(), c.geom_normals.end());
		floor_uv.insert(floor_uv.end(), c.geom_uv.begin(), c.geom_uv.end());
	}

	std::cout << "Created floor" << std::endl;

	// Create water data
	std::vector<glm::vec4> water_vertices;
	std::vector<glm::uvec3> water_faces;
	std::vector<glm::vec2> water_uv;
	create_water(water_vertices, water_faces, water_uv);

	glm::vec4 light_position = glm::vec4(kFloorXMax, 5000.0f, kFloorZMax, 1.0f);
	MatrixPointers mats; // Define MatrixPointers here for lambda to capture
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
	auto std_camera_data  = [&gui]() -> const void* {
		return &gui.getCamera()[0];
	};
	auto std_proj_data = [&mats]() -> const void* {
		return mats.projection;
	};
	auto std_light_data = [&light_position]() -> const void* {
		return &light_position[0];
	};
	int time_millis = 0;
	auto time_data = [&time_millis]() -> const void* {
		return &time_millis;
	};

	ShaderUniform std_view = { "view", matrix_binder, std_view_data };
	ShaderUniform std_camera = { "camera_position", vector3_binder, std_camera_data };
	ShaderUniform std_proj = { "projection", matrix_binder, std_proj_data };
	ShaderUniform std_light = { "light_position", vector_binder, std_light_data };
	ShaderUniform floor_model = { "model", matrix_binder, floor_model_data };
	ShaderUniform skybox_model = { "model", matrix_binder, std_camera_data };
	ShaderUniform std_time = { "time", int_binder, time_data };

	RenderDataInput floor_pass_input;
	floor_pass_input.assign(0, "vertex_position", floor_verts.data(), floor_verts.size(), 4, GL_FLOAT);
	floor_pass_input.assign(1, "normal", floor_normals.data(), floor_normals.size(), 4, GL_FLOAT);
	floor_pass_input.assign(2, "uv", floor_uv.data(), floor_uv.size(), 2, GL_FLOAT);
	floor_pass_input.assign_index(floor_faces.data(), floor_faces.size(), 3);
	RenderPass floor_pass(-1,
			floor_pass_input,
			{ vertex_shader, geometry_shader, floor_fragment_shader},
			{ floor_model, std_view, std_proj, std_light },
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

	while (!glfwWindowShouldClose(window)) {
		// Setup some basic window stuff.
		glfwGetFramebufferSize(window, &window_width, &window_height);
		glViewport(0, 0, window_width, window_height);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glCullFace(GL_BACK);

		gui.updateMatrices();
		mats = gui.getMatrixPointers();
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
			floor_pass.setup();
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, floor_faces.size() * 3, GL_UNSIGNED_INT, 0));
		}

		if (draw_water) {
			water_pass.setup();
			CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, water_faces.size() * 3, GL_UNSIGNED_INT, 0));
		}
		
		// Poll and swap.
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	glfwDestroyWindow(window);
	glfwTerminate();
#if 0
	for (size_t i = 0; i < images.size(); ++i)
		delete [] images[i].bytes;
#endif
	exit(EXIT_SUCCESS);
}
