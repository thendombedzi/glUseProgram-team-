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
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shader.hpp"
#include "tiny_obj_loader.h"
#include "Objects/EastWall/WindowWall.hpp"
#include "Objects/WestWall/Wall.hpp"
#include "lightingManager.hpp"

using namespace glm;
using namespace std;

// Global OpenGL data
GLuint shaderProgram;
glm::mat4 view, projection;

// Drone position variables
glm::vec3 dronePosition = glm::vec3(0.0f, 20.0f, 0.0f);
float droneYaw = 0.0f;   // rotation around Y axis
float dronePitch = 0.0f; // rotation around X axis
float droneRoll = 0.0f; // rotation around Z axis

// Camera zoom variables
float initialFOV = 50.0f;
float currentFOV = initialFOV;
float zoomSpeed = 5.0f; 
float minFOV = 1.0f;
float maxFOV = 90.0f;

// Camera filter:
int filterMode = 0;

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
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1400, 1000, "glUseProgram(team)", NULL, NULL);
    if (window == NULL) {
        cout << getError() << endl;
        glfwTerminate();
        throw "Failed to open GLFW window.";
    }

    glfwMakeContextCurrent(window);
    startUpGLEW();
    return window;
}

// MaterialGroup struct definition
struct MaterialGroup {
    GLuint VAO;
    GLuint VBO;
    GLsizei vertexCount;
    glm::vec3 color;
    GLuint textureID; // New: Texture ID for this material group
    bool hasTexture;  // New: Flag to indicate if this group has a texture
    float alpha = 1.0f; // New
};

struct InterleavedVertex {
    float px, py, pz;  // Position
    float nx, ny, nz;  // Normal
    float tx, ty;     // Texture coordinates
};

GLuint loadTexture(const std::string &filepath)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture wrapping and filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BIT, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char *data = stbi_load(filepath.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        // THIS IS THE LINE TO ADD/CHECK
        std::cerr << "Failed to load texture: " << filepath << " Reason: " << stbi_failure_reason() << std::endl;
        // Optionally load a default white texture if loading fails
        unsigned char defaultData[] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}; // Small white default texture data
        // For a 1x1 white texture, you can use:
        // unsigned char defaultData[] = {255, 255, 255, 255}; // RGBA
        // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, defaultData);
        // No mipmaps needed for 1x1
    }
    stbi_image_free(data);
    return textureID;
}

std::vector<MaterialGroup> loadObjModel(const std::string& filename, const tinyobj::ObjReaderConfig& config) {
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filename, config)) {
        std::cerr << "TinyObjReader failed to load " << filename << ": " << reader.Error() << std::endl;
        return {};
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader warning (" << filename << "): " << reader.Warning() << std::endl;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    const auto& materials = reader.GetMaterials();

    std::vector<MaterialGroup> materialGroups;

    for (const auto& shape : shapes) {
        std::unordered_map<int, std::vector<InterleavedVertex>> materialVertexMap;

        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int fv = shape.mesh.num_face_vertices[f];
            int mat_id = shape.mesh.material_ids[f];

            for (size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                InterleavedVertex vertex;
                vertex.px = attrib.vertices[3 * idx.vertex_index + 0];
                vertex.py = attrib.vertices[3 * idx.vertex_index + 1];
                vertex.pz = attrib.vertices[3 * idx.vertex_index + 2];

                if (!attrib.normals.empty() && idx.normal_index >= 0) {
                    vertex.nx = attrib.normals[3 * idx.normal_index + 0];
                    vertex.ny = attrib.normals[3 * idx.normal_index + 1];
                    vertex.nz = attrib.normals[3 * idx.normal_index + 2];
                } else {
                    vertex.nx = 0.0f; vertex.ny = 0.0f; vertex.nz = 1.0f; // fallback
                }

                // Get texture coordinates
                if (!attrib.texcoords.empty() && idx.texcoord_index >= 0)
                {
                    vertex.tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    vertex.ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                }
                else
                {
                    vertex.tx = 0.0f; // Default texture coordinates
                    vertex.ty = 0.0f;
                }

                materialVertexMap[mat_id].push_back(vertex);
            }

            index_offset += fv;
        }

        for (const auto& [mat_id, verts] : materialVertexMap) {
            GLuint VAO, VBO;
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(InterleavedVertex), verts.data(), GL_STATIC_DRAW);

            // Position: location 0
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(InterleavedVertex), (void*)0);
            glEnableVertexAttribArray(0);

            // Normal: location 1
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(InterleavedVertex), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // Texture coordinates: location 2
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(InterleavedVertex), (void *)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            // Color from material or default
            glm::vec3 color(0.8f); // default
            GLuint textureID = 0;  // default to no texture
            bool hasTexture = false;

            // Extract alpha from material
            float alpha = 1.0f;

            if (mat_id >= 0 && mat_id < (int)materials.size()) {
                const auto& m = materials[mat_id];
                color = glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
                alpha = m.dissolve;

                // Load texture if diffuse texture map is specified
                if (!m.diffuse_texname.empty())
                {
                    std::string texturePath = m.diffuse_texname; // tinyobjloader should handle relative paths from where the .obj is loaded
                    textureID = loadTexture(texturePath);
                    hasTexture = true;
                }
            }

            materialGroups.push_back({VAO, VBO, static_cast<GLsizei>(verts.size()), color, textureID, hasTexture, alpha});
        }
    }

    return materialGroups;
}

