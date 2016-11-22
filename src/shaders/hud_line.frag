R"zzz(
#version 330 core

uniform vec4 frag_color;
out vec4 fragment_color;

void main(){
	fragment_color = frag_color;
}
)zzz"