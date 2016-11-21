R"zzz(
#version 330 core

uniform sampler2D textureSampler;
uniform vec4 textColor;

flat in int should_render;
in vec2 uv_coords;
out vec4 fragment_color;

void main(){
	vec3 texcolor = texture(textureSampler, uv_coords).xyz;
	if(length(texcolor) == 0.0 || should_render == 0) {
		fragment_color = vec4(0, 0, 0, 0);
	} else {
		fragment_color = textColor;
	}
}
)zzz"