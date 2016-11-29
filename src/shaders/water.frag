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

	float dot_nl = dot(light_direction, vertex_normal);
	dot_nl = clamp(dot_nl, 0.0, 1.0);
	vec4 diffuse = dot_nl * vec4(0.3, 0.2, 0.9, 0.5);

	vec4 sun_refl = normalize(-light_direction - 2 * dot(-light_direction, vertex_normal) * vertex_normal);
	vec4 specular = 1.0 * pow(max(dot(camera_direction, sun_refl), 0.0), 15.0) 
		* vec4(1.0, 0.3, 0.3, 1.0);

	fragment_color = clamp((1 - reflectance) * diffuse + reflectance * (reflColor + specular), 0.0, 1.0);
	// fragment_color = specular;
}
)zzz"