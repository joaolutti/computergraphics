#define _USE_MATH_DEFINES
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include <glm/gtx/vector_query.hpp>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "imagebuffer.h"
#include "RayTrace.h"
#include "Scene.h"
#include "Lighting.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


int hasIntersection(Scene const &scene, Ray ray, int skipID){
	for (auto &shape : scene.shapesInScene) {
		Intersection tmp = shape->getIntersection(ray);
		if(
			shape->id != skipID
			&& tmp.numberOfIntersections!=0
			&& glm::distance(tmp.point, ray.origin) > 0.00001
			&& glm::distance(tmp.point, ray.origin) < glm::distance(ray.origin, scene.lightPosition) - 0.01
		){
			return tmp.id;
		}
	}
	return -1;
}

Intersection getClosestIntersection(Scene const &scene, Ray ray, int skipID){ //get the nearest
	Intersection closestIntersection;
	float min = std::numeric_limits<float>::max();
	for(auto &shape : scene.shapesInScene) {
		if(skipID == shape->id) {
			// Sometimes you need to skip certain shapes. Useful to
			// avoid self-intersection. ;)
			continue;
		}
		Intersection p = shape->getIntersection(ray);
		float distance = glm::distance(p.point, ray.origin);
		if(p.numberOfIntersections !=0 && distance < min){
			min = distance;
			closestIntersection = p;
		}
	}
	return closestIntersection;
}


glm::vec3 raytraceSingleRay(Scene const &scene, Ray const &ray, int level, int source_id) {
	// TODO: Part 3: Somewhere in this function you will need to add the code to determine
	//               if a given point is in shadow or not. Think carefully about what parts
	//               of the lighting equation should be used when a point is in shadow.
	// TODO: Part 4: Somewhere in this function you will need to add the code that does reflections and refractions.
	//               NOTE: The ObjectMaterial class already has a parameter to store the object's
	//               reflective properties. Use this parameter + the color coming back from the
	//               reflected array and the color from the phong shading equation.
	Intersection result = getClosestIntersection(scene, ray, source_id); //find intersection

	//return local color on recursion limit
	if (level < 1) {
		PhongReflection phong;
		phong.ray = ray;
		phong.scene = scene;
		phong.material = result.material;
		phong.intersection = result;
		return phong.I();
	}

	PhongReflection phong;
	phong.ray = ray;
	phong.scene = scene;
	phong.material = result.material;
	phong.intersection = result;
	glm::vec3 finalColor(0.0f);

	if(result.numberOfIntersections == 0) return glm::vec3(0, 0, 0); // black;

	

	//part 3
	vec3 lightDirection = glm::normalize(scene.lightPosition - result.point);
	Ray shadowRay(result.point + result.normal * 0.001f, lightDirection);
	int shadowIntersection = hasIntersection(scene, shadowRay, result.id);

	if (shadowIntersection != -1) { //in shadow, only use ambient component of material
		finalColor = phong.Ia();
	}
	else {
		finalColor = phong.I();
	}

	//part 4
	//reflection
	float avgReflection = (phong.material.reflectionStrength.r + phong.material.reflectionStrength.g + phong.material.reflectionStrength.b) / 3.0f;
	if (avgReflection > 0.0f) {
		glm::vec3 D = ray.direction;
		glm::vec3 N = result.normal;
		glm::vec3 reflectionDir = D - 2.0f * glm::dot(D, N) * N; //R = D - 2(N*D)N
		Ray reflectRay(result.point + N * 0.001f, reflectionDir);
		glm::vec3 reflectionColor(0.0f);
		reflectionColor = raytraceSingleRay(scene, reflectRay, level - 1, result.id);
		finalColor = (1.0f - avgReflection) * finalColor + avgReflection * reflectionColor;
	}

	//refraction
	if (phong.material.refractiveIndex > 1.0f) {
		float etai = 1.0f;
		float etat = phong.material.refractiveIndex;
		glm::vec3 N = result.normal;
		float cosi = glm::dot(ray.direction, N);

		//ray inside object
		if (cosi > 0.0f) {
			std::swap(etai, etat);
			N = -N;
			cosi = glm::dot(ray.direction, N);
		}

		float eta = etai / etat;
		float k = 1.0f - eta * eta * (1.0f - cosi * cosi);
		glm::vec3 refractionColor(0.0f);
		glm::vec3 refractionDir;
		if (k < 0.0f) {
			glm::vec3 D = ray.direction;
			refractionDir = D - 2.0f * glm::dot(D, N) * N;
		}
		else {
			refractionDir = eta * ray.direction + (eta * cosi - sqrtf(k)) * N;
		}

		refractionDir = glm::normalize(refractionDir);
		Ray refractionRay(result.point - N * 0.001f, refractionDir);
		refractionColor = raytraceSingleRay(scene, refractionRay, level - 1, result.id);

		finalColor = (1.0f - 0.5f) * finalColor + 0.5f * refractionColor;
	}

	return finalColor;
}

