//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "UnitSphere.h"

class Planet {
public:
	Planet(const std::string& texturePath, float scale, float orbitalRadius, float orbitSpeed, float selfRotationSpeed, float orbitalInclination, float axisTilt, Planet* parent = nullptr)
		: texture(texturePath, GL_LINEAR), scale(scale), orbitalRadius(orbitalRadius), orbitSpeed(orbitSpeed)
		, selfRotationSpeed(selfRotationSpeed), orbitalInclination(glm::radians(orbitalInclination)), axisTilt(glm::radians(axisTilt)), parent(parent)
	{}

	void update(float time) {
		modelMatrix = glm::mat4(1.0f);
		if (parent) {
			modelMatrix = parent->modelMatrix;
		}
		//orbital inclination
		modelMatrix = glm::rotate(modelMatrix, orbitalInclination, glm::vec3(1.0f, 0.0f, 0.0f));

		//rotate around parent
		float orbitAngle = time * orbitSpeed;
		modelMatrix = glm::rotate(modelMatrix, orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));

		//translate outward to orbital radius
		modelMatrix = glm::translate(modelMatrix, glm::vec3(orbitalRadius, 0.0f, 0.0f));

		//apply axis tilt on planet
		modelMatrix = glm::rotate(modelMatrix, axisTilt, glm::vec3(0.0f, 0.0f, 1.0f));

		//rotation around own axis
		float rotationAngle = time * selfRotationSpeed;
		modelMatrix = glm::rotate(modelMatrix, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

		//scale planet
		modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
	}

	void draw(GLint uniM, GLint texLoc, UnitSphere& sphere) {
		texture.bind();
		glUniform1i(texLoc, 0);
		glUniformMatrix4fv(uniM, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		sphere.m_gpu_geom.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sphere.m_size));
	}


	//member vars
	glm::mat4 modelMatrix;
	Texture texture;
	float scale;
	float orbitalRadius;
	float orbitSpeed;
	float selfRotationSpeed;
	float orbitalInclination;
	float axisTilt;
	Planet* parent; //pointer to parent planet in hierarchy

};

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4()
		: camera(glm::radians(45.f), glm::radians(45.f), 10.0f)
		, aspect(1.0f)
		, rightMouseDown(false)
		, mouseOldX(0.0)
		, mouseOldY(0.0)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)			rightMouseDown = true;
			else if (action == GLFW_RELEASE)	rightMouseDown = false;
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (rightMouseDown) {
			camera.incrementTheta(ypos - mouseOldY);
			camera.incrementPhi(xpos - mouseOldX);
		}
		mouseOldX = xpos;
		mouseOldY = ypos;
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		camera.incrementR(yoffset);
	}
	virtual void windowSizeCallback(int width, int height) {
		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width,  height);
		aspect = float(width)/float(height);
	}

	void viewPipeline(ShaderProgram &sp) {
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);
		//GLint location = glGetUniformLocation(sp, "lightPosition");
		//glm::vec3 light = camera.getPos();
		//glUniform3fv(location, 1, glm::value_ptr(light));
		GLint uniMat = glGetUniformLocation(sp, "V");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(V));
		uniMat = glGetUniformLocation(sp, "P");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(P));
	}
	Camera camera;
private:
	bool rightMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;
};

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453 - Assignment 4");

	GLDebug::enable();

	// CALLBACKS
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);

	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	UnitSphere sphere;
	sphere.generateGeometry();

	//uniform locations
	GLint uniM = glGetUniformLocation(shader, "M");
	GLint texLoc = glGetUniformLocation(shader, "sampler");

	//planet instances e stars
	Planet sun("textures/sun.jpg", 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	Planet earth("textures/earth.jpg", 0.5f, 5.0f, 0.2f, 1.0f, 7.0f, 23.5f, &sun);
	Planet moon("textures/moon.jpg", 0.2f, 1.5f, 1.0f, 1.0f, 5.0f, 6.68f, &earth);
	Texture stars("textures/stars.jpg", GL_LINEAR);
	glm::mat4 M_stars = glm::scale(glm::mat4(1.0f),glm::vec3(50.0f));



	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		//glEnable(GL_FRAMEBUFFER_SRGB); //disabled this and my textures became way clearer, not entirely sure why
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL /*GL_LINE*/);

		shader.use();

		a4->viewPipeline(shader);

		float time = glfwGetTime();

		sun.update(time);
		sun.draw(uniM, texLoc, sphere);
		earth.update(time);
		earth.draw(uniM, texLoc, sphere);
		moon.update(time);
		moon.draw(uniM, texLoc, sphere);

		stars.bind();
		glUniform1i(texLoc, 0);
		glUniformMatrix4fv(uniM, 1, GL_FALSE, glm::value_ptr(M_stars));
		sphere.m_gpu_geom.bind();

		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sphere.m_size));

		//glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		window.swapBuffers();
	}
	glfwTerminate();
	return 0;
}
