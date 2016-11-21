R"zzz(
#version 330 core
uniform float altitude;
in vec2 position;
in vec2 uv;
out vec2 vs_uv;
out int vs_should_render;
void main(){
	vec2 scrolled = vec2(-1, 0) + position / 600.0;
	gl_Position = vec4(scrolled, 0, 1);
	vs_uv = uv;
	vs_should_render = 1;
}
)zzz"