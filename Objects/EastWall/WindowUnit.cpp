#include "WindowUnit.hpp"
#include "Verticies.hpp"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

WindowUnit::WindowUnit() {
    // === GLASS ===
    glGenVertexArrays(1, &glassVAO);
    glGenBuffers(1, &glassVBO);
    glBindVertexArray(glassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, glassVBO);
    glBufferData(GL_ARRAY_BUFFER, glassVertices.size() * sizeof(Vertex), glassVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // === FRAME ===
    glGenVertexArrays(1, &frameVAO);
    glGenBuffers(1, &frameVBO);
    glBindVertexArray(frameVAO);
    glBindBuffer(GL_ARRAY_BUFFER, frameVBO);
    glBufferData(GL_ARRAY_BUFFER, frameVertices.size() * sizeof(Vertex), frameVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    model = glm::mat4(1.0f);
}

void WindowUnit::setTransform(const glm::mat4& model) {
    this->model = model;
}

void WindowUnit::draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader) {
    glUseProgram(shader);

    GLuint viewLoc = glGetUniformLocation(shader, "view");
    GLuint projLoc = glGetUniformLocation(shader, "projection");
    GLuint modelLoc = glGetUniformLocation(shader, "model");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Draw glass
    glUniform4f(glGetUniformLocation(shader, "objectColor"), 0.3f, 0.6f, 0.8f, 0.4f); // light blue, semi-transparent
    glBindVertexArray(glassVAO);
    glDrawArrays(GL_TRIANGLES, 0, glassVertices.size());

    // Draw frame
    glUniform4f(glGetUniformLocation(shader, "objectColor"), 0.9f, 0.9f, 0.9f, 1.0f); // white wood (birch)
    glBindVertexArray(frameVAO);
    glDrawArrays(GL_TRIANGLES, 0, frameVertices.size());

    glBindVertexArray(0);
}

