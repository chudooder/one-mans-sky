#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "render_pass.h"

class Explosion {
	RenderPass* particle_pass;

	std::vector<glm::vec4> position;
	std::vector<glm::vec4> d_position;
	std::vector<glm::vec4> color;
	std::vector<glm::vec4> d_color;

	float time;

	static const char* vert_sh;
	static const char* geom_sh;
	static const char* frag_sh;

public:
	Explosion(glm::vec4 p0, ShaderUniform m, ShaderUniform v, ShaderUniform p);
	void render(float timeDelta);
};

#endif // EXPLOSION_H
