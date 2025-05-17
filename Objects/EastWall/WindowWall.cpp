#include "WindowWall.hpp"
#include <glm/gtc/matrix_transform.hpp>

WindowWall::WindowWall(int rows, int cols, float spacingX, float spacingY)
    : rows(rows), cols(cols), spacingX(spacingX), spacingY(spacingY)
{
    float startX = -(cols - 1) * spacingX / 2.0f;
    float startY = -(rows - 1) * spacingY / 2.0f;

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            WindowUnit unit;
            glm::vec3 pos = glm::vec3(startX + x * spacingX, startY + y * spacingY, 0.0f);
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            unit.setTransform(model);
            windows.push_back(unit);
        }
    }
}

void WindowWall::draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader)
{
    for (auto& window : windows) {
        window.draw(view, projection, shader);
    }
}
