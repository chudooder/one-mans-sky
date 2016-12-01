R"zzz(
#version 330 core
uniform float time;

in vec4 position;
in vec4 d_position;
in vec4 color;
in vec4 d_color;

out vec4 vs_color;
void main(){
	float gamma = 5.5;
	gl_Position = -d_position / gamma * exp(-gamma * time) + position + d_position / gamma;
	// gl_Position = position + time * d_position - 0.5 * time * time * d_position / 2;
	vs_color = clamp(color + 2 * sqrt(time/2) * d_color, 0, 1);
}
)zzz"