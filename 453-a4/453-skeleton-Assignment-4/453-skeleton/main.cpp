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
		, selfRotationSpeed(selfRotationSpeed), orbitalInclination(glm::radians(orbitalInclination)), axisTilt(glm::radians(axisTilt)), parent(parent), modelMatrix(1.0f), localMatrix(1.0f), worldMatrix(1.0f)
	{}
	void update(float time) {
		localMatrix = glm::mat4(1.0f); //local transformations first
		//scale
		localMatrix = glm::scale(localMatrix, glm::vec3(scale));
		//self-axis rotation
		float rotationAngle = time * selfRotationSpeed;
		localMatrix = glm::rotate(localMatrix, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

		//world pos and orientation
		worldMatrix = glm::mat4(1.0f);


		if (parent) {
			glm::vec3 parentPos = glm::vec3(parent->modelMatrix[3]); //parent's position only
			float orbitAngle = time * orbitSpeed;

			//calculate orbital pos 
			glm::vec3 orbitOffset = glm::vec3(orbitalRadius * cos(orbitAngle), orbitalRadius * sin(orbitAngle) * sin(orbitalInclination), orbitalRadius * sin(orbitAngle) * cos(orbitalInclination));

			//new pos relative to parent
			worldMatrix = glm::translate(glm::mat4(1.0f), parentPos + orbitOffset);
		}
		//extra matrix to keep axis orientation constant while orbiting
		glm::mat4 tiltMatrix = glm::mat4(1.0f);
		tiltMatrix = glm::rotate(tiltMatrix, axisTilt, glm::vec3(0.0f, 0.0f, 1.0f));


		//combine world orientation with local transformations
		modelMatrix = worldMatrix * tiltMatrix * localMatrix;

	}
	void draw(GLint uniM, GLint texLoc, UnitSphere& sphere) {
		texture.bind();
		glUniform1i(texLoc, 0);
		glUniformMatrix4fv(uniM, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		sphere.m_gpu_geom.bind();
		glDrawArrays(GL_TRIANGLES, 0, GLsizei(sphere.m_size));
	}
	//member vars
	glm::mat4 modelMatrix; //transformation matrix
	Texture texture;
	float scale;
	float orbitalRadius;
	float orbitSpeed;
	float selfRotationSpeed;
	float orbitalInclination;
	float axisTilt;
	Planet* parent; //pointer to parent planet in hierarchy

private:
	glm::mat4 localMatrix; //scale and self rotation (local)
	glm::mat4 worldMatrix; //position and axis orientation (world)
};

float animationTime = 0.0f;

//for camera setup
glm::vec3 getPlanetPosition(const Planet& planet) {
	return glm::vec3(planet.modelMatrix[3]);
}

Planet* g_sun = nullptr;
Planet* g_earth = nullptr;
Planet* g_moon = nullptr;

// EXAMPLE CALLBACKS
class Assignment4 : public CallbackInterface {

public:
	Assignment4()
		: camera(glm::radians(45.f), glm::radians(45.f), 10.0f)
		, aspect(1.0f)
		, rightMouseDown(false)
		, mouseOldX(0.0)
		, mouseOldY(0.0)
		, isPaused(false)
		, animationSpeed(1.0f)
	{}

	enum class CameraTarget {
		NONE,
		SUN,
		EARTH,
		MOON
	};

	CameraTarget currentTarget = CameraTarget::NONE;

	bool isPaused;
	float animationSpeed;

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			if (key == GLFW_KEY_SPACE) {
				isPaused = !isPaused;
			}
			else if (key == GLFW_KEY_UP) {
				animationSpeed += 0.1f;
			}
			else if (key == GLFW_KEY_DOWN) {
				animationSpeed = std::max(0.1f, animationSpeed - 0.1f);
			}
			else if (key == GLFW_KEY_R) {
				animationTime = 0.0f;
			}
			else if (key == GLFW_KEY_1) {
				currentTarget = CameraTarget::SUN;
			}
			else if (key == GLFW_KEY_2) {
				currentTarget = CameraTarget::EARTH;
			}
			else if (key == GLFW_KEY_3) {
				currentTarget = CameraTarget::MOON;
			}
		}
	}
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
	GLint uniLightPos = glGetUniformLocation(shader, "lightPos");
	GLint uniViewPos = glGetUniformLocation(shader, "viewPos");
	GLint uniAmbientColor = glGetUniformLocation(shader, "ambientColor");
	GLint uniDiffuseColor = glGetUniformLocation(shader, "diffuseColor");
	GLint uniSpecularColor = glGetUniformLocation(shader, "specularColor");
	GLint uniShininess = glGetUniformLocation(shader, "shininess");
	GLint uniApplyShading = glGetUniformLocation(shader, "applyShading");

	//Planet(texturePath,scale,orbitalRadius,orbitSpeed,selfRotationSpeed,orbitalInclination,axisTilt)
	//planet instances & stars
	Planet sun("textures/sun.jpg", 5.0f, 0.0f, 0.0f, 1.0f, 0.0f, 10.0f);
	Planet earth("textures/earth.jpg", 1.0f, 10.0f, 0.2f, 1.0f, 0.0f, 23.5f, &sun);
	Planet moon("textures/moon.jpg", 0.2f, 2.5f, 0.5f, 0.5f, 45.0f, 6.68f, &earth);
	Texture stars("textures/stars.jpg", GL_LINEAR);
	glm::mat4 M_stars = glm::scale(glm::mat4(1.0f),glm::vec3(50.0f));

	//blobal pointers for camera target
	g_sun = &sun;
	g_earth = &earth;
	g_moon = &moon;


	float lastTime = glfwGetTime();

	//light position in the sun
	glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

	

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
		float deltaTime = time - lastTime;
		lastTime = time;

		if (!a4->isPaused) {
			animationTime += deltaTime * a4->animationSpeed;
		}
		//update light and view positions
		glUniform3fv(uniLightPos, 1, glm::value_ptr(lightPos));
		glm::vec3 viewPos = a4->camera.getPos();
		glUniform3fv(uniViewPos, 1, glm::value_ptr(viewPos));

		sun.update(animationTime);
		earth.update(animationTime);
		moon.update(animationTime);

		//update camera target based on planet
		if (a4->currentTarget == Assignment4::CameraTarget::SUN) {
			a4->camera.setTarget(getPlanetPosition(sun));
		}
		else if (a4->currentTarget == Assignment4::CameraTarget::EARTH) {
			a4->camera.setTarget(getPlanetPosition(earth));
		}
		else if (a4->currentTarget == Assignment4::CameraTarget::MOON) {
			a4->camera.setTarget(getPlanetPosition(moon));
		}

		//draw the sun with no shading
		glUniform1i(uniApplyShading, 0);
		sun.draw(uniM, texLoc, sphere);


		//draw earth with shading
		glUniform1i(uniApplyShading, 1);
		glUniform3f(uniAmbientColor, 0.1f, 0.1f, 0.1f);
		glUniform3f(uniDiffuseColor, 0.5f, 0.5f, 0.5f);
		glUniform3f(uniSpecularColor, 1.0f, 1.0f, 1.0f);
		glUniform1f(uniShininess, 32.0f); //tweak with these to make it cleaner for both moon and earth
		earth.draw(uniM, texLoc, sphere);

		//draw moon with shading
		glUniform1i(uniApplyShading, 1);
		glUniform3f(uniAmbientColor, 0.1f, 0.1f, 0.1f);
		glUniform3f(uniDiffuseColor, 0.3f, 0.3f, 0.3f);
		glUniform3f(uniSpecularColor, 0.5f, 0.5f, 0.5f);
		glUniform1f(uniShininess, 16.0f);
		moon.draw(uniM, texLoc, sphere);

		//draw stars
		glUniform1i(uniApplyShading, 0);
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
