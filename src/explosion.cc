#include "explosion.h"

#include <random>
#include <iostream>
#include <debuggl.h>

using namespace glm;
using namespace std;

#define N_PARTICLES 10
#define N_VELOCITY 10

const char* Explosion::vert_sh = 
#include "shaders/explosion.vert"
;
const char* Explosion::geom_sh =
#include "shaders/explosion.geom"
;
const char* Explosion::frag_sh = 
#include "shaders/explosion.frag"
;

auto float_binder = [](int loc, const void* data) {
	glUniform1fv(loc, 1, (const GLfloat*)data);
};	

vec4 random_vec4(float mean_mag, float std_mag){
	default_random_engine rng;
	normal_distribution<float> gauss(0, 1);

	vec4 v = {gauss(rng), gauss(rng), gauss(rng), 0};
	v = normalize(v);
	v *= gauss(rng) * std_mag + mean_mag;

	return v;
}

Explosion::Explosion(
	vec4 p0, 
	ShaderUniform model, 
	ShaderUniform view, 
	ShaderUniform projection) 
: time(0) {
	// geometry
	position = vector<vec4>(N_PARTICLES, p0);
	for(int i = 0; i < N_PARTICLES; ++i){
		d_position.push_back(random_vec4(N_VELOCITY, N_VELOCITY/4));
		color.push_back({1, 1, 1, 1});
		d_color.push_back({-0.5f, -1, -1, -1});
	}

	ShaderUniform time_uni = {"time", float_binder, 
		[this]() -> const void*{
			return &time;
		}
	};

	RenderDataInput particle_input;
	particle_input.assign(0, "position", position.data(), position.size(), 4, GL_FLOAT);
	particle_input.assign(1, "d_position", d_position.data(), d_position.size(), 4, GL_FLOAT);
	particle_input.assign(2, "color", color.data(), color.size(), 4, GL_FLOAT);
	particle_input.assign(3, "d_color", d_color.data(), d_color.size(), 4, GL_FLOAT);

	particle_pass = new RenderPass(-1,
		particle_input,
		{Explosion::vert_sh, Explosion::geom_sh, Explosion::frag_sh},
		{model, view, projection, time_uni},
		{"fragment_color"}
	);
	
}

void Explosion::render(float timeDelta){
	time += timeDelta;
	particle_pass->setup();
	CHECK_GL_ERROR(glDrawElements(GL_POINTS, position.size(), GL_UNSIGNED_INT, 0));
}