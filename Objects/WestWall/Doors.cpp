#include "Doors.hpp"
#include "../Vertex.hpp"
#include <glm/glm.hpp>
#include <vector>


std::vector<Vertex> Doors::createDualDoors(
    float wallWidth, float wallHeight,
    float doorWidth, float doorHeight,
    float windowStackXOffset, float zOffset
) {
    std::vector<Vertex> result;

    float yBottom = -wallHeight / 2.0f + doorHeight / 2.0f;

    float leftX = -windowStackXOffset;
    float rightX = windowStackXOffset;

    auto leftDoor = createWindowFrame(leftX, yBottom, doorWidth, doorHeight, zOffset);
    auto rightDoor = createWindowFrame(rightX, yBottom, doorWidth, doorHeight, zOffset);

    result.insert(result.end(), leftDoor.begin(), leftDoor.end());
    result.insert(result.end(), rightDoor.begin(), rightDoor.end());

    return result;
}
