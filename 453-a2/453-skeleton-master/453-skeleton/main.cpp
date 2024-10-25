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
	glm::vec3 movementVector; //movement vector for the diamond, direction and speed
	bool diamondVisible = true; //visibility of diamond for game logic
};

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader, GameObject& ship, std::vector<std::shared_ptr<GameObject>>& pickups, std::vector<glm::vec3>& initialDiamondPositions, std::vector<glm::vec3>& initialMovementVectors,
		int& score, std::string& scoreValue, bool& gameWin, bool& playerMoving) : shader(shader), ship(ship), pickups(pickups), initialDiamondPositions(initialDiamondPositions), initialMovementVectors(initialMovementVectors),
			score(score), scoreValue(scoreValue), gameWin(gameWin), playerMoving(playerMoving) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			restartGame();
		}
		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			float moveSpeed = 0.001f; //change this value to make ship go faster or slower :D, don't forget to also change the moveSpeed in the rendering loop as well.
			if (key == GLFW_KEY_W || key == GLFW_KEY_UP) {
				ship.position.x += moveSpeed * cos(ship.theta + glm::radians(90.0f));
				ship.position.y += moveSpeed * sin(ship.theta + glm::radians(90.0f));
			}
			if (key == GLFW_KEY_S || key == GLFW_KEY_DOWN) {
				ship.position.x -= moveSpeed * cos(ship.theta + glm::radians(90.0f));
				ship.position.y -= moveSpeed * sin(ship.theta + glm::radians(90.0f));
			}
		}
	}

	virtual void cursorPosCallback(double x_position, double y_position) { //callback to capture current mouse position
		int windowWidth, windowHeight;
		glfwGetWindowSize(glfwGetCurrentContext(), &windowWidth, &windowHeight);

		//converts mouse pos to normalized coordinates (-1 to 1)
		float x = static_cast<float>(x_position) / windowWidth * 2.0f - 1.0f;
		float y = 1.0f - static_cast<float>(y_position) / windowHeight * 2.0f;

		//compute angle between ship pos and mouse pos
		float dx = x - ship.position.x;
		float dy = y - ship.position.y;

		ship.theta = atan2(dy, dx) - glm::radians(90.0f); //update ship theta to face the cursor

	}

	void restartGame() { //restarts everything 
		ship.position = glm::vec3(0.0f, 0.0f, 0.0f);
		ship.theta = 0.0f;
		ship.scale = 0.1f;
		ship.transformationMatrix = glm::mat4(1.0f);

		//resets all diamonds
		for (int i = 0; i < pickups.size(); i++) {
			pickups[i]->diamondVisible = true;
			pickups[i]->position = initialDiamondPositions[i];
			pickups[i]->movementVector = initialMovementVectors[i];
			pickups[i]->theta = 0.0f;
			pickups[i]->theta = 0.1f;
			pickups[i]->transformationMatrix = glm::mat4(1.0f);
		}

		//resets game score
		score = 0;
		scoreValue = "0";
		gameWin = false;
		playerMoving = false;

	}

private:
	ShaderProgram& shader;
	GameObject& ship;
	std::vector<std::shared_ptr<GameObject>> pickups;
	std::vector<glm::vec3>& initialDiamondPositions;
	std::vector<glm::vec3>& initialMovementVectors;
	int& score;
	std::string& scoreValue;
	bool& gameWin;
	bool& playerMoving;
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

//fxn to check for collision between two objects

bool collision(const GameObject& object1, const GameObject& object2){
	float distance = glm::distance(object1.position, object2.position); //gets distance between the two objects colliding
	float collisionDistance = object1.scale + object2.scale;
	return distance < collisionDistance; //if distance < sum of scales, collision is detected
}