struct RayAndPixel {
	Ray ray;
	int x;
	int y;
};

std::vector<RayAndPixel> getRaysForViewpoint(Scene const &scene, ImageBuffer &image, glm::vec3 viewPoint) {

	//part 1
	std::vector<RayAndPixel> rays;
	int width = image.Width();
	int height = image.Height();
	float aspectRatio = static_cast<float>(width) / height;

	for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
			float fov = M_PI / 2.0f; 
			float scale = tan(fov / 2.0f);
			float u = (2.0f * (x+0.5f) / float(width) - 1.0f) * scale * aspectRatio;
			float v = (2.0f * y / float(height) - 1.0f) * scale;

			//image plane point
			glm::vec3 imagePlanePoint(u, v, -1.0f);

			//generate ray
			glm::vec3 direction = glm::normalize(imagePlanePoint - viewPoint);
			Ray r = Ray(viewPoint, direction);
			rays.push_back({ r, x, y });
		}
	}
	return rays;
}

void raytraceImage(Scene const &scene, ImageBuffer &image, glm::vec3 viewPoint) {
	// Reset the image to the current size of the screen.
	image.Initialize();

	// Get the set of rays to cast for this given image / viewpoint
	std::vector<RayAndPixel> rays = getRaysForViewpoint(scene, image, viewPoint);


	// This loops processes each ray and stores the resulting pixel in the image.
	// final color into the image at the appropriate location.
	//
	// I've written it this way, because if you're keen on this, you can
	// try and parallelize this loop to ensure that your ray tracer makes use
	// of all of your CPU cores
	//
	// Note, if you do this, you will need to be careful about how you render
	// things below too
	// std::for_each(std::begin(rays), std::end(rays), [&] (auto const &r) {
	for (auto const & r : rays) {
		glm::vec3 color = raytraceSingleRay(scene, r.ray, 5, -1);
		image.SetPixel(r.x, r.y, color);
	}
}

// EXAMPLE CALLBACKS
class Assignment5 : public CallbackInterface {

public:
	Assignment5() {
		viewPoint = glm::vec3(0, 0, 1.3); //had to zoom in z
		scene = initScene1();
		raytraceImage(scene, outputImage, viewPoint);
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			shouldQuit = true;
		}

		if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			scene = initScene1();
			raytraceImage(scene, outputImage, viewPoint);
		}

		if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
			scene = initScene2();
			raytraceImage(scene, outputImage, viewPoint);
		}
	}

	bool shouldQuit = false;

	ImageBuffer outputImage;
	Scene scene;
	glm::vec3 viewPoint;

};
// END EXAMPLES


int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();

	// Change your image/screensize here.
	int width = 800;
	int height = 800;
	Window window(width, height, "CPSC 453");

	GLDebug::enable();

	// CALLBACKS
	std::shared_ptr<Assignment5> a5 = std::make_shared<Assignment5>(); // can also update callbacks to new ones
	window.setCallbacks(a5); // can also update callbacks to new ones

	// RENDER LOOP
	while (!window.shouldClose() && !a5->shouldQuit) {
		glfwPollEvents();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		a5->outputImage.Render();

		window.swapBuffers();
	}


	// Save image to file:
	// outpuImage.SaveToFile("foo.png")

	glfwTerminate();
	return 0;
}
