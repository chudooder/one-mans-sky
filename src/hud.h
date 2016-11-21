#ifndef HUD_H
#define HUD_H

#include <GL/glew.h>
#include "render_pass.h"
#include "aircraft.h"

class Text {
	std::string text;
	size_t size;
public:
	Text(std::string s, size_t h): text(s), size(h) { }
	void getVBOs(glm::vec2 base,
		std::vector<glm::vec2>& position,
		std::vector<glm::vec2>& uv,
		std::vector<glm::uvec3>& faces);
};

class Altimeter {
	RenderPass* text_pass;
	Aircraft* aircraft;

	//VBOs
	std::vector<glm::vec2> t_position;
	std::vector<glm::vec2> t_uv;
	std::vector<glm::uvec3> t_faces;

public:
	Altimeter(Aircraft* a);
	void render();
};


#endif // HUD_H
