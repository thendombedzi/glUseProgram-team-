#include "Doors.hpp"
#include "../Vertex.hpp"
#include <vector>
#include <glm/glm.hpp>

std::vector<Vertex> Doors::createDoor(float x, float y, float width, float height, float zOffset) {
    float w = width / 2.0f;
    float h = height / 2.0f;

    std::vector<Vertex> vertices;

    // A single quad on the Z = zOffset plane (front-facing)
    glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);

    vertices.push_back({glm::vec3(x - w, y - h, zOffset), normal, glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(x + w, y - h, zOffset), normal, glm::vec2(1.0f, 0.0f)});
    vertices.push_back({glm::vec3(x + w, y + h, zOffset), normal, glm::vec2(1.0f, 1.0f)});

    vertices.push_back({glm::vec3(x - w, y - h, zOffset), normal, glm::vec2(0.0f, 0.0f)});
    vertices.push_back({glm::vec3(x + w, y + h, zOffset), normal, glm::vec2(1.0f, 1.0f)});
    vertices.push_back({glm::vec3(x - w, y + h, zOffset), normal, glm::vec2(0.0f, 1.0f)});

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