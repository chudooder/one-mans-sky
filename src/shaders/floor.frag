R"zzz(
#version 330 core
in vec4 face_normal;
in vec4 vertex_normal;
in vec4 light_direction;
in vec4 camera_direction;
in vec2 uv_coords;
in vec4 world_position;
uniform vec4 diffuse;
uniform vec4 ambient;
uniform vec4 specular;
uniform float shininess;
uniform float alpha;
uniform sampler2D textureSampler;
out vec4 fragment_color;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float prand(vec2 co) {
	return texture(textureSampler, vec2(fract(co.x), fract(co.y))).x;
}

void main() {

	vec3 snow = vec3(0.95, 0.95, 0.95);
	vec3 grass = vec3(0.4, 0.9, 0.3);
	vec3 sand = vec3(1.0, 0.93, 0.667);

	// color ramp
	float height = 500;
	float floorY = -100;
	float y = world_position.y - floorY;

	vec3 color;

	float random = prand(world_position.xz / 32);

	if(world_position.y + 3 * random < 10) {
		color = sand;
	} else if (world_position.y + 3 * random > 350){
		color = snow;
	} else {
		color = grass * (height - y) / height
			+ snow * y / height;
	}

	color.x += 0.1 * random + 0.05;
	color.y += -0.1 * random + 0.05;
	color.z += 0.1 * random + 0.05;

	float dot_nl = dot(normalize(light_direction), normalize(vertex_normal));
	dot_nl = clamp(dot_nl, 0.0, 1.0);
	fragment_color = vec4(color * dot_nl, 1.0);
	// fragment_color = vec4(random, random, random, 1.0);
	// fragment_color = vec4(texture(textureSampler, vec2(0.5, 0.5)).xyz, 1.0);
}
)zzz"
