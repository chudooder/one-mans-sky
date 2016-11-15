#include <GL/glew.h>
#include <dirent.h>

#include "procedure_geometry.h"
#include "render_pass.h"
#include "config.h"
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

	// Create floor data
	std::vector<glm::vec4> floor_vertices;
	std::vector<glm::uvec3> floor_faces;
	std::vector<glm::vec2> floor_uv;
	create_floor(floor_vertices, floor_faces, floor_uv);

	auto floor_img = std::make_shared<Image>();

	floor_img->width = pow(2, kFloorSize);
	floor_img->height = pow(2, kFloorSize);
	floor_img->stride = 3;

	auto noise = perlin_noise(0, kFloorSize, kFloorDepth);
	for(auto row : noise) {
		for(float data : row) {
			char val = (unsigned char) (255 * data);
			// std::cout << (int) val << " ";
			floor_img->bytes.push_back(val);
			floor_img->bytes.push_back(val);
			floor_img->bytes.push_back(val);
		}
		// std::cout << endl;
	}

	Material floor_mat = {
		diffuse: glm::vec4(1.0, 1.0, 1.0, 1.0),
		ambient: glm::vec4(),
		specular: glm::vec4(),
		shininess: 0.0f,
		texture: floor_img,
		offset: 0,
		nfaces: floor_faces.size()
	};

	vector<Material> floor_mats;
	floor_mats.push_back(floor_mat);

	std::cout << "created floor" << std::endl;

	// Create water data
	std::vector<glm::vec4> water_vertices;
	std::vector<glm::uvec3> water_faces;
	std::vector<glm::vec2> water_uv;
	create_water(water_vertices, water_faces, water_uv);

	glm::vec4 light_position = glm::vec4(0.0f, 100.0f, 0.0f, 1.0f);
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
	ShaderUniform std_time = { "time", int_binder, time_data };

	RenderDataInput floor_pass_input;
	floor_pass_input.assign(0, "vertex_position", floor_vertices.data(), floor_vertices.size(), 4, GL_FLOAT);
	floor_pass_input.assign(1, "uv", floor_uv.data(), floor_uv.size(), 2, GL_FLOAT);
	floor_pass_input.assign_index(floor_faces.data(), floor_faces.size(), 3);
	floor_pass_input.useMaterials(floor_mats);
	RenderPass floor_pass(-1,
			floor_pass_input,
			{ vertex_shader, geometry_shader, fragment_shader},
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

	float aspect = 0.0f;

	bool draw_floor = true;
	bool draw_water = true;

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
		
		// Then draw floor.
		if (draw_floor) {
			floor_pass.setup();
			int mid = 0;
			while (floor_pass.renderWithMaterial(mid))
				mid++;
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
