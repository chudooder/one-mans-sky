#ifndef HUD_H
#define HUD_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <sstream>
#include "render_pass.h"
#include "hud_base.h"
#include "aircraft.h"

class SliderMeter {
	bool _setup = false;
	RenderPass* text_pass;
	RenderPass* line_pass;
	RenderPass* caret_pass;

	//VBOs
	std::vector<glm::vec2> t_position;
	std::vector<glm::vec2> t_uv;
	std::vector<glm::uvec3> t_faces;

	std::vector<glm::vec2> l_position;
	std::vector<glm::uvec2> l_lines;

	std::vector<glm::vec2> c_position;
	std::vector<glm::uvec3> c_faces;

	// Uniforms
	glm::mat4 transform;
	ShaderUniform transform_uni;

	glm::vec4 clip_area;
	ShaderUniform clip_area_uni;

public:
	void setup();
	void render();
private:
	virtual glm::vec2 getTranslation() = 0;
	virtual glm::vec4 getClipArea() = 0;
	virtual void makeText(
		std::vector<glm::vec2>& position, 
		std::vector<glm::vec2>& uv, 
		std::vector<glm::uvec3>& faces) = 0;
	virtual void makeLines(
		std::vector<glm::vec2>& position, 
		std::vector<glm::uvec2>& lines) = 0;
	virtual void makeCaret(
		std::vector<glm::vec2>& position, 
		std::vector<glm::uvec3>& faces) = 0;
};

class DialMeter {
	RenderPass* dial_pass;
	RenderPass* caret_pass;
	RenderPass* text_pass;

	std::vector<glm::vec2> d_position;
	std::vector<glm::vec2> d_uv;
	std::vector<glm::uvec3> d_faces;

	std::vector<glm::vec2> c_position;
	std::vector<glm::uvec3> c_faces;

	std::vector<glm::vec2> t_position;
	std::vector<glm::vec2> t_uv;
	std::vector<glm::uvec3> t_faces;

	glm::mat4 transform;
	ShaderUniform transform_uni;

	const glm::vec2 center;
	const float width;
	const float height;

public:
	DialMeter(glm::vec2 center, float width = 0.1f, float height = 0.132f);
	void render();
private:
	void updateTransform();
	void updateText();
	virtual float getDialAmount() = 0;
	virtual std::string getDialText() = 0;
};

class Speedometer : public DialMeter {
	const Aircraft& aircraft;
public:
	Speedometer(const Aircraft& a) : DialMeter({0.94, 0.066}), aircraft(a) {}
	float getDialAmount(){
		return glm::length(aircraft.airspeed) / 200.0f;
	}
	std::string getDialText(){
		std::ostringstream str;
		str.precision(1);
		str << std::fixed << glm::length(aircraft.airspeed) << "m/s";
		return str.str();
	}
};

class Throttometer : public DialMeter {
	const Aircraft& aircraft;
public:
	Throttometer(const Aircraft& a) : DialMeter({0.06, 0.066}), aircraft(a) {}
	float getDialAmount(){
		return aircraft.throttle;
	}
	std::string getDialText(){
		std::ostringstream str;
		str.precision(1);
		str << std::fixed << aircraft.throttle * 100 << "%";
		return str.str();
	}
};

class Altimeter : public SliderMeter {
	const Aircraft& aircraft;
public:
	Altimeter(const Aircraft& a) : aircraft(a) { }
private:
	virtual glm::vec2 getTranslation();
	virtual glm::vec4 getClipArea();
	virtual void makeText(
		std::vector<glm::vec2>& position, 
		std::vector<glm::vec2>& uv, 
		std::vector<glm::uvec3>& faces);
	virtual void makeLines(
		std::vector<glm::vec2>& position, 
		std::vector<glm::uvec2>& lines);
	virtual void makeCaret(
		std::vector<glm::vec2>& position, 
		std::vector<glm::uvec3>& faces);
};

class Heading : public SliderMeter {
	const Aircraft& aircraft;
public:
	Heading(const Aircraft& a) : aircraft(a) { }
private:
	glm::vec4 getClipArea();
	glm::vec2 getTranslation();
	virtual void makeText(
		std::vector<glm::vec2>& position, 
		std::vector<glm::vec2>& uv, 
		std::vector<glm::uvec3>& faces);
	virtual void makeLines(
		std::vector<glm::vec2>& position, 
		std::vector<glm::uvec2>& lines);
	virtual void makeCaret(
		std::vector<glm::vec2>& position, 
		std::vector<glm::uvec3>& faces);	
};

class Pitch : public SliderMeter {
	const Aircraft& aircraft;
public:
	Pitch(const Aircraft& a) : aircraft(a) { }
private:
	glm::vec4 getClipArea();
	glm::vec2 getTranslation();
	virtual void makeText(
		std::vector<glm::vec2>& position, 
		std::vector<glm::vec2>& uv, 
		std::vector<glm::uvec3>& faces);
	virtual void makeLines(
		std::vector<glm::vec2>& position, 
		std::vector<glm::uvec2>& lines);
	virtual void makeCaret(
		std::vector<glm::vec2>& position, 
		std::vector<glm::uvec3>& faces);
};


#endif // HUD_H
