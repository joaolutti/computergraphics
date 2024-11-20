#include <glad/glad.h>

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
#include "Panel.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

class CurveEditorCallBack : public CallbackInterface {
public:
	CurveEditorCallBack(GLFWwindow* window, GPU_Geometry& cp_point_gpu, GPU_Geometry& cp_line_gpu, GPU_Geometry& curve_gpu)
		: window(window), cp_point_gpu(cp_point_gpu), cp_line_gpu(cp_line_gpu), curve_gpu(curve_gpu),
		currentCurve(0), selectedPoint(-1), dragging(false) {
		glfwGetWindowSize(window, &width, &height);
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) override {
		if (action == GLFW_PRESS) {
			if (key == GLFW_KEY_R) { //R press for resest
				controlPoints.clear();
				selectedPoint = -1;
				updateControlPointGeom();
				updateCurveGeom();
			}
			else if (key == GLFW_KEY_C) { //C press to change curve type (bezier or b-spline)
				currentCurve = (currentCurve + 1) % 2;
				updateCurveGeom();
			}
			else if ((key == GLFW_KEY_BACKSPACE) && selectedPoint >= 0) {
				controlPoints.erase(controlPoints.begin() + selectedPoint);
				selectedPoint = -1;
				updateControlPointGeom();
				updateCurveGeom();
			}
		}
	}

	virtual void mouseButtonCallback(int button, int action, int mods) override {
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		glm::vec3 worldPos = screenNormalized(glm::vec2(x, y));
		if (action == GLFW_PRESS) {
			if (button == GLFW_MOUSE_BUTTON_LEFT) { //left click to either add new point or drag existing point
				int i = getControlPointIndex(worldPos);
				if (i >= 0) {
					selectedPoint = i;
					dragging = true;
				}
				else {
					controlPoints.push_back(worldPos);
					selectedPoint = controlPoints.size() - 1;
					updateControlPointGeom();
					updateCurveGeom();
				}
			}
			else if (button == GLFW_MOUSE_BUTTON_RIGHT) { //right click to delete point
				int i = getControlPointIndex(worldPos);
				if (i >= 0) {
					controlPoints.erase(controlPoints.begin() + i);
					if (selectedPoint == i) {
						selectedPoint = -1;
					}
					else if (selectedPoint > i) {
						selectedPoint--;
					}
					updateControlPointGeom();
					updateCurveGeom();
				}
			}
		}
		else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) { //resets dragging flag
			dragging = false;
			selectedPoint = -1;
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) override {
		if (dragging && selectedPoint >= 0) {
			glm::vec3 worldPos = screenNormalized(glm::vec2(xpos, ypos));
			controlPoints[selectedPoint] = worldPos;
			updateControlPointGeom();
			updateCurveGeom();
		}
	}

	virtual void scrollCallback(double xoffset, double yoffset) override {
		Log::info("ScrollCallback: xoffset={}, yoffset={}", xoffset, yoffset);
	}

	virtual void windowSizeCallback(int width, int height) override {
		CallbackInterface::windowSizeCallback(width, height); // Important, calls glViewport(0, 0, width, height);
		this->width = width;
		this->height = height;
	}

	GPU_Geometry& getCurveGPU() {
		return curve_gpu;
	}

	size_t getControlPointSize() const {
		return cp_point_cpu.verts.size();
	}

	size_t getControlLineSize() const {
		return cp_line_cpu.verts.size();
	}

	size_t getCurveSize() const {
		return curve_cpu.verts.size();
	}

	const std::vector<glm::vec3>& getBSplinePoints() {
		return bspline_curve_cpu.verts;
	}

private:
	GLFWwindow* window;
	int width, height;
	GPU_Geometry& cp_point_gpu;
	GPU_Geometry& cp_line_gpu;
	GPU_Geometry& curve_gpu;
	std::vector<glm::vec3> controlPoints; //stores all control points
	int currentCurve; // 0 for bezier, 1 for b-spline
	int selectedPoint; //index for selected control point
	bool dragging; //flag to indicate if point is being dragged or not

