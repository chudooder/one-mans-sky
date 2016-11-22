R"zzz(
#version 330 core

uniform sampler2D textureSampler;
uniform vec4 frag_color;

in vec2 uv_coords;
out vec4 fragment_color;

void main(){
	vec3 texcolor = texture(textureSampler, uv_coords).xyz;
	if(length(texcolor) < 0.5) {
		fragment_color = vec4(0, 0, 0, 0);
	} else {
		fragment_color = frag_color;
	}
}
)zzz"