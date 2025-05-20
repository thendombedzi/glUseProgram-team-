#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.hpp"
#include "Camera.h"

using namespace glm;
using namespace std;

const float SCREEN_HEIGHT = 1000;
const float SCREEN_WIDTH = 1000;

// Model structure to hold the loaded furniture
struct Mesh {
    vector<float> vertices;
    vector<float> normals;
    vector<float> texCoords;
    vector<unsigned int> indices;
    GLuint VAO, VBO, EBO, normalVBO, texCoordVBO;
};

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Furniture {
    vector<Mesh> meshes;
    Material material;
    mat4 modelMatrix;
    string name;
};

const char *getError() {
    const char *errorDescription;
    glfwGetError(&errorDescription);
    return errorDescription;
}

inline void startUpGLFW() {
    glewExperimental = true; // Needed for core profile
    if (!glfwInit()) {
        throw getError();
    }
}

inline void startUpGLEW() {
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        throw getError();
    }
}

inline GLFWwindow *setUp() {
    startUpGLFW();
    glfwWindowHint(GLFW_SAMPLES, 4);               // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL
    GLFWwindow *window;                                            
    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Furniture Renderer", NULL, NULL);
    if (window == NULL) {
        cout << getError() << endl;
        glfwTerminate();
        throw "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.";
    }
    glfwMakeContextCurrent(window); // Initialize GLEW
    startUpGLEW();
    return window;
}

// void setupLighting(Shader &shader, int time) {
//     glm::vec3 lightDir = glm::vec3(0.0f, -1.0f, cos(time * 24 * 3.14159 / 360));
//     glm::vec3 lightColor = glm::vec3(0.5f + 0.5f*cos(time * 24 * 3.14159 / 360), 
//                                     0.5f + 0.5f*cos(time * 24 * 3.14159 / 360), 
//                                     0.5f + 0.5f*cos(time * 24 * 3.14159 / 360));

//     shader.use();
//     shader.setVec3("dirLight.direction", lightDir);
//     shader.setVec3("dirLight.ambient", lightColor * 0.3f);
//     shader.setVec3("dirLight.diffuse", lightColor);
//     shader.setVec3("dirLight.specular", lightColor * 0.5f);
// }

// void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
//     static bool firstMouse = true;
//     static float lastX = SCREEN_HEIGHT / 2.0;
//     static float lastY = SCREEN_WIDTH / 2.0;

//     static Camera* camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));

//     if (firstMouse) {
//         lastX = xpos;
//         lastY = ypos;
//         firstMouse = false;
//     }

//     float xoffset = xpos - lastX;
//     float yoffset = lastY - ypos;
//     lastX = xpos;
//     lastY = ypos;

//     camera->processMouseMovement(xoffset, yoffset);
// }

// Process keyboard inputs to move the camera
void processInput(GLFWwindow *window, Camera &camera, float deltaTime, float &currentTime) {
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_W, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_S, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_A, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_D, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_LEFT_SHIFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_LEFT_CONTROL, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS)
        currentTime += 0.1;
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS)
        currentTime -= 0.1;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    static bool wireframe = false;
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        wireframe = !wireframe;
        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

void processMesh(aiMesh *mesh, const aiScene *scene, Mesh &outMesh) {
    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        // Vertices
        outMesh.vertices.push_back(mesh->mVertices[i].x);
        outMesh.vertices.push_back(mesh->mVertices[i].y);
        outMesh.vertices.push_back(mesh->mVertices[i].z);
        
        // Normals
        if (mesh->HasNormals()) {
            outMesh.normals.push_back(mesh->mNormals[i].x);
            outMesh.normals.push_back(mesh->mNormals[i].y);
            outMesh.normals.push_back(mesh->mNormals[i].z);
        }
        
        // Texture coordinates
        if (mesh->mTextureCoords[0]) {
            outMesh.texCoords.push_back(mesh->mTextureCoords[0][i].x);
            outMesh.texCoords.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            outMesh.texCoords.push_back(0.0f);
            outMesh.texCoords.push_back(0.0f);
        }
    }
    
    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            outMesh.indices.push_back(face.mIndices[j]);
        }
    }
    
    // Create and setup OpenGL buffers
    glGenVertexArrays(1, &outMesh.VAO);
    glGenBuffers(1, &outMesh.VBO);
    glGenBuffers(1, &outMesh.normalVBO);
    glGenBuffers(1, &outMesh.texCoordVBO);
    glGenBuffers(1, &outMesh.EBO);
    
    glBindVertexArray(outMesh.VAO);
    
    // Vertices
    glBindBuffer(GL_ARRAY_BUFFER, outMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, outMesh.vertices.size() * sizeof(float), 
                 &outMesh.vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    // Normals
    if (!outMesh.normals.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, outMesh.normalVBO);
        glBufferData(GL_ARRAY_BUFFER, outMesh.normals.size() * sizeof(float), 
                     &outMesh.normals[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    // Texture coordinates
    if (!outMesh.texCoords.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, outMesh.texCoordVBO);
        glBufferData(GL_ARRAY_BUFFER, outMesh.texCoords.size() * sizeof(float), 
                     &outMesh.texCoords[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, outMesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, outMesh.indices.size() * sizeof(unsigned int), 
                 &outMesh.indices[0], GL_STATIC_DRAW);
    
    glBindVertexArray(0);
}

void processNode(aiNode *node, const aiScene *scene, vector<Mesh> &meshes) {
    // Process all node's meshes
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        Mesh newMesh;
        processMesh(mesh, scene, newMesh);
        meshes.push_back(newMesh);
    }
    
    // Process child nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, meshes);
    }
}

Material processMaterial(aiMaterial *material) {
    Material newMaterial;
    
    aiColor3D color;
    float shininess;
    
    // Get material properties
    if (material->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
        newMaterial.ambient = vec3(color.r, color.g, color.b);
    } else {
        newMaterial.ambient = vec3(0.1f, 0.1f, 0.1f);
    }
    
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        newMaterial.diffuse = vec3(color.r, color.g, color.b);
    } else {
        newMaterial.diffuse = vec3(0.5f, 0.5f, 0.5f);
    }
    
    if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        newMaterial.specular = vec3(color.r, color.g, color.b);
    } else {
        newMaterial.specular = vec3(1.0f, 1.0f, 1.0f);
    }
    
    if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        newMaterial.shininess = shininess;
    } else {
        newMaterial.shininess = 32.0f;
    }
    
    return newMaterial;
}

