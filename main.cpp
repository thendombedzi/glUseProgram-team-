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

// Define MaterialGroup at global scope before using it
struct MaterialGroup {
    GLuint VAO, VBO;
    GLsizei vertexCount;
    glm::vec3 color;
};

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

    GLFWwindow *window = glfwCreateWindow(1400, 1000, "Room Furniture Layout", NULL, NULL);
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

// Function to load object files and create material groups
std::vector<MaterialGroup> loadObjModel(const std::string& filename, const tinyobj::ObjReaderConfig& config) {
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filename, config)) {
        std::cerr << "TinyObjReader failed to load " << filename << ": " << reader.Error() << std::endl;
        return {};
    }
    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader warning (" << filename << "): " << reader.Warning() << std::endl;
    }
    
    const tinyobj::attrib_t &attrib = reader.GetAttrib();
    const std::vector<tinyobj::shape_t> &shapes = reader.GetShapes();
    const std::vector<tinyobj::material_t> &materials = reader.GetMaterials();
    
    std::vector<MaterialGroup> materialGroups;
    
    for (size_t s = 0; s < shapes.size(); ++s) {
        const auto& shape = shapes[s];
        std::unordered_map<int, std::vector<float>> materialVertexMap;
        
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int mat_id = shape.mesh.material_ids[f];
            int fv = shape.mesh.num_face_vertices[f];
            
            for (size_t v = 0; v < fv; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[f * fv + v];
                materialVertexMap[mat_id].insert(materialVertexMap[mat_id].end(), {
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
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
            if (mat_id >= 0 && mat_id < (int)materials.size()) {
                color = glm::vec3(materials[mat_id].diffuse[0], materials[mat_id].diffuse[1], materials[mat_id].diffuse[2]);
            }
            
            materialGroups.push_back({VAO, VBO, static_cast<GLsizei>(verts.size() / 3), color});
        }
    }
    
    return materialGroups;
}

// Structure to manage furniture with position, rotation, and scale
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
        
        for (const auto& group : materialGroups) {
            GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
            glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
            glBindVertexArray(group.VAO);
            glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
        }
    }
};

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
    shaderProgram = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");

    // Set camera
    view = glm::lookAt(
        glm::vec3(20.0f, 15.0f, 20.0f), // Position further back and higher to see full room
        glm::vec3(0.0f, 0.0f, 0.0f),    // Look at the origin
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // Set projection matrix
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.1f, 1000.0f);

    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;
    
    // Create a collection of furniture
    std::vector<Furniture> furnitureCollection;
    float groundLevel = 0.0f; // Set the common ground level for all furniture
    
    // 1-2. Loading Blue Carpet (2 instances)
    auto blue_materialGroups = loadObjModel("Objects/blue.obj", reader_config);
    if (!blue_materialGroups.empty()) {
        // First blue carpet
        furnitureCollection.push_back({
            blue_materialGroups,
            glm::vec3(7.0f, groundLevel, 12.0f), // Position
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                      // Scale
        });
        
        // Second blue carpet
        furnitureCollection.push_back({
            blue_materialGroups,
            glm::vec3(6.0f, groundLevel, 14.0f),  // Position
            glm::vec3(0.0f, 0.0f, 0.0f),        // Rotation
            glm::vec3(1.0f)                      // Scale
        });
    }
    
    // 3-4. Loading Yellow (2 instances)
    auto yellow_materialGroups = loadObjModel("Objects/yellow.obj", reader_config);
    if (!yellow_materialGroups.empty()) {
        // First yellow
        furnitureCollection.push_back({
            yellow_materialGroups,
            glm::vec3(4.0f, groundLevel, 16.0f), // Position
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                        // Scale
        });
        
        // Second yellow (removed as per previous prompt - leaving commented for reference)
        // furnitureCollection.push_back({
        //     yellow_materialGroups,
        //     glm::vec3(7.0f, groundLevel, -4.0f),  // Position
        //     glm::vec3(0.0f, 90.0f, 0.0f),         // Rotation
        //     glm::vec3(1.0f)                       // Scale
        // });
    }
    
    // 5-7. Loading LCouch (3 instances - 2 connected, 1 separate)
    auto lcouch_materialGroups = loadObjModel("Objects/LCouch.obj", reader_config);
    if (!lcouch_materialGroups.empty()) {
        // First LCouch (part of connected pair)
        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(0.0f, groundLevel, 9.5f), // Position
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                      // Scale
        });
        
        // Second LCouch (connected to first, forming L-shape)
        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(3.0f, groundLevel, 9.5f), // Position
            glm::vec3(0.0f, 90.0f, 0.0f),        // Rotation for L-shape
            glm::vec3(1.0f)                      // Scale
        });
        
        // Third LCouch (separate)
        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(-1.0f, groundLevel, -8.0f),   // Position
            glm::vec3(0.0f, 90.0f, 0.0f),        // Rotation
            glm::vec3(1.0f)                       // Scale
        });
    }
    
    // Loading Ornaments (4 instances - all on the left side, spaced equally from south to north)
    auto ornament_materialGroups = loadObjModel("Objects/Ornament.obj", reader_config);
    if (!ornament_materialGroups.empty()) {
        float startZ = 13.0f; // Southmost position
        float spacing = 6.0f; // Spacing between ornaments
        float ornamentX = -22.0f; // Further left position

        for (int i = 0; i < 4; ++i) {
            furnitureCollection.push_back({
                ornament_materialGroups,
                glm::vec3(ornamentX, groundLevel, startZ - (i * spacing)), // Consistent X, varying Z
                glm::vec3(0.0f, 0.0f, 0.0f),                             // Rotation
                glm::vec3(1.0f)                                          // Scale
            });
        }
    }
    
    // 13-14. Loading Dividers (3 instances - one for cubic couch, one for each L-couch back)
    auto divider_materialGroups = loadObjModel("Objects/divider.obj", reader_config);
    if (!divider_materialGroups.empty()) {
        // Divider for cubic couch
        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(0.0f, groundLevel, -7.3f),   // Position (top side of cubic couch)
            glm::vec3(0.0f, 90.0f, 0.0f),          // Rotation
            glm::vec3(1.0f)                       // Scale
        });
        
        // Divider for first L-couch back
        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(0.0f, groundLevel, 9.0f),   // Position (behind first L-couch)
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotation
            glm::vec3(1.0f)                       // Scale
        });

        // Divider for second L-couch back
        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(2.0f, groundLevel, 8.5f),   // Position (behind second L-couch)
            glm::vec3(0.0f, 90.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                       // Scale
        });
    }
    
    // 15-16. Loading CubicCouch (1 instance)
    auto cubicCouch_materialGroups = loadObjModel("Objects/cubicCouch.obj", reader_config);
    if (!cubicCouch_materialGroups.empty()) {
        // CubicCouch 
        furnitureCollection.push_back({
            cubicCouch_materialGroups,
            glm::vec3(18.0f, groundLevel, -17.0f),   // Position
            glm::vec3(0.0f, 85.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                      // Scale
        });
    }
    
    // 17-20. Loading Small Tables (1 instance - one was commented out)
    auto smallTable_materialGroups = loadObjModel("Objects/smallTable.obj", reader_config);
    if (!smallTable_materialGroups.empty()) {
        // Small Table 1
        furnitureCollection.push_back({
            smallTable_materialGroups,
            glm::vec3(-2.0f, groundLevel, 7.0f),   // Position
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                      // Scale
        });
    }
    
    // 21-24. Loading Tall Tables (2 instances, to the south)
    auto tallTable_materialGroups = loadObjModel("Objects/tallTable.obj", reader_config);
    if (!tallTable_materialGroups.empty()) {
        // Tall Table 1 (south)
        furnitureCollection.push_back({
            tallTable_materialGroups,
            glm::vec3(-1.0f, groundLevel, 12.0f),  // Moved further south (positive Z)
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                      // Scale
        });
        
        // Tall Table 2 (south, spaced from first)
        furnitureCollection.push_back({
            tallTable_materialGroups,
            glm::vec3(3.0f, groundLevel, 12.0f),  // Moved further south (positive Z), spaced
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                      // Scale
        });
    }
    
    auto potPlant_materialGroups = loadObjModel("Objects/potPlant.obj", reader_config);
    if (!potPlant_materialGroups.empty()) {
        // Short Table 1 (north)
        furnitureCollection.push_back({
            potPlant_materialGroups,
            glm::vec3(5.0f, groundLevel, 15.0f),  // Moved even further north (negative Z)
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                      // Scale
        });}
  
    // 25-26. Loading Short Tables (2 instances, moved even further north)
    auto shortTable_materialGroups = loadObjModel("Objects/shortTable.obj", reader_config);
    if (!shortTable_materialGroups.empty()) {
        // Short Table 1 (north)
        furnitureCollection.push_back({
            shortTable_materialGroups,
            glm::vec3(-4.0f, groundLevel, -16.0f),  // Moved even further north (negative Z)
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                      // Scale
        });
        
        // Short Table 2 (north)
        furnitureCollection.push_back({
            shortTable_materialGroups,
            glm::vec3(2.0f, groundLevel, -13.0f),   // Moved even further north (negative Z)
            glm::vec3(0.0f, 90.0f, 0.0f),        // Rotation
            glm::vec3(1.0f)                      // Scale
        });
    }
    
    // Loading Comfortable Chairs (already existing 4 instances for short tables + 4 for big table)
    auto comfortableChair_materialGroups = loadObjModel("Objects/comfortableChair.obj", reader_config);
    if (!comfortableChair_materialGroups.empty()) {
        // Chairs for Short Table 1
        // Chair 1 (behind Short Table 1)
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(-3.0f, groundLevel, -15.0f), // Position closer to table, facing table
            glm::vec3(0.0f, 180.0f, 0.0f),        // Rotate to face north (towards table)
            glm::vec3(1.0f)                      // Scale
        });
        
        // Chair 2 (in front of Short Table 1)
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(-9.5f, groundLevel, -14.5f),  // Position closer to table, facing table
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotate to face south (towards table)
            glm::vec3(1.0f)                      // Scale
        });
        
        // Chairs for Short Table 2
        // Chair 3 (behind Short Table 2)
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(7.5f, groundLevel, -12.0f), // Position closer to table, facing table
            glm::vec3(0.0f, 180.0f, 0.0f),        // Rotate to face north (towards table)
            glm::vec3(1.0f)                      // Scale
        });
        
        // Chair 4 (in front of Short Table 2)
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(0.0f, groundLevel, -13.0f),  // Position closer to table, facing table
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotate to face south (towards table)
            glm::vec3(1.0f)                      // Scale
        });

        // --- Big Table and its comfortable chairs ---
        auto bigTable_materialGroups = loadObjModel("Objects/BigTable.obj", reader_config);
        if (!bigTable_materialGroups.empty()) {
            float bigTableZ = 5.0f; 
            float bigTableX = -3.0f; 

            furnitureCollection.push_back({
                bigTable_materialGroups,
                glm::vec3(bigTableX, groundLevel, bigTableZ),
                glm::vec3(0.0f, 90.0f, 0.0f), 
                glm::vec3(1.0f) 
            });

            float bigChairOffsetZ = -1.5f; 
            float bigChairOffsetX = 1.5f; 

            furnitureCollection.push_back({
                comfortableChair_materialGroups,
                glm::vec3(bigTableX - bigChairOffsetX, groundLevel, bigTableZ + bigChairOffsetZ), 
                glm::vec3(0.0f, 90.0f, 0.0f), 
                glm::vec3(1.0f)
            });
            furnitureCollection.push_back({
                comfortableChair_materialGroups,
                glm::vec3(bigTableX + bigChairOffsetX, groundLevel, bigTableZ + bigChairOffsetZ), 
                glm::vec3(0.0f, 90.0f, 0.0f), 
                glm::vec3(1.0f)
            });

            furnitureCollection.push_back({
                comfortableChair_materialGroups,
                glm::vec3(bigTableX +1.5f, groundLevel, bigTableZ -10.0f), 
                glm::vec3(0.0f, 270.0f, 0.0f), 
                glm::vec3(1.0f)
            });
            furnitureCollection.push_back({
                comfortableChair_materialGroups,
                glm::vec3(bigTableX + 3.5f, groundLevel, bigTableZ -10.0f), 
                glm::vec3(0.0f, 270.0f, 0.0f), 
                glm::vec3(1.0f)
            });
        }
    }

    // --- NEW: Tall Chairs for Tall Tables ---
    auto tallChair_materialGroups = loadObjModel("Objects/tallChairs.obj", reader_config);
    if (!tallChair_materialGroups.empty()) {
        // Chair for Tall Table 1
        // Tall Table 1 is at (-1.0f, groundLevel, 12.0f)
        furnitureCollection.push_back({
            tallChair_materialGroups,
            glm::vec3(-1.0f, groundLevel, 11.0f), // Slightly less Z to be behind/close to table
            glm::vec3(0.0f, 0.0f, 0.0f), // Rotate to face towards the table (north)
            glm::vec3(1.0f)
        });

        // Chair for Tall Table 2
        // Tall Table 2 is at (3.0f, groundLevel, 12.0f)
        furnitureCollection.push_back({
            tallChair_materialGroups,
            glm::vec3(3.0f, groundLevel, 11.0f), // Slightly less Z to be behind/close to table
            glm::vec3(0.0f, 0.0f, 0.0f), // Rotate to face towards the table (north)
            glm::vec3(1.0f)
        });
    }

    // Main loop
    do
    {
        glfwPollEvents();

        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Set view and projection uniforms
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Render all furniture
        for (const auto& furniture : furnitureCollection) {
            furniture.render(shaderProgram);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    // Cleanup - Free all VAOs and VBOs
    for (const auto& furniture : furnitureCollection) {
        for (const auto& group : furniture.materialGroups) {
            glDeleteVertexArrays(1, &group.VAO);
            glDeleteBuffers(1, &group.VBO);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}