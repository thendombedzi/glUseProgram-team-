#include "RectangularPrism.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../Vertex.hpp"

std::vector<Vertex> RectangularPrism::createRectangularPrism(float width, float height, float depth) {
    float w = width / 2.0f;
    float h = height / 2.0f;
    float d = depth / 2.0f;

    std::vector<Vertex> vertices;

    // FRONT
    vertices.insert(vertices.end(), {
        {{-w, -h,  d}, {0, 0, 1}, {0, 0}},
        {{ w, -h,  d}, {0, 0, 1}, {1, 0}},
        {{ w,  h,  d}, {0, 0, 1}, {1, 1}},

        {{-w, -h,  d}, {0, 0, 1}, {0, 0}},
        {{ w,  h,  d}, {0, 0, 1}, {1, 1}},
        {{-w,  h,  d}, {0, 0, 1}, {0, 1}},
    });

    // BACK
    vertices.insert(vertices.end(), {
        {{ w, -h, -d}, {0, 0, -1}, {0, 0}},
        {{-w, -h, -d}, {0, 0, -1}, {1, 0}},
        {{-w,  h, -d}, {0, 0, -1}, {1, 1}},

        {{ w, -h, -d}, {0, 0, -1}, {0, 0}},
        {{-w,  h, -d}, {0, 0, -1}, {1, 1}},
        {{ w,  h, -d}, {0, 0, -1}, {0, 1}},
    });

    // LEFT
    vertices.insert(vertices.end(), {
        {{-w, -h, -d}, {-1, 0, 0}, {0, 0}},
        {{-w, -h,  d}, {-1, 0, 0}, {1, 0}},
        {{-w,  h,  d}, {-1, 0, 0}, {1, 1}},

        {{-w, -h, -d}, {-1, 0, 0}, {0, 0}},
        {{-w,  h,  d}, {-1, 0, 0}, {1, 1}},
        {{-w,  h, -d}, {-1, 0, 0}, {0, 1}},
    });

    // RIGHT
    vertices.insert(vertices.end(), {
        {{ w, -h,  d}, {1, 0, 0}, {0, 0}},
        {{ w, -h, -d}, {1, 0, 0}, {1, 0}},
        {{ w,  h, -d}, {1, 0, 0}, {1, 1}},

        {{ w, -h,  d}, {1, 0, 0}, {0, 0}},
        {{ w,  h, -d}, {1, 0, 0}, {1, 1}},
        {{ w,  h,  d}, {1, 0, 0}, {0, 1}},
    });

    // TOP
    vertices.insert(vertices.end(), {
        {{-w,  h,  d}, {0, 1, 0}, {0, 0}},
        {{ w,  h,  d}, {0, 1, 0}, {1, 0}},
        {{ w,  h, -d}, {0, 1, 0}, {1, 1}},

        {{-w,  h,  d}, {0, 1, 0}, {0, 0}},
        {{ w,  h, -d}, {0, 1, 0}, {1, 1}},
        {{-w,  h, -d}, {0, 1, 0}, {0, 1}},
    });

    // BOTTOM
    vertices.insert(vertices.end(), {
        {{-w, -h, -d}, {0, -1, 0}, {0, 0}},
        {{ w, -h, -d}, {0, -1, 0}, {1, 0}},
        {{ w, -h,  d}, {0, -1, 0}, {1, 1}},

        {{-w, -h, -d}, {0, -1, 0}, {0, 0}},
        {{ w, -h,  d}, {0, -1, 0}, {1, 1}},
        {{-w, -h,  d}, {0, -1, 0}, {0, 1}},
    });

    return vertices;
}