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
	std::vector<unsigned> index;

	float _time;
	int id;

	static int next_id;

	static const char* vert_sh;
	static const char* geom_sh;
	static const char* frag_sh;


public:
	Explosion(glm::vec4 p0, ShaderUniform v, ShaderUniform p);
	Explosion(const Explosion& exp) = delete;
	Explosion(Explosion&& exp) = delete;
	Explosion& operator=(const Explosion& exp) = delete;
	Explosion& operator=(Explosion&& exp) = delete;
	void physicsStep(float timeDelta);
	void render();
};

#endif // EXPLOSION_H
