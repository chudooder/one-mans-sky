#include "hud_base.h"

bool HUD::_init = false;
GLuint HUD::dialTx;
GLuint HUD::fontTx;
const glm::vec4 HUD::color = {0.0f, 1.0f, 0.0f, 0.6f};
const ShaderUniform HUD::color_uni = {"frag_color", vector_binder, 
	[]() -> const void* {
		return &color;
	}
};

const char* HUD::vert = 
#include "shaders/hud.vert"
;
const char* HUD::tri_geom = 
#include "shaders/hud_tri.geom"
;
const char* HUD::line_geom = 
#include "shaders/hud_line.geom"
;
const char* HUD::frag = 
#include "shaders/hud.frag"
;
const char* HUD::solid_frag =
#include "shaders/hud_solid.frag"
;