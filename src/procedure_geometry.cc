#include "procedure_geometry.h"
#include "bone_geometry.h"
#include "config.h"


std::vector<glm::vec4> bone_verts;
std::vector<glm::uvec3> bone_faces;

void read_base_bone(){
	std::ifstream file;
	file.open("assets/obj/bone.obj");
	char cmd;
	while(!file.eof()) {
		file >> cmd;
		if(cmd == 'v'){
			float x, y, z;
			file >> x;
			file >> y;
			file >> z;
			bone_verts.push_back(glm::vec4(x, y, z, 1));
		} else if (cmd == 'f'){
			unsigned int a, b, c;
			file >> a;
			file >> b;
			file >> c;
			bone_faces.push_back(glm::uvec3(a, b, c));
		}
	}
	file.close();
}

void create_floor(std::vector<glm::vec4>& floor_vertices, std::vector<glm::uvec3>& floor_faces)
{
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMax, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMax, kFloorY, kFloorZMin, 1.0f));
	floor_vertices.push_back(glm::vec4(kFloorXMin, kFloorY, kFloorZMin, 1.0f));
	floor_faces.push_back(glm::uvec3(0, 1, 2));
	floor_faces.push_back(glm::uvec3(2, 3, 0));
}

void create_bone(std::vector<glm::vec4>& verts, std::vector<glm::uvec3>& faces,
	glm::vec4 pt0, glm::vec4 pt1){
	if(bone_verts.size() == 0){
		read_base_bone();
	}

	glm::vec4 t = pt1 - pt0;
	glm::mat4 scale;
	scale[1][1] = glm::length(t);
	scale[0][0] = 0.25f;
	scale[2][2] = 0.25f;

	glm::mat4 rotate;

	glm::vec3 t_3 = glm::normalize(glm::vec3(t));
	glm::vec3 v, n, b;
	int small = 0;
	if(std::abs(t_3[1]) < std::abs(t_3[small])) small = 1;
	if(std::abs(t_3[2]) < std::abs(t_3[small])) small = 2;
	v[small] = 1;
	n = glm::normalize(glm::cross(t_3, v));
	b = glm::cross(t_3, n);

	rotate[1] = glm::vec4(t_3, 0);
	rotate[0] = glm::vec4(n, 0);
	rotate[2] = glm::vec4(b, 0);

	glm::mat4 translate;
	translate[3] = pt0;

	int offset = verts.size() - 1;
	for(glm::vec4 vert : bone_verts){
		verts.push_back(translate * rotate * scale * vert);
	}
	for(glm::uvec3 face: bone_faces){
		faces.push_back(glm::uvec3(face[0] + offset, face[1] + offset, face[2] + offset));
	}

}


// FIXME: create cylinders and lines for the bones
// Hints: Generate a lattice in [-0.5, 0, 0] x [0.5, 1, 0] We wrap this
// around in the vertex shader to produce a very smooth cylinder.  We only
// need to send a small number of points.  Controlling the grid size gives a
// nice wireframe.
