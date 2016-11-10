R"zzz(
#version 330 core
uniform vec4 light_position;
uniform vec3 camera_position;
layout (std140) uniform anim_matrices {
	mat4 UI[200];	// undeformed transformation matrix per bone
	mat4 D[200];	// deformed transformation matrix per bone
};


in vec4 vertex_position;
in vec4 normal;
in vec2 uv;
in ivec4 bone_indices;	// indices of the 4 bones with the highest weights
in vec4 bone_weights;	// weights associated with each of the bones

out vec4 vs_light_direction;
out vec4 vs_normal;
out vec2 vs_uv;
out vec4 vs_camera_direction;
void main() {

	// calculation of position via U and D interpolation
	gl_Position = vec4(0, 0, 0, 0);
	for(int i=0; i<4; i++) {
		int index = bone_indices[i];
		if(bone_indices[i] == -1) break;
		gl_Position += bone_weights[i] * D[index] * UI[index] * vertex_position;
	}

	vs_light_direction = light_position - gl_Position;
	vs_camera_direction = vec4(camera_position, 1.0) - gl_Position;
	vs_normal = normal;
	vs_uv = uv;
}
)zzz"
