#include "Grids.hpp"
#include <glm/glm.hpp>
#include <vector>
#include "../Vertex.hpp"

std::vector<Vertex> Grids::createGrooveTile(float x, float y, float width, float height, float zOffset) {
    float w = width / 2.0f;
    float h = height / 2.0f;

    std::vector<Vertex> tile = {
        {{x - w, y - h, zOffset}, {0, 0, 1}, {0, 0}},
        {{x + w, y - h, zOffset}, {0, 0, 1}, {1, 0}},
        {{x + w, y + h, zOffset}, {0, 0, 1}, {1, 1}},

        {{x - w, y - h, zOffset}, {0, 0, 1}, {0, 0}},
        {{x + w, y + h, zOffset}, {0, 0, 1}, {1, 1}},
        {{x - w, y + h, zOffset}, {0, 0, 1}, {0, 1}},
    };

    return tile;
}
