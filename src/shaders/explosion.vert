R"zzz(
#version 330 core
uniform float time;

in vec4 position;
in vec4 d_position;
in vec4 color;
in vec4 d_color;

out vec4 vs_color;
void main(){
	gl_Position = position + time * d_position;
	vs_color = clamp(color + time * d_color, 0, 1);
}
)zzz"