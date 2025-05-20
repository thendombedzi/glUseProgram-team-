#ifndef WALL_HPP
#define WALL_HPP

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "../Vertex.hpp"

class Wall {
public:
    Wall(float width, float height, float depth, int grooveCols, int grooveRows);

    void setTransform(const glm::mat4& model);
    void draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader);

private:
    GLuint VAO, VBO;
    glm::mat4 model;
    std::vector<Vertex> combinedVertices;

    void setupMesh();
};

#endif
