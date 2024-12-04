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
		glm::mat4 M = glm::mat4(1.0);
		glm::mat4 V = camera.getView();
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.f);
		//GLint location = glGetUniformLocation(sp, "lightPosition");
		//glm::vec3 light = camera.getPos();
		//glUniform3fv(location, 1, glm::value_ptr(light));
		GLint uniMat = glGetUniformLocation(sp, "M");
		glUniformMatrix4fv(uniMat, 1, GL_FALSE, glm::value_ptr(M));
		uniMat = glGetUniformLocation(sp, "V");
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
	Window window(800, 800, "CPSC 453 - Assignment 3");

	GLDebug::enable();

	Texture sunTexture("textures/sun.jpg", GL_LINEAR);
	Texture earthTexture("textures/earth.jpg", GL_LINEAR);
	Texture moonTexture("textures/moon.jpg", GL_LINEAR);
	Texture starsTexture("textures/stars.jpg", GL_LINEAR);

	// CALLBACKS
	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);

	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	UnitSphere sphere;
	sphere.generateGeometry();

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		glEnable(GL_LINE_SMOOTH);
		//glEnable(GL_FRAMEBUFFER_SRGB);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL /*GL_LINE*/);

		shader.use();

		a4->viewPipeline(shader);

		sphere.m_gpu_geom.bind();
		// Get uniform locations
		GLint uniM = glGetUniformLocation(shader, "M");
		GLint texLoc = glGetUniformLocation(shader, "sampler");

		// Draw Sun
		sunTexture.bind();
		glUniform1i(texLoc, 0);

		glm::mat4 M_sun = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
		glUniformMatrix4fv(uniM, 1, GL_FALSE, glm::value_ptr(M_sun));

		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sphere.m_size));

		// Draw Earth
		earthTexture.bind();
		glUniform1i(texLoc, 0);

		glm::mat4 M_earth = glm::translate(glm::mat4(1.0f), glm::vec3(3.0f, 0.0f, 0.0f));
		M_earth = glm::scale(M_earth, glm::vec3(0.5f));
		glUniformMatrix4fv(uniM, 1, GL_FALSE, glm::value_ptr(M_earth));

		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sphere.m_size));

		// Draw Moon
		moonTexture.bind();
		glUniform1i(texLoc, 0);

		glm::mat4 M_moon = glm::translate(glm::mat4(1.0f), glm::vec3(4.5f, 0.0f, 0.0f));
		M_moon = glm::scale(M_moon, glm::vec3(0.2f));
		glUniformMatrix4fv(uniM, 1, GL_FALSE, glm::value_ptr(M_moon));

		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sphere.m_size));

		// Draw Starry Background
		starsTexture.bind();
		glUniform1i(texLoc, 0);

		glm::mat4 M_stars = glm::scale(glm::mat4(1.0f), glm::vec3(50.0f));
		glUniformMatrix4fv(uniM, 1, GL_FALSE, glm::value_ptr(M_stars));

		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sphere.m_size));

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		window.swapBuffers();
	}
	glfwTerminate();
	return 0;
}
