#ifndef BONE_GEOMETRY_H
#define BONE_GEOMETRY_H

#define MAX_BONES 200

#include <ostream>
#include <vector>
#include <map>
#include <limits>
#include <glm/glm.hpp>
#include <mmdadapter.h>
#include <unordered_map>
#include <numeric>
#include <algorithm>

typedef std::unordered_map<int, std::unordered_map<int, float>> BlendWeights;


struct anim_matrices {
	glm::mat4 UI[MAX_BONES];
	glm::mat4 D[MAX_BONES];
};

struct BoundingBox {
	BoundingBox()
		: min(glm::vec3(-std::numeric_limits<float>::max())),
		max(glm::vec3(std::numeric_limits<float>::max())) {}
	glm::vec3 min;
	glm::vec3 max;
};

struct Bone {
	int id;
	Bone* parent;
	std::vector<Bone*> children;
	glm::mat4 init_rotation;
	glm::mat4 rotation;
	float len;

	glm::vec4 localToBone(glm::vec4 point) {
		if(!parent){
			return point;
		} else {
			glm::vec4 point_P = parent->localToBone(point);
			glm::mat4 translation;
			translation[3][0] = -parent->len;
			return glm::inverse(rotation) * translation * point_P;
		}
	}

	glm::vec4 boneToLocal(glm::vec4 point) {
		// if(!parent){
		// 	return point;
		// } else {
		// 	glm::mat4 translation;
		// 	translation[3][0] = parent->len;
		// 	point = translation * rotation * point;
		// 	return parent->boneToLocal(point);
		// }
		return deformedToWorld() * point;
	}

	glm::mat4 undeformedToWorld() {
		if(!parent) {
			return glm::mat4();	// identity
		}
		glm::mat4 translation;
		translation[3][0] = parent->len;
		return parent->undeformedToWorld() * translation * init_rotation;
	}

	glm::mat4 deformedToWorld() {
		if(!parent) {
			return glm::mat4();	// identity
		}
		glm::mat4 translation;
		translation[3][0] = parent->len;
		return parent->deformedToWorld() * translation * rotation;
	}
};


struct Skeleton {
	std::vector<glm::vec4> vertices;
	std::vector<glm::uvec2> lines;
	std::vector<Bone*> bones;
	anim_matrices anim_data;
	Bone* root = nullptr;
	Bone* highlightedBone = nullptr;
	glm::vec4 position;
	int numBones;

	void updateAnimation();
	void highlightBone(glm::vec3 origin, glm::vec3 direction);
	void highlightNextBone();
	void highlightPrevBone();
};

struct Mesh {
	Mesh();
	~Mesh();
	std::vector<glm::vec4> vertices;
	std::vector<glm::uvec3> faces;
	std::vector<glm::vec4> vertex_normals;
	std::vector<glm::vec4> face_normals;
	std::vector<glm::vec2> uv_coordinates;
	std::vector<glm::ivec4> bone_indices;
	std::vector<glm::vec4> bone_weights;
	std::vector<Material> materials;
	BoundingBox bounds;
	Skeleton skeleton;

	void loadpmd(const std::string& fn);
	void updateAnimation();
	int getNumberOfBones() const 
	{ 
		return 0;
		// FIXME: return number of bones in skeleton
	}
	glm::vec3 getCenter() const { return 0.5f * glm::vec3(bounds.min + bounds.max); }
private:
	void computeBounds();
	void computeNormals();
};

#endif
