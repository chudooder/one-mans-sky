R"zzz(#version 330 core
layout (points) in;
layout (points, max_vertices = 1) out;
uniform mat4 projection;
uniform mat4 view;

in vec4 vs_color[];
out vec4 color;
void main() {
	gl_Position = projection * view * gl_in[0].gl_Position;
	color = vs_color[0];
	EmitVertex();
	EndPrimitive();
}
)zzz"
