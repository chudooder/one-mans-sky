#ifndef TEXT_CC
#define TEXT_CC

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Text {
	std::string text;
	float height;

	static bool init;
	static GLuint fontTx;
	static void initFont();

public:
	static const glm::vec4 color;

	Text(std::string s, float h): text(s), height(h) { }

	void getVBOs(glm::vec2 base,
		std::vector<glm::vec2>& position,
		std::vector<glm::vec2>& uv,
		std::vector<glm::uvec3>& faces,
		bool reverse=false);

	static GLuint getFontTexture() {
		if(!init){
			initFont();
		}
		return fontTx;
	}

};

#endif // TEXT_CC

