#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// An example struct for Game Objects.
// You are encouraged to customize this as you see fit.
struct GameObject {
	// Struct's constructor deals with the texture.
	// Also sets default position, theta, scale, and transformationMatrix
	GameObject(std::string texturePath, GLenum textureInterpolation) :
		texture(texturePath, textureInterpolation),
		position(0.0f, 0.0f, 0.0f),
		theta(0),
		scale(1),
		transformationMatrix(1.0f) // This constructor sets it as the identity matrix
	{}

	CPU_Geometry cgeom;
	GPU_Geometry ggeom;
	Texture texture;

	glm::vec3 position;
	float theta; // Object's rotation
	// Alternatively, you could represent rotation via a normalized heading vec:
	// glm::vec3 heading;
	float scale; // Or, alternatively, a glm::vec2 scale;
	glm::mat4 transformationMatrix;
};

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}
	}

	//virtual void cursorPositionCallback(double x, double y) { //callback to capture current mouse position
	//	int windowWidth, windowHeight;
	//	glfwGetWindowSize(glfwGetCurrentContext(), &windowWidth, &windowHeight);

	//	//converts mouse pos to normalized coordinates (-1 to 1)
	//	float x = static_cast<float>(x) / windowWidth * 2.0f - 1.0f;
	//	float y = 1.0f - static_cast<float>(y) / windowHeight * 2.0f;

	//	//compute angle between ship pos and mouse pos
	//	float dx = x - ship.position.x;
	//	float dy = y - ship.position.y;

	//	ship.theta = atan2(dy, dx) - glm::radians(90.0f); //adjust it to match ship's sprite

	//}

private:
	ShaderProgram& shader;
};

CPU_Geometry shipGeom() {
	//float halfWidth = width / 2.0f;
	//float halfHeight = height / 2.0f;
	CPU_Geometry retGeom;
	//// vertices for the spaceship quad
	//retGeom.verts.push_back(glm::vec3(-halfWidth, halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(-halfWidth, -halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(halfWidth, -halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(-halfWidth, halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(halfWidth, -halfHeight, 0.f));
	//retGeom.verts.push_back(glm::vec3(halfWidth, halfHeight, 0.f));

	// For full marks (Part IV), you'll need to use the following vertex coordinates instead.
	// Then, you'd get the correct scale/translation/rotation by passing in uniforms into
	// the vertex shader.

	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
	retGeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));


	// texture coordinates
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
	retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));
	return retGeom;
}

// END EXAMPLES

//fxn to calculate transformation matrix

void updateTransformMatrix(GameObject& obj) {
	glm::mat4 identity = glm::mat4(1.0f);
	glm::mat4 translationMatrix = glm::translate(identity, obj.position);
	glm::mat4 rotationMatrix = glm::rotate(identity, obj.theta, glm::vec3(0, 0, 1));
	glm::mat4 scaleMatrix = glm::scale(identity, glm::vec3(obj.scale, obj.scale, 1.0f));

	obj.transformationMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}


int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	window.setCallbacks(std::make_shared<MyCallbacks>(shader)); // can also update callbacks to new ones

	// GL_NEAREST looks a bit better for low-res pixel art than GL_LINEAR.
	// But for most other cases, you'd want GL_LINEAR interpolation.


	//initialize ship
	GameObject ship("textures/ship.png", GL_NEAREST);
	ship.cgeom = shipGeom();
	ship.ggeom.setVerts(ship.cgeom.verts);
	ship.ggeom.setTexCoords(ship.cgeom.texCoords);
	ship.position = glm::vec3(0.0f, 0.0f, 0.0f);
	ship.theta = 0.0f;
	ship.scale = 0.1f;
	ship.transformationMatrix = glm::mat4(1.0f);


	//initialize diamond
	GameObject diamond("textures/diamond.png", GL_NEAREST);
	diamond.cgeom = shipGeom();
	diamond.ggeom.setVerts(diamond.cgeom.verts);
	diamond.ggeom.setTexCoords(diamond.cgeom.texCoords);
	diamond.position = glm::vec3(0.5f, 0.5f, 0.5f);
	diamond.theta = 0.0f;
	diamond.scale = 0.1f;
	diamond.transformationMatrix = glm::mat4(1.0f);


	


	// RENDER LOOP
	while (!window.shouldClose()) {
		int score;
		glfwPollEvents();

		shader.use();

		GLint currentProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
		GLint uniTrans = glGetUniformLocation(currentProgram, "transformationMatrix");
		if (uniTrans == -1) {
			std::cerr << "Error finding uniform in shader." << std::endl;
		}

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//draw the ship
		updateTransformMatrix(ship);
		ship.ggeom.bind();
		ship.texture.bind();

		glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(ship.transformationMatrix));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		ship.texture.unbind();

		//draw diamond
		updateTransformMatrix(diamond);
		diamond.ggeom.bind();
		diamond.texture.bind();

		glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(diamond.transformationMatrix));
		glDrawArrays(GL_TRIANGLES, 0, 6);
		diamond.texture.unbind();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		// Starting the new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Putting the text-containing window in the top-left of the screen.
		ImGui::SetNextWindowPos(ImVec2(5, 5));

		// Setting flags
		ImGuiWindowFlags textWindowFlags =
			ImGuiWindowFlags_NoMove |				// text "window" should not move
			ImGuiWindowFlags_NoResize |				// should not resize
			ImGuiWindowFlags_NoCollapse |			// should not collapse
			ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
			ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
			ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
			ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
			ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible

		// Begin a new window with these flags. (bool *)0 is the "default" value for its argument.
		ImGui::Begin("scoreText", (bool *)0, textWindowFlags);

		// Scale up text a little, and set its value
		ImGui::SetWindowFontScale(1.5f);
		ImGui::Text("Score: %d", 0); // Second parameter gets passed into "%d"

		// End the window.
		ImGui::End();

		ImGui::Render();	// Render the ImGui window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Some middleware thing

		window.swapBuffers();
	}
	// ImGui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}