// Structure to manage furniture with position, rotation and scale
struct Furniture {
    std::vector<MaterialGroup> materialGroups;
    glm::vec3 position;
    glm::vec3 rotation; // in degrees
    glm::vec3 scale;
    
    void render(GLuint shaderProgram) const {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);
        
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Get uniform location for hasTexture
        GLint hasTextureLoc = glGetUniformLocation(shaderProgram, "hasTexture");
        if (hasTextureLoc == -1) {
            std::cerr << "Uniform 'hasTexture' not found!" << std::endl;
        }
        
        for (const auto& group : materialGroups) {
            if (group.hasTexture)
            {
                glUniform1i(hasTextureLoc, 1); // Tell shader to use texture
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, group.textureID);
                GLint textureSamplerLoc = glGetUniformLocation(shaderProgram, "textureSampler");
                glUniform1i(textureSamplerLoc, 0); // Set to texture unit 0
            }
            else
            {
                glUniform1i(hasTextureLoc, 0); // Tell shader to use color
                GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
            }

            glBindVertexArray(group.VAO);
            glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
        }
    }
};

// Render functions
void renderCarpet(vector<MaterialGroup> carpet0_materialGroups, vector<MaterialGroup> carpet1_materialGroups, vector<MaterialGroup> carpet2_materialGroups, GLuint modelLoc) {
    const float CARPET_SCALE = 0.25f;
    const float CARPET_Y_POSITION = 12.0f; // Common Y position for all carpets (assuming flat on floor)
    const float CARPET_ROTATION_Y = 270.0f; // Common rotation for all carpets

    const int numRows = 12;
    const float zOffset = 14.0f;
    const float initialZ = -90.0f;

    // Fixed X positions for left, middle, right carpets
    std::vector<float> xPositions = { -34.0f, -10.0f, 12.5f };

    // Carpet data in order 0, 1, 2
    std::vector<std::vector<MaterialGroup>> carpetGroups = {
        carpet0_materialGroups,
        carpet1_materialGroups,
        carpet2_materialGroups
    };

    // All 6 permutations of 3 carpets
    std::vector<std::vector<int>> carpetPatterns = {
        {0, 1, 2},
        {2, 1, 0},
        {1, 0, 2},
        {2, 0, 1},
        {1, 2, 0},
        {0, 2, 1}
    };

    for (int i = 0; i < numRows; ++i) {
        const std::vector<int>& pattern = carpetPatterns[i % carpetPatterns.size()];

        for (int j = 0; j < 3; ++j) {
            int carpetIndex = pattern[j];
            float x = xPositions[j];
            float z = 0.0f;

            if(carpetIndex == 0)
                z = initialZ + (i * zOffset);
            else
                z = initialZ-0.6f + (i * zOffset);

            const auto& groups = carpetGroups[carpetIndex];
            if (groups.empty()) continue;

            glm::mat4 carpetModel = glm::mat4(1.0f);
            carpetModel = glm::scale(carpetModel, glm::vec3(CARPET_SCALE));
            carpetModel = glm::translate(carpetModel, glm::vec3(x, CARPET_Y_POSITION, z));
            carpetModel = glm::rotate(carpetModel, glm::radians(CARPET_ROTATION_Y), glm::vec3(0.0f, 1.0f, 0.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(carpetModel));
            for (const auto& group : groups) {
                GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 0.5f);                    
                glBindVertexArray(group.VAO);
                glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
            }
        }
    }
}

void renderRoof(vector<MaterialGroup> roof_materialGroups, GLuint modelLoc) {
    if (!roof_materialGroups.empty()) {

        // Define duplication parameters
        int numDuplicates = 4; // Number of roof panels you want
        float zOffset = 15.0f; 

        for (int i = 0; i < numDuplicates; ++i) {
            glm::mat4 roofModel = glm::mat4(1.0f);

            // Apply base transformations (from your original code)
            roofModel = glm::scale(roofModel, glm::vec3(0.8f)); // Adjust scale
            roofModel = glm::translate(roofModel, glm::vec3(-2.5f, 25.0f, 25.0f)); // Adjust base position
            roofModel = glm::rotate(roofModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face the right direction

            // Apply duplication offset
            // For each duplicate, translate it further along the X-axis
            roofModel = glm::translate(roofModel, glm::vec3(i * zOffset,0.0f ,0.0f ));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(roofModel));

            for (const auto& group : roof_materialGroups) {
                GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
                glBindVertexArray(group.VAO);
                glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
            }
        }
    }
}

