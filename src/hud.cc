#include "hud.h"
#include "jpegio.h"
#include "text.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <debuggl.h>
#include <glm/gtx/vector_angle.hpp>

#include "hud_base.h"

using namespace glm;
using namespace std;

void SliderMeter::setup() {
	transform_uni = {"transform", HUD::matrix_binder, 
		[this]() -> const void* {
	  		return &transform;
	  	}
	};
	clip_area_uni = {"clip_area", HUD::vector_binder,
		[this]() -> const void* {
			return &clip_area;
		}
	};

	makeText(t_position, t_uv, t_faces);
	RenderDataInput text_input;
	text_input.assign(0, "position", t_position.data(), t_position.size(), 2, GL_FLOAT);
	text_input.assign(1, "uv", t_uv.data(), t_uv.size(), 2, GL_FLOAT);
	text_input.assign_index(t_faces.data(), t_faces.size(), 3);

	text_pass = new RenderPass(-1, text_input,
		{ HUD::vert, HUD::tri_geom, HUD::frag },
		{ HUD::color_uni, clip_area_uni, transform_uni },
		{ "fragment_color" }
	);

	makeLines(l_position, l_lines);
	RenderDataInput lines_input;
	lines_input.assign(0, "position", l_position.data(), l_position.size(), 2, GL_FLOAT);
	lines_input.assign_index(l_lines.data(), l_lines.size(), 2);

	line_pass = new RenderPass(-1, lines_input,
		{ HUD::vert, HUD::line_geom, HUD::solid_frag },
		{ HUD::color_uni, clip_area_uni, transform_uni },
		{ "fragment_color" }
	);

	makeCaret(c_position, c_faces);
	RenderDataInput caret_input;
	caret_input.assign(0, "position", c_position.data(), c_position.size(), 2, GL_FLOAT);
	caret_input.assign_index(c_faces.data(), c_faces.size(), 3);

	caret_pass = new RenderPass(-1, caret_input,
		{ HUD::vert, HUD::tri_geom, HUD::solid_frag },
		{ HUD::color_uni, clip_area_uni},
		{ "fragment_color" }
	);

	_setup = true;
}


void SliderMeter::render(){
	if(!_setup){
		setup();
	}
	clip_area = getClipArea();
	vec2 trans = getTranslation();
	transform[3] = {trans.x, trans.y, 0, 1};
	line_pass->setup();	
	CHECK_GL_ERROR(glDrawElements(GL_LINES, l_lines.size() * 2, GL_UNSIGNED_INT, 0));
	text_pass->setup();
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, HUD::getFontTexture()));
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, t_faces.size() * 3, GL_UNSIGNED_INT, 0));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
	caret_pass->setup();
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, c_faces.size() * 3, GL_UNSIGNED_INT, 0));

}

DialMeter::DialMeter(vec2 center, float width, float height) 
	: center(center), width(width), height(height) 
{
	transform_uni = {"transform", HUD::matrix_binder, [this]() -> const void* {
		return &transform;
	}};

	// Dial
	d_position.push_back(center + vec2(-width/2, -height/2));
	d_position.push_back(center + vec2(-width/2, height/2));
	d_position.push_back(center + vec2(width/2, -height/2));
	d_position.push_back(center + vec2(width/2, height/2));
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


	// Digits
	Text t("--", 0.14 * width, 0.14 * height);
	t.centerAlignGeom({center.x, center.y - height}, t_position, t_uv, t_faces);

	RenderDataInput text_input;
	text_input.assign(0, "position", t_position.data(), t_position.size(), 2, GL_FLOAT);
	text_input.assign(1, "uv", t_uv.data(), t_uv.size(), 2, GL_FLOAT);
	text_input.assign_index(t_faces.data(), t_faces.size(), 3);

	text_pass = new RenderPass(-1, text_input,
		{HUD::vert, HUD::tri_geom, HUD::frag},
		{HUD::color_uni},
		{ "fragment_color" }
	);

	// Caret
	c_position.push_back({width/30, 0});
	c_position.push_back({-width/30, 0});
	c_position.push_back({0, 0.35 * height});
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

void DialMeter::render(){
	updateTransform();
	caret_pass->setup();
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, c_faces.size() * 3, GL_UNSIGNED_INT, 0));

	dial_pass->setup();
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, HUD::getDialTexture()));
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, d_faces.size() * 3, GL_UNSIGNED_INT, 0));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));

	updateText();
	text_pass->setup();
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, HUD::getFontTexture()));
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, t_faces.size() * 3, GL_UNSIGNED_INT, 0));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
	}

