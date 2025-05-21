#ifndef LIGHTING_MANAGER_HPP
#define LIGHTING_MANAGER_HPP
#include <GL/glew.h>
#include <glm/glm.hpp>

class LightingManager {
public:
    static void upload(GLuint shaderProgram);

    static glm::vec3 lightDirection;
    static glm::vec3 lightColor;
};

#endif
