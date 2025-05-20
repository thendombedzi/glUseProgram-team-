#ifndef WINDOW_WALL_HPP
#define WINDOW_WALL_HPP

#include "WindowUnit.hpp"
#include <vector>
#include <glm/glm.hpp>

class WindowWall {
public:
    WindowWall(int rows = 8, int cols = 8, float spacingX = 1.1f, float spacingY = 1.6f);
    void draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader);

private:
    std::vector<WindowUnit> windows;
    int rows, cols;
    float spacingX, spacingY;
};

#endif
