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
	bool reverse;

public:
	static const glm::vec4 color;

	Text(std::string s, float w, float h, float aspect = 1.0f, bool reverse = false)
		: text(s), width(w), height(h), aspect(aspect), reverse(reverse) { }

	void getVBOs(glm::vec2 base,
		std::vector<glm::vec2>& position,
		std::vector<glm::vec2>& uv,
		std::vector<glm::uvec3>& faces) 
	{
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

};

#endif // TEXT_H

