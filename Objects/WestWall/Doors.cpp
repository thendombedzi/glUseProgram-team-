#include "Doors.hpp"
#include "../Vertex.hpp"
#include <vector>
#include <glm/glm.hpp>

#include <GL/glew.h>  // or your GL headers
#include <glm/glm.hpp>

std::vector<Vertex> Doors::createDoor(float x, float y, float width, float height, float zOffset) {
    float w = width / 2.0f;
    float h = height / 2.0f;

    std::vector<Vertex> vertices;

    glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);

    vertices.push_back({glm::vec3(x - w, y - h, zOffset), normal, glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(x + w, y - h, zOffset), normal, glm::vec2(1.0f, 0.0f)});
    vertices.push_back({glm::vec3(x + w, y + h, zOffset), normal, glm::vec2(1.0f, 1.0f)});

    vertices.push_back({glm::vec3(x - w, y - h, zOffset), normal, glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(x + w, y + h, zOffset), normal, glm::vec2(1.0f, 1.0f)});
    vertices.push_back({glm::vec3(x - w, y + h, zOffset), normal, glm::vec2(0.0f, 1.0f)});

    // --- Render the door here ---

    // Generate and bind a VBO
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Assuming a VAO is already bound and your shader uses these attribute locations:
    // Position = 0, Normal = 1, TexCoord = 2
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());

    // Cleanup
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vbo);

    return vertices;
}


std::vector<Vertex> Doors::createUpperPartOfDoor(float x, float yAboveDoor, float width, float height, float zOffset ){
    float w = width / 2.0f;
    float h = height / 2.0f;

    //this should just be a square on top of the door

}

std::vector<Vertex> Doors::createWindowOnDoor(float x, float y, float width, float height, float zOffset ){
    //this should be around 6 glasses above the upper part of the door
}