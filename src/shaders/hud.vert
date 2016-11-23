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
	scrolled.xy *= 2;
	scrolled.xy -= 1;
	gl_Position = scrolled;
	vs_uv = uv;
}
)zzz"