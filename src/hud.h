#ifndef HUD_H
#define HUD_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "render_pass.h"
#include "aircraft.h"

class Altimeter {
	RenderPass* text_pass;
	RenderPass* line_pass;
	RenderPass* caret_pass;
	const Aircraft& aircraft;

	//VBOs
	std::vector<glm::vec2> t_position;
	std::vector<glm::vec2> t_uv;
	std::vector<glm::uvec3> t_faces;

	std::vector<glm::vec2> l_position;
	std::vector<glm::uvec2> l_lines;

	std::vector<glm::vec2> c_position;
	std::vector<glm::uvec3> c_faces;

	glm::mat4 transform;

	static glm::vec4 clip_area;
public:
	Altimeter(const Aircraft& a);
	void render();
private:
	void updateMatrix();
};


#endif // HUD_H
