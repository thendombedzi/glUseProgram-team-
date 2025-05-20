#ifndef RECTANGULAR_PRISM_HPP
#define RECTANGULAR_PRISM_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Verticies.hpp"
#include "../../shader.hpp"
#include "../Vertex.hpp"
#include <vector>

class RectangularPrism {
	public:
	std::vector<Vertex> createRectangularPrism(float width, float height, float depth);


};

#endif