void renderNSWalls(vector<MaterialGroup> northwall_materialGroups, GLuint modelLoc) {
    // --- Render North Wall --
    if (!northwall_materialGroups.empty()) {

        glm::mat4 northWallModel = glm::mat4(1.0f);
        northWallModel = glm::scale(northWallModel, glm::vec3(0.5f)); 
        northWallModel = glm::translate(northWallModel, glm::vec3(0.5f, 0.0f, 0.0f));
        northWallModel = glm::rotate(northWallModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(northWallModel));
        for (const auto& group : northwall_materialGroups) {
            GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
            glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
            glBindVertexArray(group.VAO);
            glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
        }
    }

    // --- Render South Wall ---
    if (!northwall_materialGroups.empty()) {
        glm::mat4 southWallModel = glm::mat4(1.0f);
        
        southWallModel = glm::scale(southWallModel, glm::vec3(-1.0f, 1.0f, 1.0f));
        southWallModel = glm::scale(southWallModel, glm::vec3(0.5f));
        southWallModel = glm::translate(southWallModel, glm::vec3(0.5f, 0.0f, 0.0f)); 
        southWallModel = glm::rotate(southWallModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        southWallModel = glm::translate(southWallModel, glm::vec3(-10.0f, 0.0f, 0.0f)); 

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(southWallModel));

        for (const auto& group : northwall_materialGroups) {
            GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
            glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
            glBindVertexArray(group.VAO);
            glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
        }
    }
}

