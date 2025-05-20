#include "Wall.hpp"
#include "Grids.hpp"
#include "RectangularPrism.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "../Vertex.hpp"

Wall::Wall(float width, float height, float depth, int grooveCols, int grooveRows) {
	RectangularPrism rectPrism;
    auto base = rectPrism.createRectangularPrism(width, height, depth);
    Grids grids;
    auto grooves = grids.createGrooveTile(width, height, grooveCols, grooveRows, 0.05f);

    combinedVertices.insert(combinedVertices.end(), base.begin(), base.end());
    combinedVertices.insert(combinedVertices.end(), grooves.begin(), grooves.end());

    setupMesh();
    model = glm::mat4(1.0f);
}

void Wall::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, combinedVertices.size() * sizeof(Vertex), combinedVertices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    // TexCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Wall::setTransform(const glm::mat4& m) {
    model = m;
}

void Wall::draw(const glm::mat4& view, const glm::mat4& projection, GLuint shader) {
    glUseProgram(shader);

    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Dark wall color, adjust as needed
    glUniform4f(glGetUniformLocation(shader, "objectColor"), 0.2f, 0.2f, 0.2f, 1.0f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, combinedVertices.size());
    glBindVertexArray(0);
}
