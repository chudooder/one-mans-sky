R"zzz(
#version 330 core
in vec4 face_normal;
in vec4 vertex_normal;
in vec4 light_direction;
in vec4 camera_direction;
in vec2 uv_coords;
in vec4 gl_FragCoord;
in vec4 screen_position;
uniform sampler2D textureSampler;
out vec4 fragment_color;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
void main() {
	float R0 = pow((1.0 - 1.33) / (1.0 + 1.33), 2);
	float cos_theta = dot(normalize(camera_direction.xyz), normalize(vertex_normal.xyz));
	float reflectance = R0 + (1 - R0) * pow(1 - cos_theta, 5);

	vec2 refl_uv = (screen_position.xy / screen_position.w * vec2(-1, 1) + 1.0) / 2.0;
	vec4 reflColor = texture(textureSampler, refl_uv);
	vec4 ambient = vec4(0.4, 0.4, 0.9, 0.3);

	fragment_color = (1 - reflectance) * ambient + reflectance * reflColor;
	// fragment_color = vec4(1.0, 1.0, 1.0, factor);
}
)zzz"