void renderWEWalls(float groundLevel, vector<MaterialGroup> westwall_materialGroups, GLuint modelLoc) {
    if (!westwall_materialGroups.empty())
    {
        int cols = 10; 
        int rows = 9; 

        float yOffset = 2.0f; 
        float xOffset = 2.0f; 

        for (int i = 0; i < rows; ++i) {
            
            glm::mat4 westWallModel = glm::mat4(1.0f);
            westWallModel = glm::scale(westWallModel, glm::vec3(1.0f)); // Adjust scale
            westWallModel = glm::rotate(westWallModel, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face the right direction

            westWallModel = glm::translate(westWallModel, glm::vec3(-22.5f, groundLevel+0.5 +(i * yOffset) ,-5.0f ));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(westWallModel));
            for (const auto& group : westwall_materialGroups) {
                GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
                glBindVertexArray(group.VAO);
                glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
            }

            for(int j = 0; j < cols; j++){
                glm::mat4 westWallModel = glm::mat4(1.0f);
                westWallModel = glm::scale(westWallModel, glm::vec3(1.0f)); // Adjust scale
                westWallModel = glm::translate(westWallModel, glm::vec3(0.0f, 0.0f, 0.0f)); // Adjust position
                westWallModel = glm::rotate(westWallModel, glm::radians(270.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face the right direction

                westWallModel = glm::translate(westWallModel, glm::vec3(-22.5f, groundLevel+0.5 + (i * yOffset),-5.0f +(j * xOffset)));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(westWallModel));
                for (const auto& group : westwall_materialGroups) {
                    GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                    glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
                    glBindVertexArray(group.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
                }
            }
        }

        //Render east wall glass
        {
            int cols = 2; 
            int rows = 6; 

            float yOffset = 2.0f; 
            float xOffset = 2.0f; 

            for (int i = 0; i < rows; ++i) {
              
                glm::mat4 eastWallModel = glm::mat4(1.0f);
                eastWallModel = glm::scale(eastWallModel, glm::vec3(1.0f)); // Adjust scale
                eastWallModel = glm::rotate(eastWallModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face the right direction

                eastWallModel = glm::translate(eastWallModel, glm::vec3(-19.0f, 10.0f +(i * yOffset) ,-9.0f ));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eastWallModel));
                for (const auto& group : westwall_materialGroups) {
                    GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                    glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
                    glBindVertexArray(group.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
                }

                for(int j = 0; j < cols; j++){
                    glm::mat4 eastWallModel = glm::mat4(1.0f);
                    eastWallModel = glm::scale(eastWallModel, glm::vec3(1.0f)); // Adjust scale
                    eastWallModel = glm::translate(eastWallModel, glm::vec3(0.0f, 0.0f, 0.0f)); // Adjust position
                    eastWallModel = glm::rotate(eastWallModel, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face the right direction

                    eastWallModel = glm::translate(eastWallModel, glm::vec3(-19.0f, 10.0f + (i * yOffset), -9.0f+(j * xOffset)));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eastWallModel));
                    for (const auto& group : westwall_materialGroups) {
                        GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                        glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
                        glBindVertexArray(group.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
                    }
                }
            }
        }


        {
            int cols = 2; 
            int rows = 6; 

            float yOffset = 2.0f; 
            float xOffset = 2.0f; 

            for (int i = 0; i < rows; ++i) {
              
                glm::mat4 eastWallModel2 = glm::mat4(1.0f);
                eastWallModel2 = glm::scale(eastWallModel2, glm::vec3(1.0f)); // Adjust scale
                eastWallModel2 = glm::rotate(eastWallModel2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face the right direction

                eastWallModel2 = glm::translate(eastWallModel2, glm::vec3(-19.0f, 10.0f +(i * yOffset),2.5f ));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eastWallModel2));
                for (const auto& group : westwall_materialGroups) {
                    GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                    glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
                    glBindVertexArray(group.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
                }

                for(int j = 0; j < cols; j++){
                    glm::mat4 eastWallModel2 = glm::mat4(1.0f);
                    eastWallModel2 = glm::scale(eastWallModel2, glm::vec3(1.0f)); // Adjust scale
                    eastWallModel2 = glm::translate(eastWallModel2, glm::vec3(0.0f, 0.0f, 0.0f)); // Adjust position
                    eastWallModel2 = glm::rotate(eastWallModel2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate to face the right direction

                    eastWallModel2 = glm::translate(eastWallModel2, glm::vec3(-19.0f, 10.0f + (i * yOffset), 2.5f+(j * xOffset)));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(eastWallModel2));
                    for (const auto& group : westwall_materialGroups) {
                        GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                        glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
                        glBindVertexArray(group.VAO);
                        glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
                    }
                }
            }
        }
    }
}

void renderFurniture(vector<Furniture> furnitureCollection) {
    for (const auto& furniture : furnitureCollection) {
        furniture.render(shaderProgram);
    }
}

void renderDrone(vector<MaterialGroup> drone_materialGroups, GLuint modelLoc) {
    if (!drone_materialGroups.empty()) {
        // glm::mat4 droneModel = glm::mat4(1.0f);
        // droneModel = glm::translate(droneModel, dronePosition); // Adjust position
        // droneModel = glm::scale(droneModel, glm::vec3(0.4f)); // Adjust scale
        // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(droneModel));
        for (const auto& group : drone_materialGroups) {
            GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
            glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
            glBindVertexArray(group.VAO);
            glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
        }
    }
}

int main() {
    GLFWwindow *window;
    try {
        window = setUp();
    } catch (const char *e) {
        cout << e << endl;
        return -1;
    }

    // Enable depth testing and disable face culling
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Load shaders 
    shaderProgram = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

    // Set projection matrix
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    projection = glm::perspective(glm::radians(currentFOV), (float)width / (float)height, 0.1f, 1000.0f);

    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;
    
// ----------Create a collection of furniture----------
    std::vector<Furniture> furnitureCollection;
    float groundLevel = 3.5f; // Set the common ground level for all furniture
    
    // Cutouts and tables near them
    auto ornament_materialGroups = loadObjModel("objects/Ornament.obj", reader_config);
    auto cutoffs_materialGroups = loadObjModel("objects/cutoffs.obj", reader_config);

    if (!ornament_materialGroups.empty())
    {
        float startZ = 16.0f;               
        float spacing = 6.0f;               
        float ornamentX = -14.5f;            
        float cutoffOffset = -9.0f; 

        for (int i = 0; i < 5; ++i)
        {
            furnitureCollection.push_back({ornament_materialGroups, glm::vec3(ornamentX, groundLevel, startZ - (i * spacing)),glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.8f)});

            // Add cutoffs between ornaments
            if (i < 4 && !cutoffs_materialGroups.empty())
            { // Add 5 cutoffs between 6 ornaments
                furnitureCollection.push_back({
                    cutoffs_materialGroups,
                    glm::vec3(ornamentX + 2.0f, groundLevel, startZ + (i*spacing) + cutoffOffset), // Position halfway
                    glm::vec3(0.0f, 180.0f, 0.0f),                                                          // Rotation
                    glm::vec3(0.7f, 0.7f, 1.2f)                                                                         // Scale
                });
            }
        }
    } 

    auto lcouch_materialGroups = loadObjModel("objects/Lcouch.obj", reader_config);
    if (!lcouch_materialGroups.empty()) {

        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(-4.0f, groundLevel, -10.0f), // Position
            glm::vec3(0.0f, 270.0f, 0.0f),         // Rotation
            glm::vec3(0.7f)                      // Scale
        });

        // First LCouch (part of connected pair)
        furnitureCollection.push_back({
             lcouch_materialGroups,
            glm::vec3(-0.2f, groundLevel, 4.0f), // Position
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(0.7f)                      // Scale
        });
        
        //Second LCouch (connected to first)
        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(2.5f, groundLevel, -1.0f), // Position
            glm::vec3(0.0f, 90.0f, 0.0f),        // Rotation for L-shape
            glm::vec3(0.7f)                      // Scale
        });
        
        // Third LCouch (separate)
        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(3.0f, groundLevel, 5.0f),   // Position
            glm::vec3(0.0f, 90.0f, 0.0f),        // Rotation
            glm::vec3(0.7f)                       // Scale
        });
    }
    

    // 13-14. Loading Dividers (2 instances)
    auto divider_materialGroups = loadObjModel("objects/divider.obj", reader_config);
    if (!divider_materialGroups.empty()) {

        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(-0.8f, groundLevel, -12.0f),   // Position (top side of cubic couch)
            glm::vec3(0.0f, 90.0f, 0.0f),          // Rotation
            glm::vec3(0.7f)                       // Scale
        });
        // Divider 1
        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(-0.8f, groundLevel, -8.0f),   // Position (top side of cubic couch)
            glm::vec3(0.0f, 90.0f, 0.0f),          // Rotation
            glm::vec3(0.7f)                       // Scale
        });
        
       furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(-0.2f, groundLevel, 4.0f),   // Position (behind first L-couch)
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotation
            glm::vec3(0.7f)                       // Scale
        });

        // Divider for second L-couch back
        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(3.0f, groundLevel, 4.5f),   // Position (behind second L-couch)
            glm::vec3(0.0f, 90.0f, 0.0f),         // Rotation
            glm::vec3(0.7f)                       // Scale
        });
    }
    
    // 15-16. Loading CubicCouch (2 instances)
    auto cubicCouch_materialGroups = loadObjModel("objects/cubicCouch.obj", reader_config);
    if (!cubicCouch_materialGroups.empty()) {
        furnitureCollection.push_back({
            cubicCouch_materialGroups,
            glm::vec3(1.8f, groundLevel, 0.0f),   
            glm::vec3(0.0f, 90.0f, 0.0f),         
            glm::vec3(0.7f, 0.7f, 0.7f)                      
        });

        furnitureCollection.push_back({
            cubicCouch_materialGroups,
            glm::vec3(6.0f, groundLevel, -26.0f),   
            glm::vec3(0.0f, 90.0f, 0.0f),         
            glm::vec3(0.7f, 0.7f, 0.7f)                      
        });
    }
    
    // 17-20. Loading Small Tables (4 instances)
    auto smallTable_materialGroups = loadObjModel("objects/smallTable.obj", reader_config);
    if (!smallTable_materialGroups.empty()) {
        // Small Table 1
        furnitureCollection.push_back({
            smallTable_materialGroups,
            glm::vec3(-2.0f, groundLevel, 6.0f),   
            glm::vec3(0.0f, 0.0f, 0.0f),         
            glm::vec3(0.5f)                      
        });

        furnitureCollection.push_back({
            smallTable_materialGroups,
            glm::vec3(-3.8f, groundLevel, 4.0f),   
            glm::vec3(0.0f, 0.0f, 0.0f),         
            glm::vec3(0.5f)                      
        });
    }
       
    // 21-24. Loading Tall Tables (4 instances - 2 pairs that are close to each other)
    auto tallTable_materialGroups = loadObjModel("objects/tallTable.obj", reader_config);
    if (!tallTable_materialGroups.empty()) {
        furnitureCollection.push_back({
            tallTable_materialGroups,
            glm::vec3(-2.0f, groundLevel, 12.0f),  
            glm::vec3(0.0f, 0.0f, 0.0f),         
            glm::vec3(0.7f)                      
        });
        
        // Tall Table 2 (south, spaced from first)
        furnitureCollection.push_back({
            tallTable_materialGroups,
            glm::vec3(-1.8f, groundLevel, 12.0f),  
            glm::vec3(0.0f, 0.0f, 0.0f),         
            glm::vec3(0.7f)                      
        });

        furnitureCollection.push_back({
        tallTable_materialGroups,
        glm::vec3(-2.5f, groundLevel, -1.0f),  
        glm::vec3(0.0f, 0.0f, 0.0f),         
        glm::vec3(0.7f)             
        });         

        furnitureCollection.push_back({
        tallTable_materialGroups,
        glm::vec3(-2.5f, groundLevel, 2.0f),  // Moved further south (positive Z), spaced
        glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
        glm::vec3(0.7f)          
    });
    }

    auto potPlant_materialGroups = loadObjModel("objects/potPlant.obj", reader_config);
    if (!potPlant_materialGroups.empty()) {
        furnitureCollection.push_back({
            potPlant_materialGroups,
            glm::vec3(3.0f, groundLevel, 11.5f),  
            glm::vec3(0.0f, 0.0f, 0.0f),         
            glm::vec3(1.0f)                      
        });

        furnitureCollection.push_back({
            potPlant_materialGroups,
            glm::vec3(5.0f, groundLevel, 11.5f),  
            glm::vec3(0.0f, 0.0f, 0.0f),         
            glm::vec3(1.0f)                      
        });

        furnitureCollection.push_back({
            potPlant_materialGroups,
            glm::vec3(0.0f, groundLevel, 8.0f),  
            glm::vec3(0.0f, 0.0f, 0.0f),         
            glm::vec3(1.0f)                      
        });
    }
    

    auto shortTable_materialGroups = loadObjModel("objects/shortTable.obj", reader_config);
    if (!shortTable_materialGroups.empty()) {
        // Short Table 1
        furnitureCollection.push_back({
            shortTable_materialGroups,
            glm::vec3(-6.0f, groundLevel, -14.0f),  
            glm::vec3(0.0f, 0.0f, 0.0f),         
            glm::vec3(0.6f)                      
        });
        
        // // Short Table 2
        furnitureCollection.push_back({
             shortTable_materialGroups,
            glm::vec3(-5.0f, groundLevel, -14.0f),   
            glm::vec3(0.0f, 90.0f, 0.0f),        
            glm::vec3(0.6f)                      
        });
    }
    
    auto comfortableChair_materialGroups = loadObjModel("objects/comfortableChair.obj", reader_config);
    if (!comfortableChair_materialGroups.empty()) {
        // Near Short Table 1
        // Chair 1
        furnitureCollection.push_back({
           comfortableChair_materialGroups,
            glm::vec3(-6.0f, groundLevel, -12.0f), 
            glm::vec3(0.0f, 180.0f, 0.0f),        
            glm::vec3(0.7f)                      
        });
        
        // Chair 2
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(-6.0f, groundLevel, -12.0f),  // Position closer to table, facing table
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotate to face south (towards table)
            glm::vec3(0.7f)                      // Scale
        });
        
    //     // Near Short Table 2
    //     // Chair 3
    //     furnitureCollection.push_back({
    //          comfortableChair_materialGroups,
    //         glm::vec3(7.5f, groundLevel, -20.0f), // Position closer to table, facing table
    //         glm::vec3(0.0f, 180.0f, 0.0f),        // Rotate to face north (towards table)
    //         glm::vec3(1.0f)                      // Scale
    //     });
        
    //     // Chair 4
    //    furnitureCollection.push_back({
    //         comfortableChair_materialGroups,
    //         glm::vec3(0.0f, groundLevel, -20.0f),  // Position closer to table, facing table
    //         glm::vec3(0.0f, 0.0f, 0.0f),          // Rotate to face south (towards table)
    //         glm::vec3(1.0f)                      // Scale
    //     });

    auto bigTable_materialGroups = loadObjModel("objects/BigTable.obj", reader_config);
    auto blue_materialGroups = loadObjModel("objects/blue.obj", reader_config);
    auto yellow_materialGroups = loadObjModel("objects/yellow.obj", reader_config);

    if (!bigTable_materialGroups.empty()) {

        float bigTableZ = -2.0f; 
        float bigTableX = -1.0f; 

        //Big Table 1
        furnitureCollection.push_back({
            bigTable_materialGroups,
            glm::vec3(bigTableX-5.0f, groundLevel, bigTableZ),
            glm::vec3(0.0f, 90.0f, 0.0f), 
            glm::vec3(0.6f) 
        });

        //Big Table 2
        furnitureCollection.push_back({
            bigTable_materialGroups,
            glm::vec3(bigTableX, groundLevel, bigTableZ),
            glm::vec3(0.0f, 90.0f, 0.0f), 
            glm::vec3(0.6f) 
        });

        // Make Big table a small Table
        furnitureCollection.push_back({
            bigTable_materialGroups,
            glm::vec3(bigTableX + 3.0f, groundLevel, bigTableZ + 13.5f),
            glm::vec3(0.0f, 90.0f, 0.0f), 
            glm::vec3(0.2f) 
        });

        float bigChairOffsetZ = -1.0f; 
        float bigChairOffsetX = 0.5f; 

        //Chairs for the table
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(bigTableX + bigChairOffsetX*3, groundLevel, bigTableZ - bigChairOffsetZ), 
            glm::vec3(0.0f, 90.0f, 0.0f), 
            glm::vec3(0.7f)
        });
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(bigTableX + bigChairOffsetX, groundLevel, bigTableZ - bigChairOffsetZ), 
            glm::vec3(0.0f, 90.0f, 0.0f), 
            glm::vec3(0.7f)
        });

        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(bigTableZ + bigChairOffsetZ, groundLevel, bigTableX-6.0f - bigChairOffsetX*2), 
            glm::vec3(0.0f, 270.0f, 0.0f), 
            glm::vec3(0.7f)
        });
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(bigTableZ + bigChairOffsetZ*2, groundLevel, bigTableX-6.0f - bigChairOffsetX*2), 
            glm::vec3(0.0f, 270.0f, 0.0f), 
            glm::vec3(0.7f)
        });

        //Blue Ottomans
      
        if (!blue_materialGroups.empty()) {
            
            //Furthest
            furnitureCollection.push_back({
                blue_materialGroups,
                glm::vec3(bigTableX - 0.0f, groundLevel, bigTableZ), 
                glm::vec3(0.0f, 0.0f, 0.0f),         
                glm::vec3(0.7f)                     
            });

            furnitureCollection.push_back({
                blue_materialGroups,
                glm::vec3(bigTableX - 2.0f, groundLevel, -0.0f),  
                glm::vec3(0.0f, 0.0f, 0.0f),        
                glm::vec3(0.7f)                      
            });
            
            furnitureCollection.push_back({
                blue_materialGroups,
                glm::vec3(bigTableX+2.0f, groundLevel, 2.0f),  
                glm::vec3(0.0f, 0.0f, 0.0f),        
                glm::vec3(0.7f)                      
            });

            
            furnitureCollection.push_back({
                blue_materialGroups,
                glm::vec3(bigTableX + 4.0f, groundLevel, bigTableZ + 16.0f),  
                glm::vec3(0.0f, 0.0f, 0.0f),        
                glm::vec3(0.7f)                      
            });

           
        }

        //Yellow Ottomans

        if (!yellow_materialGroups.empty()) {

            furnitureCollection.push_back({
                yellow_materialGroups,
                glm::vec3(bigTableX - 2.0f, groundLevel, bigTableZ), 
                glm::vec3(0.0f, 0.0f, 0.0f),         
                glm::vec3(0.7f)                     
            });

            furnitureCollection.push_back({
                yellow_materialGroups,
                glm::vec3(bigTableX +3.0f, groundLevel, bigTableZ + 0.0f), 
                glm::vec3(0.0f, 0.0f, 0.0f),         
                glm::vec3(0.7f)                        
            });

            furnitureCollection.push_back({
                yellow_materialGroups,
                glm::vec3(bigTableX + 1.0f, groundLevel, bigTableZ - 4.0f), 
                glm::vec3(0.0f, 0.0f, 0.0f),         
                glm::vec3(0.7f)                        
            });

            furnitureCollection.push_back({
                yellow_materialGroups,
                glm::vec3(bigTableX + 2.0f, groundLevel, bigTableZ + 20.0f), 
                glm::vec3(0.0f, 0.0f, 0.0f),         
                glm::vec3(0.7f)                        
            });
        }


    }
}

    auto tallChair_materialGroups = loadObjModel("objects/tallChairs.obj", reader_config);
    if (!tallChair_materialGroups.empty()) {

        //Chair for Table 1 near entrance
        furnitureCollection.push_back({
            tallChair_materialGroups,
            glm::vec3(-2.0f, groundLevel, 11.0f), // Slightly less Z to be behind/close to table
            glm::vec3(0.0f, 0.0f, 0.0f), // Rotate to face towards the table (north)
            glm::vec3(0.8f)
        });

        //Chair for Table 2 near entrance
        furnitureCollection.push_back({
            tallChair_materialGroups,
            glm::vec3(0.0f, groundLevel, 11.0f), 
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.8f)
        });

        //Chair for Table 1 near divider
        furnitureCollection.push_back({
            tallChair_materialGroups,
            glm::vec3(-2.5f, groundLevel, -2.0f), 
            glm::vec3(0.0f, 0.0f, 0.0f), 
            glm::vec3(0.8f)
        });

        //Chair for Table 2 near divider
        furnitureCollection.push_back({
            tallChair_materialGroups,
            glm::vec3(0.0f, groundLevel, 6.0f), 
            glm::vec3(0.0f, 270.0f, 0.0f), 
            glm::vec3(0.8f)
        });
    }

