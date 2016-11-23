#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "gui.h"
#include "config.h"

#define GRAVITY 9.81f
#define THRUST 20000.0f
#define MASS 5470.0f
#define BASE_DRAG 0.04f
#define BASE_LIFT 0.33f
#define WING_AREA 21.739f
#define WING_EFF 0.78f
#define WING_ASPECT 6.83
#define ATMOSPHERE_ALT 80000.0f

#define PITCH 0.4f
#define ROLL 1.2f
#define YAW 0.3f

#define STABILITY 0.01f

struct MatrixPointers;

class Aircraft {
protected:
	// Controls
	float throttleUp = 0.0f;
	float pitchUp = 0.0f;
	float rollLeft = 0.0f;
	float yawLeft = 0.0f;

	// Matrix addresses
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;

public:
	float throttle = 1.0f;
	glm::vec3 airspeed = glm::vec3(0.0f, 0.0f, -100.0f);
	glm::vec3 position = glm::vec3(0.0f, 100.0f, 100.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 look = glm::vec3(0.0f, 0.0f, -1.0f);

	Aircraft(GLFWwindow* window);

	virtual bool input(int key, int action);
	virtual void physicsStep(float time);
	MatrixPointers getMatrixPointers();
};

class KinematicAircraft : public Aircraft {
	glm::vec3 translation;
public:
	KinematicAircraft(GLFWwindow* window): Aircraft(window) { }
	virtual bool input(int key, int action);
	virtual void physicsStep(float time);
};


#endif // AIRCRAFT_H
