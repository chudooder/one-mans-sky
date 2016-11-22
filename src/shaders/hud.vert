R"zzz(
#version 330 core
uniform mat4 model;

in vec2 position;
in vec2 uv;
out vec2 vs_uv;
void main(){
	vec4 scrolled = model * vec4(position, 0, 1);
	vec2 scaled = vec2((scrolled.x - 400.0)/400.0, (scrolled.y - 300.0) / 300.0);
	gl_Position = vec4(scaled, 0, 1);
	vs_uv = uv;
}
)zzz"