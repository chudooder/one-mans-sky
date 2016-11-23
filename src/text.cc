#include "text.h"
#include <iostream>
#include <debuggl.h>
#include "jpegio.h"

using namespace std;
using namespace glm;


bool Text::init = false;
GLuint Text::fontTx;
const vec4 Text::color = vec4(0.0f, 1.0f, 0.0f, 0.6f);

void Text::initFont(){
	Image img;
	LoadJPEG("assets/hud_font.jpg", &img);

	CHECK_GL_ERROR(glGenTextures(1, &Text::fontTx));
	CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, Text::fontTx));
	CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width, img.height,
	  					        0, GL_RGB, GL_UNSIGNED_BYTE, img.bytes.data()));
	CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));

	Text::init = true;
}

void Text::getVBOs(
	vec2 base, 
	vector<vec2>& position, 
	vector<vec2>& uv, 
	vector<uvec3>& faces
){
	float width = height * aspect;
	float uvheight = 1.0f/16.0f;
	float uvwidth = uvheight * aspect;
	if(reverse) {
		base[0] -= text.length() * width;
	}

	for(unsigned i = 0; i < text.length(); i++){
		int index = position.size();
		position.push_back(base + vec2(i * width, 0));
		position.push_back(base + vec2(i * width, height));
		position.push_back(base + vec2(i * width + width, 0));
		position.push_back(base + vec2(i * width + width, height));

		char c = text[i];
		float uv_x = (c % 16) * uvheight;
		float uv_y = (c / 16) * uvheight;
		uv.push_back(vec2(uv_x, uv_y + uvheight));
		uv.push_back(vec2(uv_x, uv_y));
		uv.push_back(vec2(uv_x + uvwidth, uv_y + uvheight));
		uv.push_back(vec2(uv_x + uvwidth, uv_y));

		faces.push_back(uvec3(index, index + 2, index + 1));
		faces.push_back(uvec3(index + 3, index + 1, index + 2));
	}
}