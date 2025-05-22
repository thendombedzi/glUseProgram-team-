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
#include "Objects/EastWall/WindowWall.hpp"
#include "Objects/WestWall/Wall.hpp"
#include "lightingManager.hpp"

using namespace glm;
using namespace std;

// Global OpenGL data
GLuint shaderProgram;
glm::mat4 view, projection;

// Camera parameters
glm::vec3 cameraPos   = glm::vec3(20.0f, 15.0f, 20.0f); // Initial camera position
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Camera looks towards negative Z initially
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);  // Up direction

// Camera rotation
float yawM = -90.0f; // yawM is initialized to -90.0 degrees since a yawM of 0.0 results in a direction vector pointing to the right, so we initially rotate a bit to the left.
float pitchM = 0.0f;

// Mouse input
float lastX = 1400 / 2.0f; // Initial mouse X position (center of window)
float lastY = 1000 / 2.0f; // Initial mouse Y position (center of window)
bool firstMouse = true;

// Timing for consistent movement speed
float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Define MaterialGroup at global scope before using it
// struct MaterialGroup {
//     GLuint VAO, VBO;
//     GLsizei vertexCount;
//     glm::vec3 color;
// };
// struct MaterialGroup {
//     GLuint VAO, VBO;
//     GLsizei vertexCount;
//     glm::vec3 color;
// };

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

    GLFWwindow *window = glfwCreateWindow(1400, 1000, "IT Kiosk", NULL, NULL);
    if (window == NULL) {
        cout << getError() << endl;
        glfwTerminate();
        throw "Failed to open GLFW window.";
    }

    glfwMakeContextCurrent(window);
    startUpGLEW();
    return window;
}

<<<<<<< HEAD
=======
<<<<<<< HEAD
<<<<<<< HEAD
// Your MaterialGroup definition
struct MaterialGroup {
    GLuint VAO;
    GLuint VBO;
    GLsizei vertexCount;
    glm::vec3 color;
};

struct InterleavedVertex {
    float px, py, pz;  // Position
    float nx, ny, nz;  // Normal
};

=======
=======
>>>>>>> ee422df (placed walls and roof)
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // Adjust this value for mouse sensitivity
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yawM += xoffset;
    pitchM += yoffset;

    // Constrain pitchM to avoid flipping the camera
    if (pitchM > 89.0f)
        pitchM = 89.0f;
    if (pitchM < -89.0f)
        pitchM = -89.0f;\
    glm::vec3 front;
    front.x = cos(glm::radians(yawM)) * cos(glm::radians(pitchM));
    front.y = sin(glm::radians(pitchM));
    front.z = sin(glm::radians(yawM)) * cos(glm::radians(pitchM));
    cameraFront = glm::normalize(front);
}

<<<<<<< HEAD
>>>>>>> 729611f (placed walls and roof)
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // Adjust this value for mouse sensitivity
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yawM += xoffset;
    pitchM += yoffset;

    // Constrain pitchM to avoid flipping the camera
    if (pitchM > 89.0f)
        pitchM = 89.0f;
    if (pitchM < -89.0f)
        pitchM = -89.0f;\
    glm::vec3 front;
    front.x = cos(glm::radians(yawM)) * cos(glm::radians(pitchM));
    front.y = sin(glm::radians(pitchM));
    front.z = sin(glm::radians(yawM)) * cos(glm::radians(pitchM));
    cameraFront = glm::normalize(front);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // Adjust this value for mouse sensitivity
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yawM += xoffset;
    pitchM += yoffset;

    // Constrain pitchM to avoid flipping the camera
    if (pitchM > 89.0f)
        pitchM = 89.0f;
    if (pitchM < -89.0f)
        pitchM = -89.0f;\
    glm::vec3 front;
    front.x = cos(glm::radians(yawM)) * cos(glm::radians(pitchM));
    front.y = sin(glm::radians(pitchM));
    front.z = sin(glm::radians(yawM)) * cos(glm::radians(pitchM));
    cameraFront = glm::normalize(front);
}

// Your MaterialGroup definition
struct MaterialGroup {
    GLuint VAO;
    GLuint VBO;
    GLsizei vertexCount;
    glm::vec3 color;
};