	glm::vec3 cp_point_color = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 selected_point_color = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 cp_line_color = glm::vec3(0.0f, 1.0f, 0.0f);

	CPU_Geometry cp_point_cpu, cp_line_cpu, curve_cpu, bspline_curve_cpu;;



	//fxn to convert screen coordinates to normalized coordinates
	glm::vec3 screenNormalized(const glm::vec2& screenPos) {
		float x = (2.0f * screenPos.x) / width - 1.0f;
		float y = 1.0f - (2.0f * screenPos.y) / height;
		return glm::vec3(x, y, 0.0f);
	}

	int getControlPointIndex(const glm::vec3& worldPos) {
		for (int i = 0; i < controlPoints.size(); i++) {
			if (glm::distance(controlPoints[i], worldPos) < 0.05f) { //might need to adjust this value
				return i;
			}
		}
		return -1;
	}

	void updateControlPointGeom() {
		cp_point_cpu.verts = controlPoints;
		cp_point_cpu.cols = std::vector<glm::vec3>(controlPoints.size(), cp_point_color);
		if (selectedPoint >= 0 && selectedPoint < controlPoints.size()) {
			cp_point_cpu.cols[selectedPoint] = selected_point_color;
		}
		else {
			selectedPoint = -1;
		}
		cp_point_gpu.setVerts(cp_point_cpu.verts);
		cp_point_gpu.setCols(cp_point_cpu.cols);
		cp_line_cpu.verts = controlPoints;
		cp_line_cpu.cols = std::vector<glm::vec3>(controlPoints.size(), cp_line_color);
		cp_line_gpu.setVerts(cp_line_cpu.verts);
		cp_line_gpu.setCols(cp_line_cpu.cols);

	}

	void updateCurveGeom() {
		if (controlPoints.size() < 2) {
			curve_cpu.verts.clear();
			curve_cpu.cols.clear();
			bspline_curve_cpu.verts.clear();
			bspline_curve_cpu.cols.clear();
			curve_gpu.setVerts(curve_cpu.verts);
			curve_gpu.setCols(curve_cpu.cols);
			return;
		}
		if (currentCurve == 0) {
			curve_cpu = deCasteljau(controlPoints);
			bspline_curve_cpu = chaikin(controlPoints, 5);
		}
		else {
			curve_cpu = chaikin(controlPoints, 5);
			bspline_curve_cpu = curve_cpu;
		}
		curve_gpu.setVerts(curve_cpu.verts);
		curve_gpu.setCols(curve_cpu.cols);
	}

	CPU_Geometry deCasteljau(const std::vector<glm::vec3>& points) {
		CPU_Geometry result;
		int numPoints = points.size();
		if (numPoints < 2) return result;

		int n = numPoints - 1; //degree of bezier curve

		for (float u = 0.0f; u <= 1.0f; u += 0.005f) {
			std::vector<glm::vec3> temp = points;
			for (int r = 1; r <= n; r++) {
				for (int i = 0; i <= n - r; i++) {
					temp[i] = (1.0f - u) * temp[i] + u * temp[i + 1];
				}
			}
			result.verts.push_back(temp[0]);
		}
		result.cols = std::vector<glm::vec3>(result.verts.size(), glm::vec3(0.0f));
		return result;
	}


	CPU_Geometry chaikin(const std::vector<glm::vec3>& points, int iterations) {
		std::vector<glm::vec3> temp = points;
		for (int i = 0; i < iterations; i++) {
			std::vector<glm::vec3> newPoints;
			for (int k = 0; k < temp.size() - 1; k++) {
				glm::vec3 Q = 0.75f * temp[k] + 0.25f * temp[k + 1];
				glm::vec3 R = 0.25f * temp[k] + 0.75f * temp[k + 1];
				newPoints.push_back(Q);
				newPoints.push_back(R);
			}
			temp = newPoints;
		}
		CPU_Geometry result;
		result.verts = temp;
		result.cols = std::vector<glm::vec3>(temp.size(), glm::vec3(0.0f));
		return result;
	}

};