// ----------Load room components----------
    // Load Carpet (ground)
    std::vector<MaterialGroup> carpet0_materialGroups = loadObjModel("Objects/carpet_0.obj", reader_config);
    std::vector<MaterialGroup> carpet1_materialGroups = loadObjModel("Objects/carpet_1.obj", reader_config);
    std::vector<MaterialGroup> carpet2_materialGroups = loadObjModel("Objects/carpet_2.obj", reader_config);

    // Load Roof
    std::vector<MaterialGroup> roof_materialGroups = loadObjModel("Objects/alt_panels.obj", reader_config);

    // Load North Wall
    std::vector<MaterialGroup> northwall_materialGroups = loadObjModel("Objects/north_south_wall.obj", reader_config);

    // Load WestWall
    std::vector<MaterialGroup> westwall_materialGroups = loadObjModel("Objects/glassPanel.obj", reader_config);

    // East wall (named west)
    Wall westWall(4.0f, 10.0f, 0.2f, 5, 8);
        
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and capture mouse cursor
    // glfwSetCursorPosCallback(window, mouse_callback); // Register the callback function
    LightingManager light;

    // Load Drone 
    std::vector<MaterialGroup> drone_materialGroups = loadObjModel("Objects/drone.obj", reader_config);

    bool enterPressedLastFrame = false;

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ----------Main rendering loop----------
    do {
        // ----- Camera colour filters -----
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterPressedLastFrame) {
            filterMode = (filterMode + 1) % 4; // 0 = normal, 1 = night vison, 2 = inverted, 3 = grey scale
            enterPressedLastFrame = true;
            // cout << filterMode << endl; 
        }
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
            enterPressedLastFrame = false;
        }

        // ----- Drone Movement -----
        // Translate along all 3 axes
        float speed = 1.0f;

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            dronePosition.y += speed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            dronePosition.y -= speed;      
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            dronePosition.x -= speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            dronePosition.x += speed;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            dronePosition.z -= speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            dronePosition.z += speed;

        // Rotate along all 3 axes
        float rotationSpeed = 2.0f;

        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            droneYaw += rotationSpeed;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
            droneYaw -= rotationSpeed;
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            dronePitch += rotationSpeed;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            dronePitch -= rotationSpeed;
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
            droneRoll += rotationSpeed;
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
            droneRoll -= rotationSpeed;

        // Create the drone's rotation matrix
        glm::mat4 droneRotation = glm::mat4(1.0f);
        droneRotation = glm::rotate(droneRotation, glm::radians(droneYaw), glm::vec3(0.0f, 1.0f, 0.0f));
        droneRotation = glm::rotate(droneRotation, glm::radians(dronePitch), glm::vec3(1.0f, 0.0f, 0.0f));
        droneRotation = glm::rotate(droneRotation, glm::radians(droneRoll), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 cameraOffsetLocal = glm::vec3(0.0f, -0.5f, 0.0f);
        glm::vec3 cameraOffsetWorld = glm::vec3(droneRotation * glm::vec4(cameraOffsetLocal, 1.0f));
        glm::vec3 cameraPosition = dronePosition + cameraOffsetWorld;
        glm::vec3 cameraTargetOffsetLocal = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 cameraTargetOffsetWorld = glm::vec3(droneRotation * glm::vec4(cameraTargetOffsetLocal, 1.0f));
        glm::vec3 cameraTarget = cameraPosition + cameraTargetOffsetWorld;
        view = glm::lookAt(cameraPosition, cameraTarget, glm::vec3(0.0f, 0.0f, -1.0f)); // Or glm::vec3(0.0f, 1.0f, 0.0f) depending on your world up

        // Create the drone's model matrix with translation and rotation
        glm::mat4 droneModel = glm::mat4(1.0f);
        droneModel = glm::translate(droneModel, dronePosition);
        droneModel = glm::scale(droneModel, glm::vec3(0.4f));
        droneModel = droneModel * droneRotation; 

        // --- Camera Zoom ---
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) { 
            currentFOV -= zoomSpeed; 
            if (currentFOV < minFOV)
                currentFOV = minFOV;
            projection = glm::perspective(glm::radians(currentFOV), (float)width / (float)height, 0.1f, 1000.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) { 
            currentFOV += zoomSpeed; 
            if (currentFOV > maxFOV)
                currentFOV = maxFOV;
            projection = glm::perspective(glm::radians(currentFOV), (float)width / (float)height, 0.1f, 1000.0f);
        }

        glClearColor(0.678f, 0.847f, 0.902f, 1.0f); // background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        light.upload(shaderProgram);

        // Set up matrices uniforms
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLuint modeLoc = glGetUniformLocation(shaderProgram, "mode");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(modeLoc, filterMode);

        // Get uniform location for hasTexture
        GLint hasTextureLoc = glGetUniformLocation(shaderProgram, "hasTexture");
        if (hasTextureLoc == -1) {
            std::cerr << "Uniform 'hasTexture' not found!" << std::endl;
        }

        renderCarpet(carpet0_materialGroups, carpet1_materialGroups, carpet2_materialGroups, modelLoc);
        renderRoof(roof_materialGroups, modelLoc); 
        renderNSWalls(northwall_materialGroups, modelLoc); 
        renderWEWalls(groundLevel, westwall_materialGroups, modelLoc);
        renderFurniture(furnitureCollection); 

        westWall.draw(view, projection, shaderProgram);
        glm::mat4 translationMatrix(
            glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),  // Column 0 (X-axis basis vector)
            glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),  // Column 1 (Y-axis basis vector)
            glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),  // Column 2 (Z-axis basis vector)
            glm::vec4(10.0f, -10.0f, 20.0f, 1.0f) // Column 3 (Translation vector)
        );
        westWall.setTransform(translationMatrix);

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(droneModel));
        renderDrone(drone_materialGroups, modelLoc);

        // ----- Mini-map rendering -----
        int miniWidth = 150;
        int miniHeight = 300;

        // Save current viewport
        glViewport(0, 0, width, height); 

        // Set viewport to top-right corner
        glViewport(width - miniWidth, height - miniHeight - 5, miniWidth, miniHeight);

        // Fixed top-down camera position for mini-map
        glm::vec3 topDownCam = glm::vec3(0.0f, 30.0f, 0.0f);     
        glm::vec3 lookingDown = glm::vec3(0.0f, 0.0f, 0.0f);       
        glm::vec3 topDownUp = glm::vec3(0.0f, 0.0f, -1.0f);       

        glm::mat4 minimapView = glm::lookAt(topDownCam, lookingDown, topDownUp);
        glm::mat4 minimapProj = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, 1.0f, 100.0f); // Orthographic projection

        // Upload matrices
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(minimapView));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(minimapProj));

        renderCarpet(carpet0_materialGroups, carpet1_materialGroups, carpet2_materialGroups, modelLoc);
        renderFurniture(furnitureCollection); 
        
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(droneModel));
        renderDrone(drone_materialGroups, modelLoc); 

        // Restore main viewport 
        glViewport(0, 0, width, height);

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// ----------Cleanup----------
    for (const auto &furniture : furnitureCollection) {
        for (const auto &group : furniture.materialGroups) {
            glDeleteVertexArrays(1, &group.VAO);
            glDeleteBuffers(1, &group.VBO);
            if (group.hasTexture) {
                glDeleteTextures(1, &group.textureID);
            }
        }
    }
    for (const auto& group : carpet0_materialGroups) {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
    }
    for (const auto& group : carpet1_materialGroups) {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
    }
    for (const auto& group : carpet2_materialGroups) {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
    }
    for (const auto& group : roof_materialGroups) {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
    }
    for (const auto& group : northwall_materialGroups) {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

