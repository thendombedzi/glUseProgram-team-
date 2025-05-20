#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "tiny_obj_loader.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "Objects/EastWall/WindowWall.hpp"

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
    glDisable(GL_CULL_FACE);


    // Load shaders
    shaderProgram = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");

    // Set camera (positioned far back to fit full kiosk wall)
    view = glm::lookAt(
        glm::vec3(0.0f, 3.0f, 10.0f), // camera position
        glm::vec3(0.0f, 0.0f, 0.0f),    // look at origin
        glm::vec3(0.0f, 1.0f, 0.0f)     // up vector
    );

    // Set projection matrix
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.1f, 1000.0f);

    // Build scene
    // WindowWall wall(8,8,0.9,1.5); //default size is 8x8, but we can do this in a scene generator class
    // NorthWall northWall("Objects/NorthWall/N_SWall.obj");

    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;  // <-- enable triangulation

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile("Objects/NorthWall/N_SWall.obj", reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader error: " << reader.Error() << std::endl;
        }
        return -1;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader warning: " << reader.Warning() << std::endl;
    }

    // Get loaded data from reader
    const tinyobj::attrib_t &attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t> &shapes = reader.GetShapes();
    const std::vector<tinyobj::material_t> &materials = reader.GetMaterials();

    // 2. Prepare VAOs/VBOs per shape, with material color
    struct MaterialGroup {
        GLuint VAO, VBO;
        GLsizei vertexCount;
        glm::vec3 color;
    };

    std::vector<MaterialGroup> materialGroups;

    for (size_t s = 0; s < shapes.size(); ++s) {
        const auto& shape = shapes[s];
        std::unordered_map<int, std::vector<float>> materialVertexMap;

        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int mat_id = shape.mesh.material_ids[f];
            int fv = shape.mesh.num_face_vertices[f];

            for (size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[f * fv + v];
                float vx = attrib.vertices[3 * idx.vertex_index + 0];
                float vy = attrib.vertices[3 * idx.vertex_index + 1];
                float vz = attrib.vertices[3 * idx.vertex_index + 2];
                materialVertexMap[mat_id].insert(materialVertexMap[mat_id].end(), {vx, vy, vz});
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

            glm::vec3 color(0.8f); // default gray
            if (mat_id >= 0 && mat_id < (int)materials.size()) {
                color = glm::vec3(
                    materials[mat_id].diffuse[0],
                    materials[mat_id].diffuse[1],
                    materials[mat_id].diffuse[2]
                );
            }

            materialGroups.push_back({VAO, VBO, static_cast<GLsizei>(verts.size() / 3), color});
        }
    }


    // Optional: wireframe for debugging
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // Main loop
    do 
    {
        glfwPollEvents();

            glClearColor(0.15f, 0.15f, 0.15f, 1.0f); // background color
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(shaderProgram);

            // Set up matrices uniforms here:
            GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
            GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
            GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

            glm::mat4 model = glm::mat4(1.0f); // Identity, or adjust if you want transforms
            model = glm::scale(model, glm::vec3(0.15f)); // Shrink it if it's too big
            model = glm::translate(model, glm::vec3(0.5f, 0.0f, 0.0f)); // 
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

            for (const auto& group : materialGroups) {
                GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);

                glBindVertexArray(group.VAO);
                glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);

                // glm::mat4 mirrorModel = glm::mat4(1.0f);
                // mirrorModel = glm::scale(mirrorModel, glm::vec3(-0.15f, -0.15f, -0.15f)); // Negative X for mirror
                // mirrorModel = glm::translate(mirrorModel, glm::vec3(-0.8f, 0.0f, 0.0f)); // Adjust position if needed
                // mirrorModel = glm::rotate(mirrorModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

                // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(mirrorModel));

                // for (const auto& group : materialGroups) {
                //     GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                //     glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);

                //     glBindVertexArray(group.VAO);
                //     glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
                // }
            }
            glfwSwapBuffers(window);
            glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS &&
    glfwWindowShouldClose(window) == 0);

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