// Can swap the callback instead of maintaining a state machine

class TurnTable3DViewerCallBack : public CallbackInterface {

public:
	TurnTable3DViewerCallBack(GLFWwindow* window) : window(window), radius(5.0f), theta(0.0f), phi(0.0f), lastX(0.0), lastY(0.0), leftMousePress(false) {
		glfwGetWindowSize(window, &width, &height);
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (action == GLFW_PRESS) {
			if (key == GLFW_KEY_R) { //camera reset
				radius = 5.0f;
				theta = 0.0f;
				phi = 0.0f;
			}
		}
	}
	virtual void mouseButtonCallback(int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (action == GLFW_PRESS) {
				leftMousePress = true;
				glfwGetCursorPos(window, &lastX, &lastY);

			}
			else if (action == GLFW_RELEASE) {
				leftMousePress = false;
			}
		}
	}
	virtual void cursorPosCallback(double xpos, double ypos) {
		if (leftMousePress) {
			float xOffset = static_cast<float>(xpos - lastX);
			float yOffset = static_cast<float>(ypos - lastY);
			float sens = 0.005f; //sensitivity
			xOffset *= sens;
			yOffset *= sens;

			theta += xOffset;
			phi += yOffset;

			//clamp it before 90 degrees to not flip the camera
			if (phi > glm::radians(89.0f))
				phi = glm::radians(89.0f);
			if (phi < glm::radians(-89.0f))
				phi = glm::radians(-89.0f);

			lastX = xpos;
			lastY = ypos;
		}
	}
	virtual void scrollCallback(double xoffset, double yoffset) {
		radius -= static_cast<float>(yoffset);
		if (radius < 1.0f)
			radius = 1.0f; //small if to not get too close to object
	}
	virtual void windowSizeCallback(int width, int height) {

		// The CallbackInterface::windowSizeCallback will call glViewport for us
		CallbackInterface::windowSizeCallback(width, height);
		this->width = width;
		this->height = height;
	}

	glm::mat4 const getViewMatrix() {
		float x = radius * cos(phi) * sin(theta);
		float y = radius * sin(phi);
		float z = radius * cos(phi) * cos(theta);

		glm::vec3 cameraPos = glm::vec3(x, y, z);
		glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

		return glm::lookAt(cameraPos, target, up);
	}

private:
	GLFWwindow* window;
	int width, height;

	float radius; //distance from origin
	float theta; //rotation around y axis
	float phi; //rotation aroun xz plane

	double lastX, lastY;
	bool leftMousePress;

};


class CurveEditorPanelRenderer : public PanelRendererInterface {
public:
	CurveEditorPanelRenderer()
		: inputText(""), buttonClickCount(0), sliderValue(0.0f),
		dragValue(0.0f), inputValue(0.0f), checkboxValue(false),
		comboSelection(0)
	{
		// Initialize options for the combo box
		options[0] = "Option 1";
		options[1] = "Option 2";
		options[2] = "Option 3";

		// Initialize color (white by default)
		colorValue[0] = 1.0f; // R
		colorValue[1] = 1.0f; // G
		colorValue[2] = 1.0f; // B
	}

