#ifndef TEXT_H
#define TEXT_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

using namespace glm;

class Text {
	std::string text;
	float width;
	float height;
	float aspect;

public:
	static const glm::vec4 color;

	Text(std::string s, float w, float h, float aspect = 1.0f)
		: text(s), width(w), height(h), aspect(aspect) { }

	void leftAlignGeom(glm::vec2 base,
		std::vector<glm::vec2>& position,
		std::vector<glm::vec2>& uv,
		std::vector<glm::uvec3>& faces)
	{
		getVBOs(base, position, uv, faces);
	}

	void rightAlignGeom(glm::vec2 base,
		std::vector<glm::vec2>& position,
		std::vector<glm::vec2>& uv,
		std::vector<glm::uvec3>& faces)
	{
		getVBOs(glm::vec2(base.x - width * text.length(), base.y), position, uv, faces);
	}

	void centerAlignGeom(glm::vec2 base,
		std::vector<glm::vec2>& position,
		std::vector<glm::vec2>& uv,
		std::vector<glm::uvec3>& faces)
	{
		getVBOs(glm::vec2(base.x - width * text.length() / 2, base.y), position, uv, faces);
	}

private:
	void getVBOs(glm::vec2 base,
		std::vector<glm::vec2>& position,
		std::vector<glm::vec2>& uv,
		std::vector<glm::uvec3>& faces) 
	{
		float uvheight = 1.0f/16.0f;
		float uvwidth = uvheight * aspect;

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

};

#endif // TEXT_H

