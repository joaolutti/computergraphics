#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Geometry.h"

class UnitSphere {
public:
	UnitSphere();
	void generateGeometry(int stacks = 32, int slices = 64);

	GPU_Geometry m_gpu_geom;
	size_t m_size;
};
