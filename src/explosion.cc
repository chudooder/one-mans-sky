#include "explosion.h"

#include <random>
#include <iostream>
#include <debuggl.h>
#include <chrono>

using namespace glm;
using namespace std;

#define N_PARTICLES 20000
#define N_VELOCITY 150

const char* Explosion::vert_sh = 
#include "shaders/explosion.vert"
;
const char* Explosion::geom_sh =
#include "shaders/explosion.geom"
;
const char* Explosion::frag_sh = 
#include "shaders/explosion.frag"
;

int Explosion::next_id = 0;

default_random_engine rng;
normal_distribution<float> gauss(0, 1);

auto float_binder = [](int loc, const void* data) {
	glUniform1fv(loc, 1, (const GLfloat*)data);
};	

vec4 random_vec4(float mean_mag, float std_mag){

	vec4 v = {gauss(rng), gauss(rng), gauss(rng), 0};
	v = normalize(v);
	v *= gauss(rng) * std_mag + mean_mag;

	cout << v[0] << " " << v[1] << " " << v[2] << " " << v[3] << endl;

	return v;
}

float* asdf;

Explosion::Explosion(
	vec4 p0, 
	ShaderUniform view, 
	ShaderUniform projection) 
: _time(0), id(next_id++) {
	// geometry
	position = vector<vec4>(N_PARTICLES, p0);
	for(int i = 0; i < N_PARTICLES; ++i){
		d_position.push_back(random_vec4(N_VELOCITY, 0.7f * N_VELOCITY));
		float start_r = gauss(rng) * 0.1;
		float speed_r = gauss(rng) * 0.1;
		float start_a = gauss(rng) * 0;
		float speed_a = gauss(rng) * 0;
		color.push_back(vec4((1+ start_r) * 3, 2 + start_r, 2 + start_r, 1+ start_a));
		d_color.push_back(vec4(-1.5 + speed_r, -1.5+ speed_r, -1.5+ speed_r, -0.5f+ speed_a));
		index.push_back(i);
	}

	ShaderUniform time_uni = {"time", float_binder, 
		[this]() -> const void*{
			return &(this->_time);
		}
	};

	RenderDataInput particle_input;
	particle_input.assign(0, "position", position.data(), position.size(), 4, GL_FLOAT);
	particle_input.assign(1, "d_position", d_position.data(), d_position.size(), 4, GL_FLOAT);
	particle_input.assign(2, "color", color.data(), color.size(), 4, GL_FLOAT);
	particle_input.assign(3, "d_color", d_color.data(), d_color.size(), 4, GL_FLOAT);
	particle_input.assign_index(index.data(), index.size(), 1);

	particle_pass = new RenderPass(-1,
		particle_input,
		{Explosion::vert_sh, Explosion::geom_sh, Explosion::frag_sh},
		{view, projection, time_uni},
		{"fragment_color"}
	);
	
}

void Explosion::physicsStep(float timeDelta){
	this->_time += timeDelta;
}

void Explosion::render(){
	particle_pass->setup();
	CHECK_GL_ERROR(glPointSize(7));
	CHECK_GL_ERROR(glDrawElements(GL_POINTS, N_PARTICLES, GL_UNSIGNED_INT, 0));
}