R"zzz(
#version 330 core
uniform mat4 transform;

in vec2 position;
in vec2 uv;
out vec2 vs_uv;
void main(){
	vec4 scrolled;
	if(transform[3].w == 1.0) {
		scrolled = transform * vec4(position, 0, 1);
	} else {
		scrolled = vec4(position, 0, 1);
	}
	vec2 scaled = vec2((scrolled.x - 400.0)/400.0, (scrolled.y - 300.0) / 300.0);
	gl_Position = vec4(scaled, 0, 1);
	vs_uv = uv;
}
)zzz"