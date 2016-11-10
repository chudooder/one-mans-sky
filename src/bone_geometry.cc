#include "config.h"
#include "bone_geometry.h"
#include "procedure_geometry.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>
#include <queue>
#include <cmath>

#include "gui.h"

/*
 * For debugging purpose.
 */
template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
	size_t count = std::min(v.size(), static_cast<size_t>(1000));
	for (size_t i = 0; i < count; ++i) os << i << " " << v[i] << "\n";
	os << "size = " << v.size() << "\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const BoundingBox& bounds)
{
	os << "min = " << bounds.min << " max = " << bounds.max;
	return os;
}

template <typename T>
std::vector<size_t> sort_indices(const std::vector<T> &v) {
	std::vector<size_t> idx(v.size());
	std::iota(idx.begin(), idx.end(), 0);

	std::sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] < v[i2];});

	return idx;
}


// FIXME: Implement bone animation.


Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::loadpmd(const std::string& fn)
{
	MMDReader mr;
	mr.open(fn);
	mr.getMesh(vertices, faces, vertex_normals, uv_coordinates);
	computeBounds();
	mr.getMaterial(materials);

	// load skeleton bones
	glm::vec3 offset;
	int p;
	std::unordered_map<int, Bone*> bones;
	bones[0] = new Bone(); //root bone
	skeleton.bones.push_back(bones[0]);
	mr.getJoint(0, offset, p);
	skeleton.position = glm::vec4(offset, 0);
	skeleton.numBones = 1;

	BlendWeights weights;

	while(mr.getJoint(skeleton.numBones, offset, p)){
		Bone *bone = new Bone();
		bone->id = skeleton.numBones;
		bone->parent = bones[p];
		bone->parent->children.push_back(bone);

		glm::vec3 t, v, n, b;
		glm::vec4 norm_offset = glm::vec4(offset, 0);
		if (glm::length(norm_offset) != 0) {
			norm_offset = glm::normalize(norm_offset);
		}
		t = glm::vec3(bone->parent->localToBone(norm_offset));

		int small = 0;
		if(std::abs(t[1]) < std::abs(t[small])) small = 1;
		if(std::abs(t[2]) < std::abs(t[small])) small = 2;
		v[small] = 1;
		n = glm::normalize(glm::cross(t, v));
		b = glm::cross(t, n);

		bone->rotation[0] = glm::vec4(t, 0);
		bone->rotation[1] = glm::vec4(n, 0);
		bone->rotation[2] = glm::vec4(b, 0);

		bone->init_rotation = bone->rotation;
		bone->len = glm::length(offset);

		bones[bone->id] = bone;
		skeleton.numBones++;
		skeleton.bones.push_back(bone);
	}

	skeleton.root = bones[0];

	// load joint weights into skeleton matrix
	std::vector<SparseTuple> tuples;
	mr.getJointWeights(tuples);
	for(SparseTuple tup : tuples) {
		Bone* parent = skeleton.bones[tup.jid];
		for(Bone* child : parent->children) {
			weights[tup.vid][child->id] = tup.weight;
		}	
	}

	// collect the top 4 weights/bones for each vertex, normalized
	for(int vid = 0; vid < vertices.size(); vid++) {
		auto weight_map = weights[vid];
		std::vector<int> k;
		std::vector<float> v;
		k.reserve(weight_map.size());
		v.reserve(weight_map.size());
		for(auto kv : weight_map) {
			k.push_back(kv.first);
			v.push_back(kv.second);
		}

		std::vector<size_t> indices = sort_indices(v);

		size_t lim = 4;
		if (indices.size() < 4) lim = indices.size();

		float total_weight = 0.0f;
		for(size_t i=0; i<lim; i++) {
			total_weight += v[indices[i]];
		}

		glm::ivec4 bi(-1, -1, -1, -1);
		glm::vec4 bw(0, 0, 0, 0);
		for(int i=0; i<lim; i++) {
			bi[i] = k[indices[i]];
			bw[i] = v[indices[i]] / total_weight;
		}
		bone_indices.push_back(bi);
		bone_weights.push_back(bw);
	}

	// precompute skeleton undeformed to world matrices
	glm::mat4 skele_transform;
	skele_transform[3] = skeleton.position;
	skele_transform[3][3] = 1;
	for(Bone* bone : skeleton.bones) {
		skeleton.anim_data.UI[bone->id] = 
			glm::inverse(skele_transform * bone->undeformedToWorld());
	}

	skeleton.updateAnimation();	
}

void Mesh::updateAnimation()
{
	
}


void Mesh::computeBounds()
{
	bounds.min = glm::vec3(std::numeric_limits<float>::max());
	bounds.max = glm::vec3(-std::numeric_limits<float>::max());
	for (const auto& vert : vertices) {
		bounds.min = glm::min(glm::vec3(vert), bounds.min);
		bounds.max = glm::max(glm::vec3(vert), bounds.max);
	}
}

void Skeleton::updateAnimation() {
	std::queue<Bone*> bones;

	std::vector<glm::vec4> _vertices;
	std::vector<glm::uvec2> _lines;	

	glm::mat4 skele_transform;
	skele_transform[3] = position;
	skele_transform[3][3] = 1;

	bones.push(root);
	while(!bones.empty()) {
		Bone* b = bones.front();
		bones.pop();
		for(Bone* c : b->children) {
			bones.push(c);
		}
		
		if(b == root) continue;
		glm::vec4 pt0 = b->boneToLocal(glm::vec4(0, 0, 0, 1)) + position;
		glm::vec4 pt1 = b->boneToLocal(glm::vec4(b->len, 0, 0, 1)) + position;
		
		int n = _vertices.size();
		_vertices.push_back(pt0);
		_vertices.push_back(pt1);
		_lines.push_back(glm::uvec2(n, n+1));

		// update transform matrices
		anim_data.D[b->id] = skele_transform * b->deformedToWorld();
	}

	vertices = _vertices;
	lines = _lines;
}

void Skeleton::highlightBone(glm::vec3 origin, glm::vec3 direction) {
	std::queue<Bone*> bones;
	bones.push(root);
	highlightedBone = nullptr;
	float bestT = 9999999.0f;
	while(!bones.empty()) {
		Bone* b = bones.front();
		bones.pop();
		for(Bone* c : b->children) {
			bones.push(c);
		}
		
		if(b == root) continue;

		glm::vec3 origin_ = glm::vec3(b->localToBone(glm::vec4(origin, 1) - position));
		glm::vec3 direction_ = glm::vec3(b->localToBone(glm::vec4(direction, 0)));

		float t;
		if(IntersectCylinder(origin_, direction_, 0.25f, b->len, &t) && t < bestT) {
			bestT = t;
			highlightedBone = b;
		}
	}
}

void Skeleton::highlightNextBone() {
	if(highlightedBone == nullptr) {
		highlightedBone = bones[1];
		return;
	}

	int id = highlightedBone->id;
	if(id == bones.size()) {
		highlightedBone = bones[1];
	} else {
		highlightedBone = bones[id + 1];
	}
}

void Skeleton::highlightPrevBone() {
	if(highlightedBone == nullptr) {
		highlightedBone = bones[1];
		return;
	}

	int id = highlightedBone->id;
	if(id == 1) {
		highlightedBone = bones[bones.size() - 1];
	} else {
		highlightedBone = bones[id - 1];
	}
}