int main() {
	int score = 0;
	std::string scoreValue = "0";
	static bool playerMoving = false;
	bool gameWin = false;
	std::string winMessage = "You've collected all the diamonds! Great job! Press [R] to start a new game.";


	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired


	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	//window.setCallbacks(std::make_shared<MyCallbacks>(shader, ship)); // can also update callbacks to new ones

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
	//window.setCallbacks(std::make_shared<MyCallbacks>(shader, ship, pickups));


	//initialize diamonds (pickups)
	std::vector<std::shared_ptr<GameObject>> pickups; //shared pointer to make it easier to reference diamonds objects
	std::vector<glm::vec3> initialDiamondPositions;
	std::vector<glm::vec3> initialMovementVectors;


	//creats 5 diamonds in different places, with different movement vectors 
	for (int i = 0; i < 5; i++) {
		auto diamond = std::make_shared<GameObject>("textures/diamond.png", GL_NEAREST);
		diamond->cgeom = shipGeom();
		diamond->ggeom.setVerts(diamond->cgeom.verts);
		diamond->ggeom.setTexCoords(diamond->cgeom.texCoords);
		diamond->theta = 0.0f;
		diamond->scale = 0.1f;
		diamond->transformationMatrix = glm::mat4(1.0f);

		switch (i) { //set initial position for diamonds, can change to determine where they spawn on startup
			case 0:
				diamond->position = glm::vec3(0.5f, 0.45f, 0.0f);
				break;
			case 1:
				diamond->position = glm::vec3(-0.5f, -0.55f, 0.0f);
				break;
			case 2:
				diamond->position = glm::vec3(-0.7f, 0.9f, 0.0f);
				break;
			case 3:
				diamond->position = glm::vec3(-0.9f, -0.85f, 0.0f);
				break;
			case 4:
				diamond->position = glm::vec3(0.2f, -0.9f, 0.0f);
				break;
		}

		diamond->movementVector = glm::vec3(0.0005f * (i + 1), 0.0005f * (i - 1), 0.0f);

		initialDiamondPositions.push_back(diamond->position);
		initialMovementVectors.push_back(diamond->movementVector);

		pickups.push_back(diamond); //adds diamonds to pickups vector
	}


	window.setCallbacks(std::make_shared<MyCallbacks>(
		shader, ship, pickups, initialDiamondPositions, initialMovementVectors,
		score, scoreValue, gameWin, playerMoving));

	// RENDER LOOP
	while (!window.shouldClose()) { 
		glfwPollEvents();

		//since keyCallback just captures key events, we need to check key states each frame to ensure continuous movement when keys are held down
		float moveSpeed = 0.001f; //need to change movespeed here too
		if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_UP) == GLFW_PRESS) {
			ship.position.x += moveSpeed * cos(ship.theta + glm::radians(90.0f));
			ship.position.y += moveSpeed * sin(ship.theta + glm::radians(90.0f));
		}
		if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_DOWN) == GLFW_PRESS) {
			ship.position.x -= moveSpeed * cos(ship.theta + glm::radians(90.0f));
			ship.position.y -= moveSpeed * sin(ship.theta + glm::radians(90.0f));
		}

		//make sure ship doesn't go off the screen bounds
		ship.position.x = glm::clamp(ship.position.x, -1.0f + ship.scale, 1.0f - ship.scale);
		ship.position.y = glm::clamp(ship.position.y, -1.0f + ship.scale, 1.0f - ship.scale);


		for (auto& diamondPointer : pickups) { //auto& to ensure it references the actual object, not copies
			auto& diamond = *diamondPointer;
			if (!diamond.diamondVisible) continue;

			diamond.position += diamond.movementVector;
			//checks if it hits window edge and invert movement vector
			if (diamond.position.x + diamond.scale >= 1.0f || diamond.position.x - diamond.scale <= -1.0f) {
				diamond.movementVector.x *= -1.0f;
			}
			if (diamond.position.y + diamond.scale >= 1.0f || diamond.position.y - diamond.scale <= -1.0f) {
				diamond.movementVector.y *= -1.0f;
			}
		}

		if (!playerMoving && (ship.position != glm::vec3(0.0f, 0.0f, 0.0f))) {
			playerMoving = true;
		}

		for (auto& diamondPointer : pickups) {
			auto& diamond = *diamondPointer;
			if (!diamond.diamondVisible) continue;

			if (playerMoving && collision(ship, diamond)) {
				diamond.diamondVisible = false;
				ship.scale += 0.05f; //increase ship's size, big number rn for debugging
				score += 1;

				scoreValue = std::to_string(score); //updates score everytime ship picks up a diamond
				if (score == 5) {
					gameWin = true;
				}
			}
		}

		

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

		//draw diamonds
		for (auto& diamondPointer : pickups) {
			auto& diamond = *diamondPointer;
			if (!diamond.diamondVisible) continue;
			updateTransformMatrix(diamond);
			diamond.ggeom.bind();
			diamond.texture.bind();

			glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(diamond.transformationMatrix));
			glDrawArrays(GL_TRIANGLES, 0, 6);
			diamond.texture.unbind();
		}

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
		if (gameWin) { //if loop to display win message
			ImGui::Text("%s", winMessage.c_str());
		}
		else {
			ImGui::Text("Score: %s", scoreValue.c_str()); // Second parameter gets passed into "%d"
		}
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
