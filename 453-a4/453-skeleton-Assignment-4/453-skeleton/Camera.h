#pragma once

//------------------------------------------------------------------------------
// This file contains an implementation of a spherical camera
//------------------------------------------------------------------------------

//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Camera {
public:

	Camera(float t, float p, float r);

	glm::mat4 getView();
	glm::vec3 getPos();
	void incrementTheta(float dt);
	void incrementPhi(float dp);
	void incrementR(float dr);

	void setTarget(const glm::vec3 &target) {
		cameraTarget = target;
	}

private:

	float theta;
	float phi;
	float radius;
	glm::vec3 cameraTarget = glm::vec3(0.0f);
};