	virtual void render() override {
		// Color selector
		ImGui::ColorEdit3("Select Background Color", colorValue); // RGB color selector
		ImGui::Text("Selected Color: R: %.3f, G: %.3f, B: %.3f", colorValue[0], colorValue[1], colorValue[2]);

		// Text input
		ImGui::InputText("Input Text", inputText, IM_ARRAYSIZE(inputText));

		// Display the input text
		ImGui::Text("You entered: %s", inputText);

		// Button
		if (ImGui::Button("Click Me")) {
			buttonClickCount++;
		}
		ImGui::Text("Button clicked %d times", buttonClickCount);

		// Scrollable block
		ImGui::TextWrapped("Scrollable Block:");
		ImGui::BeginChild("ScrollableChild", ImVec2(0, 100), true); // Create a scrollable child
		for (int i = 0; i < 20; i++) {
			ImGui::Text("Item %d", i);
		}
		ImGui::EndChild();

		// Float slider
		ImGui::SliderFloat("Float Slider", &sliderValue, 0.0f, 100.0f, "Slider Value: %.3f");

		// Float drag
		ImGui::DragFloat("Float Drag", &dragValue, 0.1f, 0.0f, 100.0f, "Drag Value: %.3f");

		// Float input
		ImGui::InputFloat("Float Input", &inputValue, 0.1f, 1.0f, "Input Value: %.3f");

		// Checkbox
		ImGui::Checkbox("Enable Feature", &checkboxValue);
		ImGui::Text("Feature Enabled: %s", checkboxValue ? "Yes" : "No");

		// Combo box
		ImGui::Combo("Select an Option", &comboSelection, options, IM_ARRAYSIZE(options));
		ImGui::Text("Selected: %s", options[comboSelection]);

		// Displaying current values
		ImGui::Text("Slider Value: %.3f", sliderValue);
		ImGui::Text("Drag Value: %.3f", dragValue);
		ImGui::Text("Input Value: %.3f", inputValue);
	}

	glm::vec3 getColor() const {
		return glm::vec3(colorValue[0], colorValue[1], colorValue[2]);
	}

private:
	float colorValue[3];  // Array for RGB color values
	char inputText[256];  // Buffer for input text
	int buttonClickCount; // Count button clicks
	float sliderValue;    // Value for float slider
	float dragValue;      // Value for drag input
	float inputValue;     // Value for float input
	bool checkboxValue;   // Value for checkbox
	int comboSelection;   // Index of selected option in combo box
	const char* options[3]; // Options for the combo box
};

CPU_Geometry generateSurfaceOfRevolution(const std::vector<glm::vec3>& curvePoints, int numSlices) {
	CPU_Geometry surface;

	float deltaTheta = 2.0f * glm::pi<float>() / numSlices; //angle increment for each slice

	//iterates over segments of curve
	for (size_t i = 0; i < curvePoints.size() - 1; i++) {
		glm::vec3 p1 = curvePoints[i];
		glm::vec3 p2 = curvePoints[i + 1];

		for (int j = 0; j < numSlices; j++) { //for each slice around  y-axis
			float theta = j * deltaTheta;
			float nextTheta = (j + 1) * deltaTheta;

			//vertices
			glm::vec3 v1 = glm::vec3(p1.x * cos(theta), p1.y, p1.x * sin(theta));
			glm::vec3 v2 = glm::vec3(p2.x * cos(theta), p2.y, p2.x * sin(theta));
			glm::vec3 v3 = glm::vec3(p2.x * cos(nextTheta), p2.y, p2.x * sin(nextTheta));
			glm::vec3 v4 = glm::vec3(p1.x * cos(nextTheta), p1.y, p1.x * sin(nextTheta));

			//2 triangles
			surface.verts.push_back(v1);
			surface.verts.push_back(v2);
			surface.verts.push_back(v3);

			surface.verts.push_back(v1);
			surface.verts.push_back(v3);
			surface.verts.push_back(v4);
		}
	}
	return surface;
}


enum class ViewMode { EDITOR_2D, VIEWER_3D, SURFACE_3D };

