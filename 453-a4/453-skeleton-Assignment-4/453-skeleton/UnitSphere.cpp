#include "UnitSphere.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

UnitSphere::UnitSphere() : m_size(0) {}

void UnitSphere::generateGeometry(int stacks, int slices) {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;

	float pi = glm::pi<float>();

	//generate vertices
	for (int i = 0; i <= stacks; i++) {
		float v = float(i) / stacks;
		float phi = v * pi;

		for (int j = 0; j <= slices; j++) {
			float u = (float)j / slices;
			float theta = u * (2 * pi);
			float x = cos(theta) * sin(phi);
			float y = cos(phi);
			float z = sin(theta) * sin(phi);

			glm::vec3 position = glm::vec3(x, y, z);
			glm::vec3 normal = glm::normalize(position);
			glm::vec2 texCoord = glm::vec2(u, 1.0f-v);
			positions.push_back(position);
			normals.push_back(normal);
			texCoords.push_back(texCoord);

		}
	}
	std::vector<glm::vec3> finalPositions;
	std::vector<glm::vec3> finalNormals;
	std::vector<glm::vec2> finalTexCoords;

	//generate triangles
	for (int i = 0; i < stacks; i++) {
		for (int j = 0; j < slices; j++) {
			int row1 = i * (slices + 1);
			int row2 = (i + 1) * (slices + 1);

			//1st triangle
			finalPositions.push_back(positions[row1 + j]);
			finalNormals.push_back(normals[row1 + j]);
			finalTexCoords.push_back(texCoords[row1 + j]);

			finalPositions.push_back(positions[row2 + j]);
			finalNormals.push_back(normals[row2 + j]);
			finalTexCoords.push_back(texCoords[row2 + j]);

			finalPositions.push_back(positions[row1 + j + 1]);
			finalNormals.push_back(normals[row1 + j + 1]);
			finalTexCoords.push_back(texCoords[row1 + j + 1]);

			//2nd triangle
			finalPositions.push_back(positions[row1 + j + 1]);
			finalNormals.push_back(normals[row1 + j + 1]);
			finalTexCoords.push_back(texCoords[row1 + j + 1]);

			finalPositions.push_back(positions[row2 + j]);
			finalNormals.push_back(normals[row2 + j]);
			finalTexCoords.push_back(texCoords[row2 + j]);

			finalPositions.push_back(positions[row2 + j + 1]);
			finalNormals.push_back(normals[row2 + j + 1]);
			finalTexCoords.push_back(texCoords[row2 + j + 1]);
		}
	}

	m_gpu_geom.bind();
	m_gpu_geom.setVerts(finalPositions);
	m_gpu_geom.setNormals(finalNormals);
	m_gpu_geom.setTexCoords(finalTexCoords);

	m_size = finalPositions.size();

}
