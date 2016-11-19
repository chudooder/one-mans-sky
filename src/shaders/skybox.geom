R"zzz(#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;
uniform mat4 projection;
uniform mat4 view;
out vec3 TexCoords;

void main() {
	mat4 view2 = mat4(mat3(view));
	for (int n = 0; n < gl_in.length(); n++) {
		TexCoords = gl_in[n].gl_Position.xyz;
		gl_Position = projection * view2 * gl_in[n].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}
)zzz"
