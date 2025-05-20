#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <unordered_map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "tiny_obj_loader.h"

using namespace glm;
using namespace std;

// Global OpenGL data
GLuint shaderProgram;
glm::mat4 view, projection;

const char *getError()
{
    const char *errorDescription;
    glfwGetError(&errorDescription);
    return errorDescription;
}

inline void startUpGLFW()
{
    glewExperimental = true; // Needed for core profile
    if (!glfwInit())
    {
        throw getError();
    }
}

inline void startUpGLEW()
{
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        throw getError();
    }
}

inline GLFWwindow *setUp()
{
    startUpGLFW();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1400, 1000, "glUseProgram(team)", NULL, NULL);
    if (window == NULL)
    {
        cout << getError() << endl;
        glfwTerminate();
        throw "Failed to open GLFW window.";
    }

    glfwMakeContextCurrent(window);
    startUpGLEW();
    return window;
}

int main()
{
    GLFWwindow *window;
    try
    {
        window = setUp();
    }
    catch (const char *e)
    {
        cout << e << endl;
        return -1;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Load shaders
    shaderProgram = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

    // Set camera
    view = glm::lookAt(
        glm::vec3(10.0f, 3.0f, 0.0f), 
        glm::vec3(0.0f, -1.0f, 0.0f),   
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // Set projection matrix
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.1f, 1000.0f);

    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;

    // Load Carpet
    tinyobj::ObjReader carpet_reader;
    std::string carpet_file = "carpet_1.obj"; // Adjust path
    if (!carpet_reader.ParseFromFile(carpet_file, reader_config)) {
        std::cerr << "TinyObjReader failed to load " << carpet_file << ": " << carpet_reader.Error() << std::endl;
        return -1;
    }
    if (!carpet_reader.Warning().empty()) {
        std::cout << "TinyObjReader warning (carpet): " << carpet_reader.Warning() << std::endl;
    }
    const tinyobj::attrib_t &carpet_attrib = carpet_reader.GetAttrib();
    const std::vector<tinyobj::shape_t> &carpet_shapes = carpet_reader.GetShapes();
    const std::vector<tinyobj::material_t> &carpet_materials = carpet_reader.GetMaterials();

    // Load Roof
    tinyobj::ObjReader roof_reader;
    std::string roof_file = "alt_panels.obj"; // Adjust path
    if (!roof_reader.ParseFromFile(roof_file, reader_config)) {
        std::cerr << "TinyObjReader failed to load " << roof_file << ": " << roof_reader.Error() << std::endl;
        return -1;
    }
    if (!roof_reader.Warning().empty()) {
        std::cout << "TinyObjReader warning (roof): " << roof_reader.Warning() << std::endl;
    }
    const tinyobj::attrib_t &roof_attrib = roof_reader.GetAttrib();
    const std::vector<tinyobj::shape_t> &roof_shapes = roof_reader.GetShapes();
    const std::vector<tinyobj::material_t> &roof_materials = roof_reader.GetMaterials();

    // 2. Prepare VAOs/VBOs per shape, with material color
    struct MaterialGroup {
        GLuint VAO, VBO;
        GLsizei vertexCount;
        glm::vec3 color;
    };

    // Prepare VAOs/VBOs for Carpet
    std::vector<MaterialGroup> carpet_materialGroups;
    for (size_t s = 0; s < carpet_shapes.size(); ++s) {
        // ... (rest of the VAO/VBO generation code for carpet, similar to what you had) ...
        const auto& shape = carpet_shapes[s];
        std::unordered_map<int, std::vector<float>> materialVertexMap;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int mat_id = shape.mesh.material_ids[f];
            int fv = shape.mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[f * fv + v];
                materialVertexMap[mat_id].insert(materialVertexMap[mat_id].end(), {
                    carpet_attrib.vertices[3 * idx.vertex_index + 0],
                    carpet_attrib.vertices[3 * idx.vertex_index + 1],
                    carpet_attrib.vertices[3 * idx.vertex_index + 2]
                });
            }
        }
        for (const auto& [mat_id, verts] : materialVertexMap) {
            GLuint VAO, VBO;
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glm::vec3 color(0.8f);
            if (mat_id >= 0 && mat_id < (int)carpet_materials.size()) {
                color = glm::vec3(carpet_materials[mat_id].diffuse[0], carpet_materials[mat_id].diffuse[1], carpet_materials[mat_id].diffuse[2]);
            }
            carpet_materialGroups.push_back({VAO, VBO, static_cast<GLsizei>(verts.size() / 3), color});
        }
    }

    // Prepare VAOs/VBOs for Roof
    std::vector<MaterialGroup> roof_materialGroups;
    for (size_t s = 0; s < roof_shapes.size(); ++s) {
        // ... (rest of the VAO/VBO generation code for roof, similar to carpet) ...
        const auto& shape = roof_shapes[s];
        std::unordered_map<int, std::vector<float>> materialVertexMap;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int mat_id = shape.mesh.material_ids[f];
            int fv = shape.mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[f * fv + v];
                materialVertexMap[mat_id].insert(materialVertexMap[mat_id].end(), {
                    roof_attrib.vertices[3 * idx.vertex_index + 0],
                    roof_attrib.vertices[3 * idx.vertex_index + 1],
                    roof_attrib.vertices[3 * idx.vertex_index + 2]
                });
            }
        }
        for (const auto& [mat_id, verts] : materialVertexMap) {
            GLuint VAO, VBO;
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glm::vec3 color(0.8f);
            if (mat_id >= 0 && mat_id < (int)roof_materials.size()) {
                color = glm::vec3(roof_materials[mat_id].diffuse[0], roof_materials[mat_id].diffuse[1], roof_materials[mat_id].diffuse[2]);
            }
            roof_materialGroups.push_back({VAO, VBO, static_cast<GLsizei>(verts.size() / 3), color});
        }
    }

    // Main loop
    do
    {
        glfwPollEvents();

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Set up matrices uniforms
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Render Carpet
        glm::mat4 carpetModel = glm::mat4(1.0f);
        carpetModel = glm::scale(carpetModel, glm::vec3(0.15f)); // Adjust scale
        carpetModel = glm::translate(carpetModel, glm::vec3(0.0f, -10.0f, 0.0f)); // Adjust position
        // Add any specific rotations for the carpet if needed
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(carpetModel));
        for (const auto& group : carpet_materialGroups) {
            GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
            glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
            glBindVertexArray(group.VAO);
            glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
        }

        // Render Roof
        glm::mat4 roofModel = glm::mat4(1.0f);
        roofModel = glm::scale(roofModel, glm::vec3(0.15f)); // Adjust scale
        roofModel = glm::translate(roofModel, glm::vec3(0.0f, 10.0f, 0.0f)); // Adjust position
        // Add any specific rotations for the roof if needed
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(roofModel));
        for (const auto& group : roof_materialGroups) {
            GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
            glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
            glBindVertexArray(group.VAO);
            glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}