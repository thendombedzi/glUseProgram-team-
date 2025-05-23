#ifndef LIGHTING_MANAGER_HPP
#define LIGHTING_MANAGER_HPP
#include <GL/glew.h>
#include <glm/glm.hpp>

class LightingManager {
public:
    static void upload(GLuint shaderProgram, glm::vec3 FinalColor);

    static glm::vec3 lightDirection;
};

#endif
