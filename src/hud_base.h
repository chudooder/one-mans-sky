#ifndef HUD_BASE_H
#define HUD_BASE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <debuggl.h>
#include <iostream>
#include "jpegio.h"
#include "render_pass.h"

class HUD {
	static bool _init;
	static GLuint fontTx;
	static GLuint dialTx;

public:
	const static glm::vec4 color;

	// Binders
	static void matrix_binder(int loc, const void* data) {
		glUniformMatrix4fv(loc, 1, GL_FALSE, (const GLfloat*)data);
	};
 	static void vector_binder(int loc, const void* data) {
		glUniform4fv(loc, 1, (const GLfloat*)data);
	};

	// Uniforms
	const static ShaderUniform color_uni;


	// Shaders
	static const char* vert;
	static const char* tri_geom;
	static const char* line_geom;
	static const char* frag;
	static const char* solid_frag;

	// Textures
	static GLuint getFontTexture() {
		if(!_init) init();
		return fontTx;
	}
	static GLuint getDialTexture() {
		if(!_init) init();
		return dialTx;
	}

private:
	static void init() {
		Image font_img;
		LoadJPEG("assets/hud_font.jpg", &font_img);

		CHECK_GL_ERROR(glGenTextures(1, &fontTx));
		CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0));
		CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, fontTx));
		CHECK_GL_ERROR(glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB, font_img.width, font_img.height, 0, GL_RGB, 
			GL_UNSIGNED_BYTE, font_img.bytes.data()
		));
		CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));

		Image dial_img;
		LoadJPEG("assets/dial.jpg", &dial_img);

		CHECK_GL_ERROR(glGenTextures(1, &dialTx));
		CHECK_GL_ERROR(glActiveTexture(GL_TEXTURE0));
		CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, dialTx));
		CHECK_GL_ERROR(glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGB, dial_img.width, dial_img.height, 0, GL_RGB, 
			GL_UNSIGNED_BYTE, dial_img.bytes.data()
		));
		CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, 0));

		_init = true;
	}
};

#endif // HUD_BASE_H
