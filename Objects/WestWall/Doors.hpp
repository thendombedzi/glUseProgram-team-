#ifndef DOORS_HPP
#define DOORS_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "../Vertex.hpp"
#include <vector>

class Doors {
	public:
	std::vector<Vertex> createDualDoors(float wallWidth, float wallHeight,float doorWidth, float doorHeight,float windowStackXOffset, float zOffset);
};

#endif