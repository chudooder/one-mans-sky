#include "hud.h"
#include "jpegio.h"
#include <iostream>
#include <debuggl.h>
#include <string>

const char* vert = "";
const char* geom = "";
const char* frag = "";

using namespace glm;
using namespace std;

const char* altimeter_vert = 
#include "shaders/altimeter.vert"
;

const char* hud_geom = 
#include "shaders/hud.geom"
;

const char* text_frag = 
#include "shaders/text.frag"
;

GLuint font_tex;
bool font_init = false;
void init_font(){
	Image img;
	LoadJPEG("assets/samplefont.jpg", &img);
	cout << img.width  << "x" << img.height << endl;

	CHECK_GL_ERROR(glGenTextures(1, &font_tex));
	std::cout << font_tex << std::endl;
	CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, font_tex));
	CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height, 0, GL_RGB, GL_FLOAT, img.bytes.data()));
	font_init = true;
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));
	std::cout << "Bound font" << std::endl;
}

GLuint get_font_tex(){
	if(!font_init){
		init_font();
	}
	return font_tex;
}

void Text::getVBOs(vec2 base, vector<vec2>& position, vector<vec2>& uv, vector<uvec3>& faces){
	for(unsigned i = 0; i < text.length(); i++){
		int index = position.size();
		position.push_back(base + vec2(i * size, 0));
		position.push_back(base + vec2(i * size, size));
		position.push_back(base + vec2(i * size + size, 0));
		position.push_back(base + vec2(i * size + size, size));

		char c = text[i];
		float uv_x = (c % 16)/16.0f;
		float uv_y = (c / 16)/14.0f;
		uv.push_back(vec2(uv_x, 1.0f - uv_y));
		uv.push_back(vec2(uv_x, 1.0f - (uv_y + 1.0f/14.0f)));
		uv.push_back(vec2(uv_x + 1.0f/16.0f, 1.0f - uv_y));
		uv.push_back(vec2(uv_x + 1.0f/16.0f, 1.0f - (uv_y + 1.0f/14.0f)));

		faces.push_back(uvec3(index, index + 2, index + 1));
		faces.push_back(uvec3(index + 3, index + 1, index + 2));
	}
}

Altimeter::Altimeter(Aircraft* a): aircraft(a){
	// Generate the text
	// get_font_tex();
	for (int i = -10000; i < 80000; i += 1000) {
		Text t(to_string(i), 16);
		t.getVBOs(vec2(0, i / 10 + 8), t_position, t_uv, t_faces);
	}

	auto float_binder = [](int loc, const void* data) {
		glUniform1fv(loc, 1, (const GLfloat*)data);
	};
	auto altitude_data = [&a]() -> const void* {
		return &a->position[1];
	};
	ShaderUniform position_uniform = {"altitude", float_binder, altitude_data};

	RenderDataInput text_input;
	text_input.assign(0, "position", t_position.data(), t_position.size(), 2, GL_FLOAT);
	text_input.assign(1, "uv", t_uv.data(), t_uv.size(), 2, GL_FLOAT);
	text_input.assign_index(t_faces.data(), t_faces.size(), 3);

	text_pass = new RenderPass(-1, text_input,
		{ altimeter_vert, hud_geom, text_frag },
		{ position_uniform },
		{ "fragment_color" }
	);
}

void Altimeter::render(){
	text_pass->setup();
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, get_font_tex()));
	CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, t_faces.size() * 3, GL_UNSIGNED_INT, 0));
}


