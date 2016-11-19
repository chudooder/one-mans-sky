R"zzz(
#version 330 core
in vec3 TexCoords;
uniform samplerCube skybox;
out vec4 fragment_color;

void main() {
	fragment_color = texture(skybox, TexCoords);
	// fragment_color = vec4(1.0, 0.0, 0.0, 1.0);
}
)zzz"
