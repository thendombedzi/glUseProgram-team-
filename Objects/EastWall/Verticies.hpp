#ifndef VERTICES_HPP
#define VERTICES_HPP

#include <vector>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

// Just declarations here
extern std::vector<Vertex> glassVertices;
extern std::vector<Vertex> frameVertices;

#endif
