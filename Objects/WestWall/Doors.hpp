#ifndef DOORS_HPP
#define DOORS_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Vertex.hpp"

class Doors {
	public:
	std::vector<Vertex> createDoor(float x, float y, float width, float height, float zOffset = 0.11f);
	std::vector<Vertex> createUpperPartOfDoor(float x, float yAboveDoor, float width, float height, float zOffset = 0.11f);
	std::vector<Vertex> createWindowOnDoor(float x, float y, float width, float height, float zOffset = 0.12f);
};


#endif