R"zzz(
#version 330 core
in vec4 face_normal;
in vec4 vertex_normal;
in vec4 light_direction;
in vec4 camera_direction;
in vec2 uv_coords;
uniform sampler2D textureSampler;
out vec4 fragment_color;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
void main() {
	float alpha = 1.0 - (0.9 * dot(normalize(camera_direction.xyz), vec3(0,1,0))
		+ 0.1 * dot(normalize(camera_direction.xyz), normalize(vertex_normal.xyz)));

	fragment_color = vec4(0.4, 0.4, 0.9, alpha);
}
)zzz"