#ifndef GRIDS_HPP
#define GRIDS_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Vertex.hpp"

class Grids {
	public:
	std::vector<Vertex> createGrooveTile(float x, float y, float width, float height, float zOffset = 0.11f);
};

#endif