struct InterleavedVertex {
    float px, py, pz;  // Position
    float nx, ny, nz;  // Normal
};

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


    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    const auto& materials = reader.GetMaterials();

    std::vector<MaterialGroup> materialGroups;

    for (const auto& shape : shapes) {
        std::unordered_map<int, std::vector<InterleavedVertex>> materialVertexMap;

        size_t index_offset = 0;

    for (const auto& shape : shapes) {
        std::unordered_map<int, std::vector<InterleavedVertex>> materialVertexMap;

        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int fv = shape.mesh.num_face_vertices[f];
            int mat_id = shape.mesh.material_ids[f];

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

                materialVertexMap[mat_id].push_back(vertex);
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

                materialVertexMap[mat_id].push_back(vertex);
            }

            index_offset += fv;

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
            glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(InterleavedVertex), verts.data(), GL_STATIC_DRAW);

            // Position: location 0
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(InterleavedVertex), (void*)0);
            glEnableVertexAttribArray(0);

            // Normal: location 1
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(InterleavedVertex), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // Color from material or default
            glm::vec3 color(0.8f); // default

            // Normal: location 1
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(InterleavedVertex), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // Color from material or default
            glm::vec3 color(0.8f); // default
            if (mat_id >= 0 && mat_id < (int)materials.size()) {
                const auto& m = materials[mat_id];
                color = glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
                const auto& m = materials[mat_id];
                color = glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);
            }

            materialGroups.push_back({ VAO, VBO, static_cast<GLsizei>(verts.size()), color });

            materialGroups.push_back({ VAO, VBO, static_cast<GLsizei>(verts.size()), color });
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

    // Load shaders (assuming "vertex_shader.glsl" and "fragment_shader.glsl" are correct for both objects)
    shaderProgram = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

    // Set camera to view the entire room and furniture
    // view = glm::lookAt(
    //     glm::vec3(20.0f, 15.0f, 20.0f), // Position further back and higher to see full room
    //     glm::vec3(0.0f, 0.0f, 0.0f),    // Look at the origin
    //     glm::vec3(0.0f, 1.0f, 0.0f)
    // );

    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // Set projection matrix
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    projection = glm::perspective(glm::radians(60.0f), (float)width / height, 0.1f, 1000.0f);

    tinyobj::ObjReaderConfig reader_config;
    reader_config.triangulate = true;
    
    // Create a collection of furniture
    std::vector<Furniture> furnitureCollection;
    float groundLevel = 3.3f; // Set the common ground level for all furniture
    
    // 1-2. Loading Blue Carpet (2 instances)
    // auto blue_materialGroups = loadObjModel("Objects/blue.obj", reader_config);
    // if (!blue_materialGroups.empty()) {
    //     // First blue carpet
    //     furnitureCollection.push_back({
    //          blue_materialGroups,
    //         glm::vec3(7.0f, groundLevel, 12.0f), // Position
    //         glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
    //         glm::vec3(1.0f)                      // Scale
    //     });
        
    //     // Second blue carpet
    //     furnitureCollection.push_back({
    //          blue_materialGroups,
    //         glm::vec3(6.0f, groundLevel, 14.0f),  // Position
    //         glm::vec3(0.0f, 0.0f, 0.0f),        // Rotation
    //         glm::vec3(1.0f)                      // Scale
    //     });
    // }
    
    // // 3-4. Loading Yellow (2 instances)
    // auto yellow_materialGroups = loadObjModel("Objects/yellow.obj", reader_config);
    // if (!yellow_materialGroups.empty()) {
    //     // First yellow
    //     furnitureCollection.push_back({
    //          yellow_materialGroups,
    //         glm::vec3(4.0f, groundLevel, 16.0f), // Position
    //         glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
    //         glm::vec3(1.0f)                        // Scale
    //     });
        
       
    // }
    
    // 5-7. Loading LCouch (3 instances - 2 connected, 1 separate)
    auto lcouch_materialGroups = loadObjModel("Objects/Lcouch.obj", reader_config);
    if (!lcouch_materialGroups.empty()) {
        // First LCouch (part of connected pair)
        furnitureCollection.push_back({
             lcouch_materialGroups,
            glm::vec3(-0.15f, groundLevel, 0.0f), // Position
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(0.7f)                      // Scale
        });
        
        //Second LCouch (connected to first)
        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(2.5f, groundLevel, -4.5f), // Position
            glm::vec3(0.0f, 90.0f, 0.0f),        // Rotation for L-shape
            glm::vec3(0.7f)                      // Scale
        });
        
        // Third LCouch (separate)
        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(2.0f, groundLevel, 2.5f),   // Position
            glm::vec3(0.0f, 90.0f, 0.0f),        // Rotation
            glm::vec3(0.7f)                       // Scale
        });
    }
    
    // 8-12. Loading Ornaments (5 instances)
    auto ornament_materialGroups = loadObjModel("Objects/Ornament.obj", reader_config);
    if (!ornament_materialGroups.empty()) {
        float startZ = 13.0f; // Southmost position
        float spacing = 5.0f; // Spacing between ornaments
        float ornamentX = -14.0f; // Further left position

        for (int i = 0; i < 4; ++i) {
            furnitureCollection.push_back({
                ornament_materialGroups,
                glm::vec3(ornamentX, groundLevel, startZ - (i * spacing)), // Consistent X, varying Z
                glm::vec3(0.0f, 0.0f, 0.0f),                             // Rotation
                glm::vec3(0.8f)                                          // Scale
            });
        }
    }
    // 13-14. Loading Dividers (2 instances)
    auto divider_materialGroups = loadObjModel("Objects/divider.obj", reader_config);
    if (!divider_materialGroups.empty()) {

        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(-0.8f, groundLevel, -16.3f),   // Position (top side of cubic couch)
            glm::vec3(0.0f, 90.0f, 0.0f),          // Rotation
            glm::vec3(0.7f)                       // Scale
        });
        // Divider 1
        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(-0.8f, groundLevel, -12.3f),   // Position (top side of cubic couch)
            glm::vec3(0.0f, 90.0f, 0.0f),          // Rotation
            glm::vec3(0.7f)                       // Scale
        });
        
       furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(-0.2f, groundLevel, 0.0f),   // Position (behind first L-couch)
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotation
            glm::vec3(0.7f)                       // Scale
        });

        // Divider for second L-couch back
        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(2.0f, groundLevel, 3.0f),   // Position (behind second L-couch)
            glm::vec3(0.0f, 90.0f, 0.0f),         // Rotation
            glm::vec3(0.7f)                       // Scale
        });
    }
    
    // 15-16. Loading CubicCouch (2 instances)
    auto cubicCouch_materialGroups = loadObjModel("Objects/cubicCouch.obj", reader_config);
    if (!cubicCouch_materialGroups.empty()) {
         furnitureCollection.push_back({
            cubicCouch_materialGroups,
            glm::vec3(1.8f, groundLevel, 1.0f),   // Position
            glm::vec3(0.0f, 90.0f, 0.0f),         // Rotation
            glm::vec3(0.7f, 0.7f, 0.7f)                      // Scale
        });
    }
    
    // 17-20. Loading Small Tables (4 instances)
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
       
    // 21-24. Loading Tall Tables (4 instances - 2 pairs that are close to each other)
    auto tallTable_materialGroups = loadObjModel("Objects/tallTable.obj", reader_config);
    if (!tallTable_materialGroups.empty()) {
        furnitureCollection.push_back({
            tallTable_materialGroups,
            glm::vec3(-2.0f, groundLevel, 12.0f),  // Moved further south (positive Z)
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(0.7f)                      // Scale
        });
        
        // Tall Table 2 (south, spaced from first)
        furnitureCollection.push_back({
            tallTable_materialGroups,
            glm::vec3(-1.0f, groundLevel, 12.0f),  // Moved further south (positive Z), spaced
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(0.7f)                      // Scale
        });
    }
    //     auto potPlant_materialGroups = loadObjModel("Objects/potPlant.obj", reader_config);
    // if (!potPlant_materialGroups.empty()) {
    //     // Short Table 1 (north)
    //     furnitureCollection.push_back({
    //         potPlant_materialGroups,
    //         glm::vec3(5.0f, groundLevel, 15.0f),  // Moved even further north (negative Z)
    //         glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
    //         glm::vec3(1.0f)                      // Scale
    //     });}
    
    // 25-26. Loading Short Tables (2 instances)
    // auto shortTable_materialGroups = loadObjModel("Objects/shortTable.obj", reader_config);
    // if (!shortTable_materialGroups.empty()) {
    //     // Short Table 1
    //     furnitureCollection.push_back({
    //           shortTable_materialGroups,
    //         glm::vec3(-4.0f, groundLevel, -16.0f),  // Moved even further north (negative Z)
    //         glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
    //         glm::vec3(1.0f)                      // Scale
    //     });
        
    //     // Short Table 2
    //     furnitureCollection.push_back({
    //          shortTable_materialGroups,
    //         glm::vec3(2.0f, groundLevel, -13.0f),   // Moved even further north (negative Z)
    //         glm::vec3(0.0f, 90.0f, 0.0f),        // Rotation
    //         glm::vec3(1.0f)                      // Scale
    //     });
    // }
    
    // // 27-30. Loading Comfortable Chairs (4 instances)
    // auto comfortableChair_materialGroups = loadObjModel("Objects/comfortableChair.obj", reader_config);
    // if (!comfortableChair_materialGroups.empty()) {
    //     // Near Short Table 1
    //     // Chair 1
    //     furnitureCollection.push_back({
    //        comfortableChair_materialGroups,
    //         glm::vec3(-3.0f, groundLevel, -15.0f), // Position closer to table, facing table
    //         glm::vec3(0.0f, 180.0f, 0.0f),        // Rotate to face north (towards table)
    //         glm::vec3(1.0f)                      // Scale
    //     });
        
    //     // Chair 2
    //     furnitureCollection.push_back({
    //          comfortableChair_materialGroups,
    //         glm::vec3(-9.5f, groundLevel, -14.5f),  // Position closer to table, facing table
    //         glm::vec3(0.0f, 0.0f, 0.0f),          // Rotate to face south (towards table)
    //         glm::vec3(1.0f)                      // Scale
    //     });
        
    //     // Near Short Table 2
    //     // Chair 3
    //     furnitureCollection.push_back({
    //          comfortableChair_materialGroups,
    //         glm::vec3(7.5f, groundLevel, -12.0f), // Position closer to table, facing table
    //         glm::vec3(0.0f, 180.0f, 0.0f),        // Rotate to face north (towards table)
    //         glm::vec3(1.0f)                      // Scale
    //     });
        
    //     // Chair 4
    //    furnitureCollection.push_back({
    //         comfortableChair_materialGroups,
    //         glm::vec3(0.0f, groundLevel, -13.0f),  // Position closer to table, facing table
    //         glm::vec3(0.0f, 0.0f, 0.0f),          // Rotate to face south (towards table)
    //         glm::vec3(1.0f)                      // Scale
    //     });
    //      auto bigTable_materialGroups = loadObjModel("Objects/BigTable.obj", reader_config);
    //     if (!bigTable_materialGroups.empty()) {
    //         float bigTableZ = 5.0f; 
    //         float bigTableX = -3.0f; 

    //         furnitureCollection.push_back({
    //             bigTable_materialGroups,
    //             glm::vec3(bigTableX, groundLevel, bigTableZ),
    //             glm::vec3(0.0f, 90.0f, 0.0f), 
    //             glm::vec3(1.0f) 
    //         });

    //         float bigChairOffsetZ = -1.5f; 
    //         float bigChairOffsetX = 1.5f; 

    //         furnitureCollection.push_back({
    //             comfortableChair_materialGroups,
    //             glm::vec3(bigTableX - bigChairOffsetX, groundLevel, bigTableZ + bigChairOffsetZ), 
    //             glm::vec3(0.0f, 90.0f, 0.0f), 
    //             glm::vec3(1.0f)
    //         });
    //         furnitureCollection.push_back({
    //             comfortableChair_materialGroups,
    //             glm::vec3(bigTableX + bigChairOffsetX, groundLevel, bigTableZ + bigChairOffsetZ), 
    //             glm::vec3(0.0f, 90.0f, 0.0f), 
    //             glm::vec3(1.0f)
    //         });

    //         furnitureCollection.push_back({
    //             comfortableChair_materialGroups,
    //             glm::vec3(bigTableX +1.5f, groundLevel, bigTableZ -10.0f), 
    //             glm::vec3(0.0f, 270.0f, 0.0f), 
    //             glm::vec3(1.0f)
    //         });
    //         furnitureCollection.push_back({
    //             comfortableChair_materialGroups,
    //             glm::vec3(bigTableX + 3.5f, groundLevel, bigTableZ -10.0f), 
    //             glm::vec3(0.0f, 270.0f, 0.0f), 
    //             glm::vec3(1.0f)
    //         });
    //     }
    
    // }

    auto tallChair_materialGroups = loadObjModel("Objects/tallChairs.obj", reader_config);
    if (!tallChair_materialGroups.empty()) {
        // Chair for Tall Table 1
        // Tall Table 1 is at (-1.0f, groundLevel, 12.0f)
        furnitureCollection.push_back({
            tallChair_materialGroups,
            glm::vec3(-1.0f, groundLevel, 11.0f), // Slightly less Z to be behind/close to table
            glm::vec3(0.0f, 0.0f, 0.0f), // Rotate to face towards the table (north)
            glm::vec3(0.8f)
        });

        // Chair for Tall Table 2
        // Tall Table 2 is at (3.0f, groundLevel, 12.0f)
        furnitureCollection.push_back({
            tallChair_materialGroups,
            glm::vec3(3.0f, groundLevel, 11.0f), // Slightly less Z to be behind/close to table
            glm::vec3(0.0f, 0.0f, 0.0f), // Rotate to face towards the table (north)
            glm::vec3(0.8f)
        });
    }
    
    // --- Load Room Components ---
    // Load Carpet (ground)
    std::vector<MaterialGroup> carpet0_materialGroups = loadObjModel("carpet_0.obj", reader_config);
    std::vector<MaterialGroup> carpet1_materialGroups = loadObjModel("carpet_1.obj", reader_config);
    std::vector<MaterialGroup> carpet2_materialGroups = loadObjModel("carpet_2.obj", reader_config);

    // Load Roof
    std::vector<MaterialGroup> roof_materialGroups = loadObjModel("alt_panels.obj", reader_config);

    // Load NorthWall
    std::vector<MaterialGroup> northwall_materialGroups = loadObjModel("N_SWall.obj", reader_config);


