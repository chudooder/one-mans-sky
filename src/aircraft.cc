#include "aircraft.h"
#include <iostream>

#define TELEMETRY false

using namespace glm;


Aircraft::Aircraft(GLFWwindow* window){
	int width, height;
	glfwGetWindowSize(window, &width, &height);

	projection = glm::perspective(
		(float)(kFov * (M_PI / 180.0f)), 
		static_cast<float>(width) / height, 
		kNear, kFar);
	model = glm::mat4(1.0f);
	view = glm::lookAt(position, position + look, up);
}

bool Aircraft::input(int key, int action){
	if(action != GLFW_PRESS && action != GLFW_RELEASE){
		return false;
	}
	bool down = action == GLFW_PRESS;

	switch(key){
	case GLFW_KEY_W: case GLFW_KEY_S:
		pitchUp += (down ? 1 : -1) * (key == GLFW_KEY_W ? 1 : -1);
		return true;
	case GLFW_KEY_Q: case GLFW_KEY_E:
		rollLeft += (down ? 1 : -1) * (key == GLFW_KEY_Q ? 1 : -1);
		return true;
	case GLFW_KEY_A: case GLFW_KEY_D:
		yawLeft += (down ? 1 : -1) * (key == GLFW_KEY_A ? 1 : -1);
		return true;
	case GLFW_KEY_R: case GLFW_KEY_F:
		throttleUp += (down ? 1 : -1) * (key == GLFW_KEY_R ? 1 : -1);
		return true;
	}
	return false;
}

void Aircraft::physicsStep(float time){

	position += airspeed * time;

	float altitude = position[1];
	float air_density = 1.225 * (ATMOSPHERE_ALT - altitude) / ATMOSPHERE_ALT;
	float aoa = acos(dot(up, normalize(airspeed))) - M_PI/2;

	float lift_c = BASE_LIFT + 2 * M_PI * aoa;
	float drag_c = BASE_DRAG + pow(lift_c, 2) / (M_PI * WING_EFF * WING_ASPECT);

	vec3 weight = vec3(0.0f, -GRAVITY * MASS, 0.0f);
	vec3 thrust = THRUST * throttle * look;
	vec3 drag = drag_c * air_density * glm::dot(airspeed, airspeed) * 0.5f * 
		   WING_AREA * -normalize(airspeed);
	vec3 lift = lift_c * air_density * glm::dot(airspeed, airspeed) * 0.5f *
	 	   WING_AREA * up;


	weight /= MASS;
	thrust /= MASS;
	drag /= MASS;
	lift /= MASS;


	airspeed += (weight + thrust + drag + lift) * time;

	// Stability
	vec3 stabAxis = cross(look, glm::normalize(airspeed));
	if(length(stabAxis) > 0){
		up = rotate(up, STABILITY * length(airspeed) * length(stabAxis) * time, stabAxis);
		look = rotate(look, STABILITY * length(airspeed) * length(stabAxis) * time, stabAxis);
	}

	// Debug
	if(TELEMETRY){
		std::cout << "--- BEGIN TELEMETRY ---" << std::endl;
		std::cout << "Altitude: " << altitude << std::endl;
		std::cout << "Air Density: " << air_density << std::endl; 
		std::cout << "Angle of Attack: " << aoa << std::endl; 
		std::cout << "Lift Coef: " << lift_c << std::endl; 
		std::cout << "Drag Coef: " << drag_c << std::endl; 
		std::cout << "Weight: " << weight[0] << " " << weight[1] << " " << weight[2] << std::endl; 
		std::cout << "Thrust: " << thrust[0] << " " << thrust[1] << " " << thrust[2] << std::endl; 
		std::cout << "Drag: " << drag[0] << " " << drag[1] << " " << drag[2] << std::endl; 
		std::cout << "Lift: " << lift[0] << " " << lift[1] << " " << lift[2] << std::endl; 
		std::cout << "Airspeed: " << airspeed[0] << " " << airspeed[1] << " " << airspeed[2] << std::endl; 
	}

	// Controls
	throttle = max(0.0f, min(1.0f, throttle + throttleUp * time));
	vec3 pitchAxis = cross(up, look);
	vec3 rollAxis = -look;
	vec3 yawAxis = up;

	up = rotate(up, pitchUp * PITCH * time, pitchAxis);
	look = rotate(look, pitchUp * PITCH * time, pitchAxis);
	up = rotate(up, rollLeft * ROLL * time, rollAxis);
	look = rotate(look, rollLeft * ROLL * time, rollAxis);
	up = rotate(up, yawLeft * YAW * time, yawAxis);
	look = rotate(look, yawLeft * YAW * time, yawAxis);
}

MatrixPointers Aircraft::getMatrixPointers(){
	view = glm::lookAt(position, position + look, up);
	MatrixPointers ret;

	ret.projection = &projection[0][0];
	ret.model= &model[0][0];
	ret.view = &view[0][0];

	return ret;
}

bool KinematicAircraft::input(int key, int action){
	if(Aircraft::input(key, action)){
		return true;
	}
	if(action != GLFW_PRESS && action != GLFW_RELEASE){
		return false;
	}
	bool down = action == GLFW_PRESS;
	switch(key){
	case GLFW_KEY_LEFT: case GLFW_KEY_RIGHT:
		translation.x += (down ? 1 : -1) * (key == GLFW_KEY_RIGHT ? 1 : -1);
		return true;
	case GLFW_KEY_UP: case GLFW_KEY_DOWN:
		translation.y += (down ? 1 : -1) * (key == GLFW_KEY_UP ? 1 : -1);
		return true;
	case GLFW_KEY_PERIOD: case GLFW_KEY_COMMA:
		translation.z += (down ? 1 : -1) * (key == GLFW_KEY_PERIOD ? 1 : -1);
		return true;
	}

	return false;
}

void KinematicAircraft::physicsStep(float time){
	vec3 pitchAxis = cross(up, look);
	vec3 rollAxis = -look;
	vec3 yawAxis = up;

	up = rotate(up, pitchUp * PITCH * time, pitchAxis);
	look = rotate(look, pitchUp * PITCH * time, pitchAxis);
	up = rotate(up, rollLeft * ROLL * time, rollAxis);
	look = rotate(look, rollLeft * ROLL * time, rollAxis);
	up = rotate(up, yawLeft * YAW * time, yawAxis);
	look = rotate(look, yawLeft * YAW * time, yawAxis);

	position += 300 * time * (translation.x * cross(look, up) + translation.y * up + translation.z * look);

}