int main() {
	Log::debug("Starting main");

	// Initialize GLFW and create window
	glfwInit();
	Window window(800, 800, "CPSC 453: Assignment 3");
	Panel panel(window.getGLFWwindow());
	int width = 800, height = 800;
	bool wireframe = false; //flag for wiref

	// Initialize shaders
	ShaderProgram shader_program_default(
		"shaders/test.vert",
		"shaders/test.frag"
	);

	// Initialize GPU geometries
	GPU_Geometry cp_point_gpu, cp_line_gpu, curve_gpu, surface_gpu;
	CPU_Geometry surface_cpu;

	ViewMode currentView = ViewMode::EDITOR_2D;

	// Create the callback object
	auto curve_editor_callback = std::make_shared<CurveEditorCallBack>(
		window.getGLFWwindow(), cp_point_gpu, cp_line_gpu, curve_gpu);

	auto turntable_callback = std::make_shared<TurnTable3DViewerCallBack>(
		window.getGLFWwindow());


	// Set callbacks
	window.setCallbacks(curve_editor_callback);

	// Panel renderer
	auto curve_editor_panel_renderer = std::make_shared<CurveEditorPanelRenderer>();
	panel.setPanelRenderer(curve_editor_panel_renderer);

	while (!window.shouldClose()) {
		glfwPollEvents();

		static bool keyPress = false;
		static bool keyPressW = false;
		if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_V) == GLFW_PRESS) {
			

			if (!keyPress) {
				keyPress = true;
				if (currentView == ViewMode::EDITOR_2D) {
					currentView = ViewMode::VIEWER_3D;
					window.setCallbacks(turntable_callback);

				}
				else if (currentView == ViewMode::VIEWER_3D) {
					currentView = ViewMode::SURFACE_3D;
					window.setCallbacks(turntable_callback);

					const std::vector<glm::vec3>& bSplinePoints = curve_editor_callback->getBSplinePoints();
					if (!bSplinePoints.empty()) {
						surface_cpu = generateSurfaceOfRevolution(bSplinePoints, 32);
						surface_cpu.cols.resize(surface_cpu.verts.size(), glm::vec3(0.0f, 0.0f, 0.0f));
						surface_gpu.setVerts(surface_cpu.verts);
						surface_gpu.setCols(surface_cpu.cols);
					}
				}
				else {
					currentView = ViewMode::EDITOR_2D;
					window.setCallbacks(curve_editor_callback);
				}
			}
		}
		else {
			keyPress = false;
		}

		if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_W) == GLFW_PRESS) {
			if (!keyPressW) {
				keyPressW = true;
				wireframe = !wireframe;
			}
			else {
				keyPressW = false;
			}
			window.swapBuffers();
		}


		glm::vec3 background_colour = curve_editor_panel_renderer->getColor();

		// Clear buffers
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_FRAMEBUFFER_SRGB);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glClearColor(background_colour.r, background_colour.g, background_colour.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use shader program
		shader_program_default.use();
		// Set up matrices
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view;
		glm::mat4 projection;

		if (currentView == ViewMode::EDITOR_2D) {
			// 2D Orthographic projection
			projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f);
			view = glm::mat4(1.0f);
		}
		else {
			// 3D Perspective projection
			projection = glm::perspective(glm::radians(45.0f), width / (float)height, 0.1f, 100.0f);
			view = turntable_callback->getViewMatrix();
		}

		// Pass matrices to the shade
		GLint modelLoc = glGetUniformLocation(shader_program_default, "model");
		GLint viewLoc = glGetUniformLocation(shader_program_default, "view");
		GLint projLoc = glGetUniformLocation(shader_program_default, "projection");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		if (currentView == ViewMode::EDITOR_2D) {
			// Render control points
			cp_point_gpu.bind();
			glPointSize(15.f);
			glDrawArrays(GL_POINTS, 0, curve_editor_callback->getControlPointSize());

			// Render control polygon
			cp_line_gpu.bind();
			glDrawArrays(GL_LINE_STRIP, 0, curve_editor_callback->getControlLineSize());

			// Render the curve
			curve_gpu.bind();
			glDrawArrays(GL_LINE_STRIP, 0, curve_editor_callback->getCurveSize());
		}
		else if (currentView == ViewMode::VIEWER_3D) {
			// Render the curve in 3D
			curve_gpu.bind();
			glDrawArrays(GL_LINE_STRIP, 0, curve_editor_callback->getCurveSize());
		}
		else if (currentView == ViewMode::SURFACE_3D) {
			// Render the surface of revolution
			surface_gpu.bind();

			if (wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(surface_cpu.verts.size()));

			// Reset to fill mode for other rendering
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Render panel
		glDisable(GL_FRAMEBUFFER_SRGB);
		panel.render();

		// Swap buffers
		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
