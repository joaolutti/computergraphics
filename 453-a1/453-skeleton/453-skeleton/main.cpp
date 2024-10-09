#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

//Sierpinski Triangle fxn
void sierpinski(CPU_Geometry& cpuGeom, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 color, int depth) {

	if (depth == 0) {
		cpuGeom.verts.push_back(a); //top
		cpuGeom.verts.push_back(b); //left
		cpuGeom.verts.push_back(c); //right

		for (int i = 0; i < 3; i++) {
			cpuGeom.cols.push_back(color);
		}
		return;
	}

	glm::vec3 ab = (a + b) / 2.0f;
	glm::vec3 bc = (b + c) / 2.0f;
	glm::vec3 ca = (c + a) / 2.0f;

	sierpinski(cpuGeom, a, ab, ca, glm::vec3(1, 0, 0), depth - 1);
	sierpinski(cpuGeom, ab, b, bc, glm::vec3(0, 1, 0), depth - 1);
	sierpinski(cpuGeom, ca, bc, c, glm::vec3(0, 0, 1), depth - 1);
}

//Pythagoreas Tree fxns

void pythagoras(CPU_Geometry& cpuGeom, glm::vec2 position, float size, float angle, int depth) {
	if (depth == 0) {
		glm::vec2 p0 = position; // bottom left
		glm::vec2 p1 = p0 + glm::vec2(size * std::cos(angle), size * std::sin(angle)); // bottom right
		glm::vec2 p2 = p1 + glm::vec2(-size * std::sin(angle), size * std::cos(angle)); // top right
		glm::vec2 p3 = p0 + glm::vec2(-size * std::sin(angle), size * std::cos(angle)); // top left

		//colors for each corner (debugging)
		glm::vec3 color0 = glm::vec3(1.0f, 0.0f, 0.0f); // red
		glm::vec3 color1 = glm::vec3(0.0f, 1.0f, 0.0f); // green
		glm::vec3 color2 = glm::vec3(0.0f, 0.0f, 1.0f); // blue
		glm::vec3 color3 = glm::vec3(1.0f, 1.0f, 0.0f); // yellow

		// assign corresponding colors
		cpuGeom.verts.push_back(glm::vec3(p0, 0.0f));
		cpuGeom.cols.push_back(color0);
		cpuGeom.verts.push_back(glm::vec3(p1, 0.0f));
		cpuGeom.cols.push_back(color1);
		cpuGeom.verts.push_back(glm::vec3(p2, 0.0f));
		cpuGeom.cols.push_back(color2);


		cpuGeom.verts.push_back(glm::vec3(p0, 0.0f));
		cpuGeom.cols.push_back(color0);
		cpuGeom.verts.push_back(glm::vec3(p2, 0.0f));
		cpuGeom.cols.push_back(color2);
		cpuGeom.verts.push_back(glm::vec3(p3, 0.0f));
		cpuGeom.cols.push_back(color3);
	}
	else {
		glm::vec2 p0 = position;
		glm::vec2 p1 = p0 + glm::vec2(size * std::cos(angle), size * std::sin(angle));
		glm::vec2 p2 = p1 + glm::vec2(-size * std::sin(angle), size * std::cos(angle));
		glm::vec2 p3 = p0 + glm::vec2(-size * std::sin(angle), size * std::cos(angle));

		cpuGeom.verts.push_back(glm::vec3(p0, 0.0f));
		cpuGeom.verts.push_back(glm::vec3(p1, 0.0f));
		cpuGeom.verts.push_back(glm::vec3(p2, 0.0f));

		cpuGeom.verts.push_back(glm::vec3(p0, 0.0f));
		cpuGeom.verts.push_back(glm::vec3(p2, 0.0f));
		cpuGeom.verts.push_back(glm::vec3(p3, 0.0f));

		for (int i = 0; i < 6; i++) {
			cpuGeom.cols.push_back(glm::vec3(1.0f, 1.0f, 1.0f));
		}

		float newSize = size / sqrt(2.0f); //size of next square, for iteration


		//problem i think is here for further iterations
		float left_angle = angle + glm::pi<float>() / 4.0f; //rotates by 45 degrees
		glm::vec2 left_position = p3; //top left corner of the current square as base 
		pythagoras(cpuGeom, left_position, newSize, left_angle, depth - 1);

		float right_angle = angle - glm::pi<float>() / 4.0f;
		glm::vec2 right_position = p2; //top right corner of current square as base
		right_position -= glm::vec2(newSize * std::cos(right_angle), newSize * std::sin(right_angle)); //adjusts right_position to correctly identify the position for rotation, fixed it from branching only left
		pythagoras(cpuGeom, right_position, newSize, right_angle, depth - 1);


	}
}


//Koch Snowflake fxns
void koch(CPU_Geometry& cpuGeom, glm::vec3 start, glm::vec3 end, int depth, glm::vec3 color = glm::vec3(1, 0, 0)) {

	//base case if depth = 0
	cpuGeom.verts.push_back(start);
	cpuGeom.cols.push_back(color);

	if (depth > 0) {
		//calculate points 
		glm::vec3 oneThird = (2.0f * start + end) / 3.0f; //point at 1/3 of the way between start and end of line
		glm::vec3 twoThirds = (start + 2.0f * end) / 3.0f; //point at 2/3 of the way between start and end of line

		//rotate to find peak of triangle
		glm::vec3 direction = (end - start) / 3.0f; //vector for the middle third of the line segment
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-60.0f), glm::vec3(0.0f, 0.0f, 1.0f)); //rotates vector by -60 degress around Z to create the triangle, uses a 4x4 transformation matrix
		glm::vec3 trianglePeak = oneThird + glm::vec3(rotationMatrix * glm::vec4(direction, 0.0f)); 

		//creates new segments for fractal iterations, essentially creates a triangle
		koch(cpuGeom, start, oneThird, depth - 1); //from start to 1/3 of line
		koch(cpuGeom, oneThird, trianglePeak, depth - 1, glm::vec3(0, 0, 1)); //from 1/3 to peak
		koch(cpuGeom, trianglePeak, twoThirds, depth - 1, glm::vec3(0, 0, 1)); //from peak to 2/3
		koch(cpuGeom, twoThirds, end, depth - 1); //from 2/3 to ends
	}
	cpuGeom.verts.push_back(end);
	cpuGeom.cols.push_back(color);
}