Furniture loadFurniture(const string &path, glm::mat4 modelMatrix = glm::mat4(1.0f)) {
    Furniture furniture;
    
    // Create an Assimp importer
    Assimp::Importer importer;
    
    // Load the model
    const aiScene *scene = importer.ReadFile(path, 
        aiProcess_Triangulate | 
        aiProcess_GenSmoothNormals | 
        aiProcess_FlipUVs | 
        aiProcess_CalcTangentSpace);
    
    // Check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
        return furniture;
    }
    
    // Extract the directory path
    furniture.name = path.substr(path.find_last_of("/\\") + 1);
    
    // Process nodes recursively
    processNode(scene->mRootNode, scene, furniture.meshes);
    
    // Process material if available
    if (scene->HasMaterials()) {
        furniture.material = processMaterial(scene->mMaterials[0]);
    } else {
        // Default material
        furniture.material.ambient = vec3(0.1f, 0.1f, 0.1f);
        furniture.material.diffuse = vec3(0.5f, 0.5f, 0.5f);
        furniture.material.specular = vec3(1.0f, 1.0f, 1.0f);
        furniture.material.shininess = 32.0f;
    }
    
    furniture.modelMatrix = modelMatrix;
    
    return furniture;
}

void renderFurniture(Furniture &furniture, GLuint shaderProgram) {
    // Use the shader program
    glUseProgram(shaderProgram);
    
    // Set material properties
    GLint ambientLoc = glGetUniformLocation(shaderProgram, "material.ambient");
    GLint diffuseLoc = glGetUniformLocation(shaderProgram, "material.diffuse");
    GLint specularLoc = glGetUniformLocation(shaderProgram, "material.specular");
    GLint shininessLoc = glGetUniformLocation(shaderProgram, "material.shininess");
    
    glUniform3fv(ambientLoc, 1, glm::value_ptr(furniture.material.ambient));
    glUniform3fv(diffuseLoc, 1, glm::value_ptr(furniture.material.diffuse));
    glUniform3fv(specularLoc, 1, glm::value_ptr(furniture.material.specular));
    glUniform1f(shininessLoc, furniture.material.shininess);
    
    // Set model matrix
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(furniture.modelMatrix));
    
    // Draw each mesh
    for (Mesh &mesh : furniture.meshes) {
        glBindVertexArray(mesh.VAO);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
}

int main() {
    GLFWwindow *window;
    try {
        window = setUp();
    }
    catch (const char *e) {
        cout << e << endl;
        throw;
    }

    // Create and compile shaders
     GLuint shaderProgram = LoadShaders("vertex.glsl", "fragment.glsl");
    
    // Initialize camera
    Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), // Lower camera position (y=0 instead of y=1)
             glm::vec3(0.0f, 1.0f, 0.0f), 
             -90.0f, // This turns camera to look along negative X-axis
             0.0f, // Slight downward tilt (negative pitch looks down)
             SCREEN_WIDTH, SCREEN_HEIGHT);

             
    glClearColor(0.79f, 0.91f, 0.97f, 1.0f);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetWindowUserPointer(window, &camera);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Load furniture models
    vector<Furniture> furnitureItems;
    
    // Add your furniture models here with appropriate positioning
    // furnitureItems.push_back(loadFurniture("models/blueOrnament.fbx", 
    //                        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))));
    
    glm::mat4 modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f, 0.5f, 0.5f));
// Add this transformation if your model uses a different coordinate system
glm::mat4 baseTransform = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
furnitureItems.push_back(loadFurniture("models/blueOrnament.fbx", 
                         glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f)) * baseTransform * modelMatrix));
    // furnitureItems.push_back(loadFurniture("models/cubic.fbx",
    //                        glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f))));
    
    // Additional furniture items can be added here
    
    float currentDayTime = 12; // Start at noon
    double lastTime = glfwGetTime();

    // Main rendering loop
    do {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // Handle user input
        processInput(window, camera, deltaTime, currentDayTime);

        // Setup lighting
        //setupLighting(shader, currentDayTime);

        // Update view and projection matrices
        glUseProgram(shaderProgram);
        
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(camera.getProjectionMatrix()));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.getPosition()));
        // Render all furniture items
        for (auto &furniture : furnitureItems) {
            renderFurniture(furniture, shaderProgram);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    // Clean up
    for (auto &furniture : furnitureItems) {
        for (auto &mesh : furniture.meshes) {
            glDeleteVertexArrays(1, &mesh.VAO);
            glDeleteBuffers(1, &mesh.VBO);
            glDeleteBuffers(1, &mesh.normalVBO);
            glDeleteBuffers(1, &mesh.texCoordVBO);
            glDeleteBuffers(1, &mesh.EBO);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}