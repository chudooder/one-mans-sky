#include "hud.h"
#include "jpegio.h"
#include "text.h"
#include <iostream>
#include <string>
#include <debuggl.h>

using namespace glm;
using namespace std;

// shaders
const char* hud_vert = 
#include "shaders/hud.vert"
;

const char* hud_tri_geom = 
#include "shaders/hud_tri.geom"
;

const char* hud_line_geom = 
#include "shaders/hud_line.geom"
;

const char* text_frag = 
#include "shaders/text.frag"
;

const char* line_frag =
#include "shaders/hud_line.frag"
;


// uniforms
auto matrix_binder = [](int loc, const void* data) {
	glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat*)data);
};
auto vector_binder = [](int loc, const void* data) {
	glUniform4fv(loc, 1, (const GLfloat*)data);
};
auto hud_color_data = []() -> const void* {
	return &Text::color;
};

vec4 Altimeter::clip_area = {0, 150, 800, 450};

Altimeter::Altimeter(const Aircraft& a) : aircraft(a) {
	// Generate the text
	for (int i = -10000; i < 80000; i += 100) {
		string s = to_string(i);
		Text t(s, 14, 0.7f, true);
		t.getVBOs(vec2(765, 300 + i - 7), t_position, t_uv, t_faces);
	}

	// Generate the lines
	for (int i = -10000; i < 80000; i += 10) {
		int b = l_position.size();
		if(i % 100 == 0){
			l_position.push_back(vec2(770, 300 + i));
		} else {
			l_position.push_back(vec2(780, 300 + i));
		}
		l_position.push_back(vec2(788, 300 + i));
		l_lines.push_back(uvec2(b, b + 1));
	}

	// Generate the caret
	c_position.push_back(vec2(790, 300));
	c_position.push_back(vec2(798, 304));
	c_position.push_back(vec2(798, 296));
	c_faces.push_back(uvec3(0, 2, 1));

	// Uniforms
	auto transform_data = [this]() -> const void* {
		return &transform;
	};
	auto clip_data = []() -> const void* {
		return &Altimeter::clip_area;
	};
	ShaderUniform transform_uni = {"transform", matrix_binder, transform_data};
	ShaderUniform frag_color_uni = {"frag_color", vector_binder, hud_color_data};
	ShaderUniform clip_area_uni = {"clip_area", vector_binder, clip_data};


	// VBOs
	RenderDataInput text_input;
	text_input.assign(0, "position", t_position.data(), t_position.size(), 2, GL_FLOAT);
	text_input.assign(1, "uv", t_uv.data(), t_uv.size(), 2, GL_FLOAT);
	text_input.assign_index(t_faces.data(), t_faces.size(), 3);

	RenderDataInput lines_input;
	lines_input.assign(0, "position", l_position.data(), l_position.size(), 2, GL_FLOAT);
	lines_input.assign_index(l_lines.data(), l_lines.size(), 2);

	RenderDataInput caret_input;
	caret_input.assign(0, "position", c_position.data(), c_position.size(), 2, GL_FLOAT);
	caret_input.assign_index(c_faces.data(), c_faces.size(), 3);

	text_pass = new RenderPass(-1, text_input,
		{ hud_vert, hud_tri_geom, text_frag },
		{ transform_uni, frag_color_uni, clip_area_uni },
		{ "fragment_color" }
	);

	line_pass = new RenderPass(-1, lines_input,
		{ hud_vert, hud_line_geom, line_frag },
		{ transform_uni, frag_color_uni, clip_area_uni },
		{ "fragment_color" }
	);

	caret_pass = new RenderPass(-1, caret_input,
		{ hud_vert, hud_tri_geom, line_frag },
		{ frag_color_uni, clip_area_uni},
		{ "fragment_color" }
	);


}

void Altimeter::render(){
	updateMatrix();
	line_pass->setup();	
	CHECK_GL_ERROR(glDrawElements(GL_LINES, l_lines.size() * 2, GL_UNSIGNED_INT, 0));
	text_pass->setup();
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, Text::getFontTexture()));
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, t_faces.size() * 3, GL_UNSIGNED_INT, 0));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
	caret_pass->setup();
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, c_faces.size() * 3, GL_UNSIGNED_INT, 0));

}

void Altimeter::updateMatrix(){
	transform[3][1] = -aircraft.position[1];
}