void DialMeter::updateTransform(){
	float angle = std::min(1.0f, std::max(-1.0f, getDialAmount() * 2 - 1)) 
		* M_PI * 0.71;

	mat4 rotation;
	rotation[0] = {cos(-angle), sin(-angle), 0, 0};
	rotation[1] = {-sin(-angle), cos(-angle), 0, 0};
	rotation[2] = {0, 0, 1, 0};
	rotation[3] = {0, 0, 0, 1};
	mat4 translation;
	translation[3] = {center.x, center.y, 0, 1};
	transform = translation * rotation;
}

void DialMeter::updateText(){
	t_position.clear();
	t_uv.clear();
	t_faces.clear();

	Text t(getDialText(), 0.12f * width, 0.14 * height, 0.85f);
	t.centerAlignGeom({center.x, center.y - height * 0.45}, t_position, t_uv, t_faces);

	text_pass->updateVBO(0, t_position.data(), t_position.size());
	text_pass->updateVBO(1, t_uv.data(), t_uv.size());
	text_pass->updateIndex(t_faces.data(), t_faces.size());
}

vec2 Altimeter::getTranslation(){
	return {0, -aircraft.position[1]/1000.0f};
}

vec4 Altimeter::getClipArea() {
	return {0, 0.25, 1, 0.75};
}

void Altimeter::makeText(vector<vec2>& position, vector<vec2>& uv, vector<uvec3>& faces) {
	for (int i = -10000; i < 80000; i += 100) {
		string s = to_string(i);
		Text t(s, 0.01f, 0.02f, 0.7f);
		t.rightAlignGeom(vec2(0.95, 0.5 + i/1000.0f - 0.01f), position, uv, faces);
	}
}

void Altimeter::makeLines(vector<vec2>& position, vector<uvec2>& lines){
	for (int i = -10000; i < 80000; i += 10) {
		int b = position.size();
		if(i % 100 == 0){
			position.push_back(vec2(0.955, 0.5f + i/1000.0f));
		} else if (i % 50 == 0){
			position.push_back(vec2(0.965, 0.5f + i/1000.0f));
		} else {
			position.push_back(vec2(0.975, 0.5f + i/1000.0f));
		}
		position.push_back(vec2(0.985, 0.5f + i/1000.0f));
		lines.push_back(uvec2(b, b + 1));
	}	
}

void Altimeter::makeCaret(vector<vec2>& position, vector<uvec3>& faces){
	position.push_back(vec2(0.987f, 0.5f));
	position.push_back(vec2(0.995f, 0.505f));
	position.push_back(vec2(0.995f, 0.495f));
	faces.push_back(uvec3(0, 2, 1));
}

vec4 Heading::getClipArea(){
	return {0.25, 0, 0.75, 1};
}

vec2 Heading::getTranslation() {
	float heading = atan(-aircraft.look.x, aircraft.look.z);
	cout << heading / (2 * M_PI) * 360.0f << endl;
	return {-heading / (2 * M_PI) * 360.0f / 130.0f, 0};
}

void Heading::makeText(vector<vec2>& position, vector<vec2>& uv, vector<uvec3>& faces) {
	for(int i = -360; i <= 360; i += 10){
		if(i % 90 == 0){
			int j = (i + 360) % 360;
			string s;
			if(j == 0){
				s = "N";
			} else if (j == 90){
				s = "E";
			} else if (j == 180){
				s = "S";
			} else {
				s = "W";
			}
			Text t(s, 0.015f, 0.02f);
			t.centerAlignGeom({0.5f + i / 130.0f, 0.93f}, position, uv, faces);
		} else {	
			string s = to_string((i + 360) % 360) + char(176);
			Text t(s, 0.01f, 0.02f, 0.7f);
			t.centerAlignGeom({0.504f + i / 130.0f, 0.93f}, position, uv, faces);
		}
	}
}


void Heading::makeLines(vector<vec2>& position, vector<uvec2>& lines) {
	for(int i = -360; i <= 360; i += 1){
		int b = position.size();
		position.push_back({0.5f + i/130.0f, 0.982f});
		if(i % 10 == 0){
			position.push_back({0.5f + i/130.0f, 0.952f});
		} else if (i % 5 == 0){
			position.push_back({0.5f + i/130.0f, 0.962f});
		} else {			
			position.push_back({0.5f + i/130.0f, 0.972f});
		}
		lines.push_back({b, b + 1});
	}
}


void Heading::makeCaret(vector<vec2>& position, vector<uvec3>& faces) {
	position.push_back({0.5f, 0.985f});
	position.push_back({0.504f, 0.995f});
	position.push_back({0.496f, 0.995f});
	faces.push_back({0, 1, 2});
}

