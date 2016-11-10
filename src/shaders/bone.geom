R"zzz(#version 330 core
layout (lines) in;
layout (line_strip, max_vertices = 2) out;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
void main() {
	for(int n = 0; n < gl_in.length(); n++) {
		gl_Position = projection * view * model * gl_in[n].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}

)zzz"