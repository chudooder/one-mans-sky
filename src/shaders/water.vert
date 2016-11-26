R"zzz(
#version 330 core
uniform vec4 light_direction;
uniform vec3 camera_position;
uniform int time;
in vec4 vertex_position;
in vec2 uv;
out vec4 vs_light_direction;
out vec4 vs_normal;
out vec2 vs_uv;
out vec4 vs_camera_direction;
void main() {
	// float pi = 3.14159;
	// float slowness = 5;
	// float scale = 0.5;

	// float seconds = time / 1000.0 / slowness;
	gl_Position = vertex_position;
	// gl_Position.y = scale * (
	// 	  1.0 * sin((seconds + vertex_position.x) * 2 * pi) 
	// 	+ 1.0 * cos((seconds + vertex_position.z) * 2 * pi)
	// 	);

	// float dx = scale * (
	// 	  1.0 * 2 * pi * cos((seconds + vertex_position.x) * 2 * pi)
	// 	);
	// float dz = scale * (
	// 	  1.0 * 2 * pi * -sin((seconds + vertex_position.z) * 2 * pi)
	// 	);

	// vs_normal = vec4(normalize(vec3(dx, 1, dz)), 1.0);
	vs_normal = vec4(0.0, 1.0, 0.0, 0.0);
	vs_light_direction = light_direction;
	vs_camera_direction = vec4(camera_position, 1.0) - gl_Position;
	vs_uv = uv;
}
)zzz"