//creates the snowflake by starting from 3 koch curves (equilateral triangle)
void generateKoch(CPU_Geometry& cpuGeom, glm::vec3 a, glm::vec3 b, glm::vec3 c, int depth) {
	koch(cpuGeom, a, b, depth);
	koch(cpuGeom, b, c, depth);
	koch(cpuGeom, c, a, depth);
}

//Dragon Curve fxn
void dragon(CPU_Geometry& cpuGeom, glm::vec3 start, glm::vec3 end, int angle, int depth) {

	cpuGeom.verts.push_back(start);
	cpuGeom.cols.push_back(glm::vec3(1, 1, 1));

	if (depth > 0) {
		glm::vec3 direction = (end - start) / float(glm::sqrt(2.0f)); //vector from start to end of line, scaled
		glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(45.0f * angle), glm::vec3(0.0f, 0.0f, 1.0f)); //rotates direction vector, with a 4x4 rotation matrix
		glm::vec3 midpoint = start + glm::vec3(rotationMatrix * glm::vec4(direction, 0.0f));

		dragon(cpuGeom, start, midpoint, -1, depth - 1); //draws first half of curve
		dragon(cpuGeom, midpoint, end, 1, depth - 1); //draws second half of curve
	}
	cpuGeom.verts.push_back(end);
	cpuGeom.cols.push_back(glm::vec3(0, 0, 1));

}

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader, CPU_Geometry& cpuGeom, GPU_Geometry& gpuGeom) : shader(shader), cpuGeom(cpuGeom), gpuGeom(gpuGeom), depth(0), selectedScene(1), drawMode(GL_TRIANGLES) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) override {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}

		if (action == GLFW_PRESS || action == GLFW_REPEAT) {
			if (key == GLFW_KEY_UP && depth < 6) {
				depth++;
				updateGeom();
			}
			if (key == GLFW_KEY_DOWN && depth > 0) {
				depth--;
				updateGeom();
			}
			if (key == GLFW_KEY_1) {
				selectedScene = 1;
				updateGeom();
			}
			else if (key == GLFW_KEY_2) {
				selectedScene = 2;
				updateGeom();
			}
			else if (key == GLFW_KEY_3) {
				selectedScene = 3;
				updateGeom();
			}
			else if (key == GLFW_KEY_4) {
				selectedScene = 4;
				updateGeom();
			}
		}
	}

	void updateGeom() {
		cpuGeom.verts.clear();
		cpuGeom.cols.clear();

		switch (selectedScene) {
			case 1:
				drawMode = GL_TRIANGLES;
				sierpinski(cpuGeom, glm::vec3(0, 0.5, 0), glm::vec3(-0.5, -0.5, 0), glm::vec3(0.5, -0.5, 0), glm::vec3(1, 0, 0), depth);
				break;
			case 2:
				drawMode = GL_TRIANGLES;
				{
					glm::vec2 position(-0.125f, -0.75f);
					float size = 0.25f;
					float angle = 0.0f;
					pythagoras(cpuGeom, position, size, angle, depth);
				}
				break;
			case 3:
				drawMode = GL_LINE_STRIP;
				generateKoch(cpuGeom, glm::vec3(0.5, -0.5, 0), glm::vec3(0, 0.5, 0), glm::vec3(-0.5, -0.5, 0), depth);
				break;
			case 4:
				drawMode = GL_LINE_STRIP;
				dragon(cpuGeom, glm::vec3(-0.5f, 0, 0), glm::vec3(0.5f, 0, 0), -1, depth);
				break;
		}

		gpuGeom.setVerts(cpuGeom.verts);
		gpuGeom.setCols(cpuGeom.cols);
	}
	GLenum getDrawMode() const {
		return drawMode;
	}

private:
	ShaderProgram& shader;
	CPU_Geometry& cpuGeom;
	GPU_Geometry& gpuGeom;
	int depth;
	int selectedScene;
	GLenum drawMode;
};

class MyCallbacks2 : public CallbackInterface {

public:
	MyCallbacks2() {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			std::cout << "called back" << std::endl;
		}
	}
};
// END EXAMPLES


int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired

	GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");


	// GEOMETRY
	CPU_Geometry cpuGeom;
	GPU_Geometry gpuGeom;

	auto callbacks = std::make_shared<MyCallbacks>(shader, cpuGeom, gpuGeom); // can also update callbacks to new ones
	window.setCallbacks(callbacks);

	sierpinski(cpuGeom, glm::vec3(0, 0.5, 0), glm::vec3(-0.577, -0.5, 0), glm::vec3(0.577, -0.5, 0), glm::vec3(1, 0, 0), 0);

	gpuGeom.setVerts(cpuGeom.verts);
	gpuGeom.setCols(cpuGeom.cols);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		shader.use();
		gpuGeom.bind();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(callbacks->getDrawMode(), 0, cpuGeom.verts.size());
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();
	}

	glfwTerminate();
	return 0;
}
