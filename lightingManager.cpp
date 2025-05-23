#include "LightingManager.hpp"
#include <glm/gtc/type_ptr.hpp>

glm::vec3 LightingManager::lightDirection = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));

void LightingManager::upload(GLuint shaderProgram, glm::vec3 FinalColor) {
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, glm::value_ptr(lightDirection));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(FinalColor));
}
