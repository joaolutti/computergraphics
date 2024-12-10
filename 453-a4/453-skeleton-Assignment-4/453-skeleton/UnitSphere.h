#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Geometry.h"

class UnitSphere {
public:
	UnitSphere();
	void generateGeometry(int stacks = 16, int slices = 32);

	GPU_Geometry m_gpu_geom;
	size_t m_size;
};
