R"zzz(
#version 330 core
layout (lines) in;
layout (line_strip, max_vertices=2) out;

in vec2 vs_uv[];
out vec4 screen_coords;
out vec2 uv_coords;

void main(){
	int n = 0;
	for (n = 0; n < 2; n++) {
		uv_coords = vs_uv[n];
		gl_Position = gl_in[n].gl_Position;
		screen_coords = gl_in[n].gl_Position;
		EmitVertex();
	}

	EndPrimitive();
}
)zzz"