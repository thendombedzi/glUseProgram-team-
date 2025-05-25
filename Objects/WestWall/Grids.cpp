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

std::vector<Vertex> Grids::createGrooveTileGrid(float wallWidth, float wallHeight, float tileThickness) {
    std::vector<Vertex> vertices;

    float tileHeight = 2.0f;
    float tileWidth = 1.0f;
    float horizontalSpacing = 0.1f; 
    float verticalSpacing = 0.1f;  

    float effectiveTileWidth = tileWidth + horizontalSpacing;
    float effectiveTileHeight = tileHeight + verticalSpacing;

    int cols = 7;
    int rows = 8;

    float totalGridWidth = cols * effectiveTileWidth - horizontalSpacing; 
    float totalGridHeight = rows * effectiveTileHeight - verticalSpacing; 

    float offsetX = (wallWidth - totalGridWidth) / 2.0f + 0.5f;
    float offsetY = (wallHeight - totalGridHeight) / 2.0f + 1.0f;

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
           
            float x = col * effectiveTileWidth - wallWidth / 2.0f + offsetX;
            float y = row * effectiveTileHeight - wallHeight / 2.0f + offsetY;

            auto tile = createGrooveTile(x, y, tileWidth, tileHeight, tileThickness);
            vertices.insert(vertices.end(), tile.begin(), tile.end());
        }
    }

    return vertices;
}