<<<<<<< HEAD
=======
<<<<<<< HEAD
<<<<<<< HEAD
    WindowWall wall(8,8,0.9,1.5); //default size is 8x8, but we can do this in a scene generator class
    Wall westWall(4.0f, 10.0f, 0.2f, 5, 8);

    LightingManager light;
=======
>>>>>>> 729611f (placed walls and roof)
    // East and West Walls
    WindowWall wall(30,25,0.9,1.5); //default size is 8x8, but we can do this in a scene generator class
    Wall westWall(4.0f, 10.0f, 0.2f, 5, 8);
        
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide and capture mouse cursor
    glfwSetCursorPosCallback(window, mouse_callback); // Register the callback function
    LightingManager light;

    // Main loop
    do {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input processing (keyboard movement)
        float cameraSpeed = 10.0f * deltaTime; // Adjust this value to change speed
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) // Move up
            cameraPos += cameraSpeed * cameraUp;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) // Move down
            cameraPos -= cameraSpeed * cameraUp;

        // Update the view matrix (camera "looks at" cameraFront from cameraPos)
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glClearColor(0.678f, 0.847f, 0.902f, 1.0f); // background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        light.upload(shaderProgram);
        light.upload(shaderProgram);

        // Set up matrices uniforms
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // --- Render Carpet ---
        const float CARPET_SCALE = 0.25f;
        const float CARPET_Y_POSITION = 8.0f; // Common Y position for all carpets (assuming flat on floor)
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
                    glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
                    glBindVertexArray(group.VAO);
                    glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
                }
            }
        }


        // --- Render Roof ---
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

        // --- Render NorthWall ---
        if (!northwall_materialGroups.empty()) {
            glm::mat4 northWallModel = glm::mat4(1.0f); // Identity, or adjust if you want transforms
            northWallModel = glm::scale(northWallModel, glm::vec3(0.5f)); // Shrink it if it's too big
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
            // Apply mirror scale (e.g., along X-axis to reflect across YZ plane)
            southWallModel = glm::scale(southWallModel, glm::vec3(-1.0f, 1.0f, 1.0f));
            // Apply the original transformations (or adjust as needed for the mirrored object)
            southWallModel = glm::scale(southWallModel, glm::vec3(0.5f));
            southWallModel = glm::translate(southWallModel, glm::vec3(0.5f, 0.0f, 0.0f)); // Same translation as original
            southWallModel = glm::rotate(southWallModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        
            southWallModel = glm::translate(southWallModel, glm::vec3(-10.0f, 0.0f, 0.0f)); // Example adjustment

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(southWallModel));
            for (const auto& group : northwall_materialGroups) {
                GLuint colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
                glUniform4f(colorLoc, group.color.r, group.color.g, group.color.b, 1.0f);
                glBindVertexArray(group.VAO);
                glDrawArrays(GL_TRIANGLES, 0, group.vertexCount);
            }
<<<<<<< HEAD
        }

        // Render all furniture
        for (const auto& furniture : furnitureCollection) {
            furniture.render(shaderProgram);
=======
>>>>>>> ee422df (placed walls and roof)
        }

        // // Render all furniture
        // for (const auto& furniture : furnitureCollection) {
        //     furniture.render(shaderProgram);
        // }

        //Render East and West Walls
        // wall.draw(view, projection, shaderProgram); //Uncomment the draw call to see the wall
        // westWall.draw(view, projection, shaderProgram);
        //westWall.draw(view, projection, shaderProgram);

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
    // Cleanup for room objects
    for (const auto& group : carpet0_materialGroups) {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
    }
    for (const auto& group : carpet1_materialGroups) {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
    }
    for (const auto& group : carpet2_materialGroups) {
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

