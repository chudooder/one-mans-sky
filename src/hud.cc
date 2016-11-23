#include "hud.h"
#include "jpegio.h"
#include "text.h"
#include <iostream>
#include <string>
#include <debuggl.h>

#include "hud_base.h"

using namespace glm;
using namespace std;

const vec4 Altimeter::clip_area = {0, 0.25f, 1, 0.75f};
const ShaderUniform Altimeter::clip_area_uni = {"clip_area", HUD::vector_binder,
	[]() -> const void* {
		return &Altimeter::clip_area;
	}
};


Altimeter::Altimeter(const Aircraft& a) 
	: aircraft(a),
	  transform_uni({"transform", HUD::matrix_binder, [this]() -> const void* {
	  	return &transform;
	  }})
{
	// Text
	for (int i = -10000; i < 80000; i += 100) {
		string s = to_string(i);
		Text t(s, 0.01f, 0.02f, 0.7f, true);
		t.getVBOs(vec2(0.95, 0.5 + i/1000.0f - 0.01f), t_position, t_uv, t_faces);
	}

	RenderDataInput text_input;
	text_input.assign(0, "position", t_position.data(), t_position.size(), 2, GL_FLOAT);
	text_input.assign(1, "uv", t_uv.data(), t_uv.size(), 2, GL_FLOAT);
	text_input.assign_index(t_faces.data(), t_faces.size(), 3);

	text_pass = new RenderPass(-1, text_input,
		{ HUD::vert, HUD::tri_geom, HUD::frag },
		{ HUD::color_uni, clip_area_uni, transform_uni },
		{ "fragment_color" }
	);


	// Lines
	for (int i = -10000; i < 80000; i += 10) {
		int b = l_position.size();
		if(i % 100 == 0){
			l_position.push_back(vec2(0.955, 0.5f + i/1000.0f));
		} else {
			l_position.push_back(vec2(0.975, 0.5f + i/1000.0f));
		}
		l_position.push_back(vec2(0.985, 0.5f + i/1000.0f));
		l_lines.push_back(uvec2(b, b + 1));
	}

	RenderDataInput lines_input;
	lines_input.assign(0, "position", l_position.data(), l_position.size(), 2, GL_FLOAT);
	lines_input.assign_index(l_lines.data(), l_lines.size(), 2);

	line_pass = new RenderPass(-1, lines_input,
		{ HUD::vert, HUD::line_geom, HUD::solid_frag },
		{ HUD::color_uni, clip_area_uni, transform_uni },
		{ "fragment_color" }
	);

	// Caret
	c_position.push_back(vec2(0.987f, 0.5f));
	c_position.push_back(vec2(0.995f, 0.505f));
	c_position.push_back(vec2(0.995f, 0.495f));
	c_faces.push_back(uvec3(0, 2, 1));

	RenderDataInput caret_input;
	caret_input.assign(0, "position", c_position.data(), c_position.size(), 2, GL_FLOAT);
	caret_input.assign_index(c_faces.data(), c_faces.size(), 3);

	caret_pass = new RenderPass(-1, caret_input,
		{ HUD::vert, HUD::tri_geom, HUD::solid_frag },
		{ HUD::color_uni, clip_area_uni},
		{ "fragment_color" }
	);
}

void Altimeter::render(){
	updateMatrix();
	line_pass->setup();	
	CHECK_GL_ERROR(glDrawElements(GL_LINES, l_lines.size() * 2, GL_UNSIGNED_INT, 0));
	text_pass->setup();
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, HUD::getFontTexture()));
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, t_faces.size() * 3, GL_UNSIGNED_INT, 0));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
	caret_pass->setup();
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, c_faces.size() * 3, GL_UNSIGNED_INT, 0));

}

void Altimeter::updateMatrix(){
	transform[3][1] = -aircraft.position[1]/1000.0f;
}

Speedometer::Speedometer(const Aircraft& a) : aircraft(a) {
	transform_uni = {"transform", HUD::matrix_binder, [this]() -> const void* {
		return &transform;
	}};

	// Dial
	d_position.push_back({0.89, -0.01});
	d_position.push_back({0.89, 0.122});
	d_position.push_back({0.99, -0.01});
	d_position.push_back({0.99, 0.122});
	d_uv.push_back({0, 1});
	d_uv.push_back({0, 0});
	d_uv.push_back({1, 1});
	d_uv.push_back({1, 0});
	d_faces.push_back({0, 2, 1});
	d_faces.push_back({1, 2, 3});

	RenderDataInput dial_input;
	dial_input.assign(0, "position", d_position.data(), d_position.size(), 2, GL_FLOAT);
	dial_input.assign(1, "uv", d_uv.data(), d_uv.size(), 2, GL_FLOAT);
	dial_input.assign_index(d_faces.data(), d_faces.size(), 3);

	dial_pass = new RenderPass(-1, dial_input, 
		{HUD::vert, HUD::tri_geom, HUD::frag },
		{HUD::color_uni},
		{ "fragment_color" });

	// Caret
	c_position.push_back({0.003, 0});
	c_position.push_back({-0.003, 0});
	c_position.push_back({0, 0.048});
	c_faces.push_back({2, 1, 0});

	RenderDataInput caret_input;
	caret_input.assign(0, "position", c_position.data(), c_position.size(), 2, GL_FLOAT);
	caret_input.assign_index(c_faces.data(), c_faces.size(), 3);

	caret_pass = new RenderPass(-1, caret_input,
		{HUD::vert, HUD::tri_geom, HUD::solid_frag},
		{HUD::color_uni, transform_uni},
		{ "fragment_color" }
	);
}

void Speedometer::render(){
	updateTransform();
	caret_pass->setup();
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, c_faces.size() * 3, GL_UNSIGNED_INT, 0));

	dial_pass->setup();
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, HUD::getDialTexture()));
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, d_faces.size() * 3, GL_UNSIGNED_INT, 0));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
	}

void Speedometer::updateTransform(){
	float speed = length(aircraft.airspeed);
	float angle = (speed - 100.0f) / 100.0f * M_PI * 2.0f / 3.0f;
	angle = glm::min((float) M_PI * 2.0f / 3.0f, angle); //clamp
	mat4 rotation;
	rotation[0] = {cos(-angle), sin(-angle), 0, 0};
	rotation[1] = {-sin(-angle), cos(-angle), 0, 0};
	rotation[2] = {0, 0, 1, 0};
	rotation[3] = {0, 0, 0, 1};
	mat4 translation;
	translation[3] = {0.94, 0.056, 0, 1};
	transform = translation * rotation;
}

