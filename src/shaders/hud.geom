R"zzz(
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=3) out;

in vec2 vs_uv[];
in int vs_should_render[];
out vec2 uv_coords;
flat out int should_render;

void main(){
	int n = 0;
	should_render = vs_should_render[0];
	for (n = 0; n < gl_in.length(); n++) {
		uv_coords = vs_uv[n];
		gl_Position = gl_in[n].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
)zzz"