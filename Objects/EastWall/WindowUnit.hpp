#ifndef WINDOW_UNIT_HPP
#define WINDOW_UNIT_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Verticies.hpp"
#include "../../shader.hpp"

class WindowUnit {
public:
    WindowUnit(); // could optionally take size or color config
    void draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader);
    void setTransform(const glm::mat4& model);

private:
    GLuint glassVAO, glassVBO;
    GLuint frameVAO, frameVBO;
    glm::mat4 model;

};


#endif