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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Include stb_image.h

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

// Your MaterialGroup definition - UPDATED to include texture ID
struct MaterialGroup
{
    GLuint VAO;
    GLuint VBO;
    GLsizei vertexCount;
    glm::vec3 color;
    GLuint textureID; // New: Texture ID for this material group
    bool hasTexture;  // New: Flag to indicate if this group has a texture
};

// Interleaved vertex structure - UPDATED to include texture coordinates
struct InterleavedVertex
{
    float px, py, pz; // Position
    float nx, ny, nz; // Normal
    float tx, ty;     // Texture coordinates
};

// Helper function to load a texture using stb_image
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
std::vector<MaterialGroup> loadObjModel(const std::string &filename, const tinyobj::ObjReaderConfig &config)
{
    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(filename, config))
    {
        std::cerr << "TinyObjReader failed to load " << filename << ": " << reader.Error() << std::endl;
        return {};
    }

    if (!reader.Warning().empty())
    {
        std::cout << "TinyObjReader warning (" << filename << "): " << reader.Warning() << std::endl;
    }

    const auto &attrib = reader.GetAttrib();
    const auto &shapes = reader.GetShapes();
    const auto &materials = reader.GetMaterials();

    std::vector<MaterialGroup> materialGroups;

    for (const auto &shape : shapes)
    {
        std::unordered_map<int, std::vector<InterleavedVertex>> materialVertexMap;

        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f)
        {
            int fv = shape.mesh.num_face_vertices[f];
            int mat_id = shape.mesh.material_ids[f];

            for (size_t v = 0; v < fv; ++v)
            {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                InterleavedVertex vertex;
                vertex.px = attrib.vertices[3 * idx.vertex_index + 0];
                vertex.py = attrib.vertices[3 * idx.vertex_index + 1];
                vertex.pz = attrib.vertices[3 * idx.vertex_index + 2];

                if (!attrib.normals.empty() && idx.normal_index >= 0)
                {
                    vertex.nx = attrib.normals[3 * idx.normal_index + 0];
                    vertex.ny = attrib.normals[3 * idx.normal_index + 1];
                    vertex.nz = attrib.normals[3 * idx.normal_index + 2];
                }
                else
                {
                    vertex.nx = 0.0f;
                    vertex.ny = 0.0f;
                    vertex.nz = 1.0f; // fallback
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

        for (const auto &[mat_id, verts] : materialVertexMap)
        {
            GLuint VAO, VBO;
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(InterleavedVertex), verts.data(), GL_STATIC_DRAW);

            // Position: location 0
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(InterleavedVertex), (void *)0);
            glEnableVertexAttribArray(0);

            // Normal: location 1
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(InterleavedVertex), (void *)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            // Texture coordinates: location 2
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(InterleavedVertex), (void *)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);

            glm::vec3 color(0.8f); // default color
            GLuint textureID = 0;  // default to no texture
            bool hasTexture = false;

            if (mat_id >= 0 && mat_id < (int)materials.size())
            {
                const auto &m = materials[mat_id];
                color = glm::vec3(m.diffuse[0], m.diffuse[1], m.diffuse[2]);

                // Load texture if diffuse texture map is specified
                if (!m.diffuse_texname.empty())
                {
                    std::string texturePath = m.diffuse_texname; // tinyobjloader should handle relative paths from where the .obj is loaded
                    textureID = loadTexture(texturePath);
                    hasTexture = true;
                }
            }

            materialGroups.push_back({VAO, VBO, static_cast<GLsizei>(verts.size()), color, textureID, hasTexture});
        }
    }

    return materialGroups;
}

// Structure to manage furniture with position, rotation, and scale
struct Furniture
{
    std::vector<MaterialGroup> materialGroups;
    glm::vec3 position;
    glm::vec3 rotation; // in degrees
    glm::vec3 scale;

    void render(GLuint shaderProgram) const
    {
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

        for (const auto &group : materialGroups)
        {
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

    // Enable depth testing and disable face culling
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Load shaders (assuming "vertex_shader.glsl" and "fragment_shader.glsl" are correct for both objects)
    shaderProgram = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

    // Set camera to view the entire room and furniture
    view = glm::lookAt(
        glm::vec3(20.0f, 15.0f, 20.0f), // Position further back and higher to see full room
        glm::vec3(0.0f, 0.0f, 0.0f),    // Look at the origin
        glm::vec3(0.0f, 1.0f, 0.0f));

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
    if (!blue_materialGroups.empty())
    {
        // First blue carpet
        furnitureCollection.push_back({
            blue_materialGroups,
            glm::vec3(7.0f, groundLevel, 12.0f), // Position
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotation
            glm::vec3(1.0f)                       // Scale
        });

        // Second blue carpet
        furnitureCollection.push_back({
            blue_materialGroups,
            glm::vec3(6.0f, groundLevel, 14.0f), // Position
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotation
            glm::vec3(1.0f)                       // Scale
        });
    }

    // 3-4. Loading Yellow (2 instances)
    auto yellow_materialGroups = loadObjModel("Objects/yellow.obj", reader_config);
    if (!yellow_materialGroups.empty())
    {
        // First yellow
        furnitureCollection.push_back({
            yellow_materialGroups,
            glm::vec3(4.0f, groundLevel, 16.0f), // Position
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotation
            glm::vec3(1.0f)                       // Scale
        });
    }

    // 5-7. Loading LCouch (3 instances - 2 connected, 1 separate)
    auto lcouch_materialGroups = loadObjModel("Objects/LCouch.obj", reader_config);
    if (!lcouch_materialGroups.empty())
    {
        // First LCouch (part of connected pair)
        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(0.0f, groundLevel, 9.5f), // Position
            glm::vec3(0.0f, 0.0f, 0.0f),        // Rotation
            glm::vec3(1.0f)                     // Scale
        });

        // Second LCouch (connected to first)
        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(3.0f, groundLevel, 9.5f), // Position
            glm::vec3(0.0f, 90.0f, 0.0f),       // Rotation for L-shape
            glm::vec3(1.0f)                     // Scale
        });

        // Third LCouch (separate)
        furnitureCollection.push_back({
            lcouch_materialGroups,
            glm::vec3(-1.0f, groundLevel, -8.0f), // Position
            glm::vec3(0.0f, 90.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                       // Scale
        });
    }

    // 8-12. Loading Ornaments (5 instances)
    auto ornament_materialGroups = loadObjModel("Objects/Ornament.obj", reader_config);
    auto cutoffs_materialGroups = loadObjModel("Objects/cutoffs.obj", reader_config); // Load the cutoffs model

    if (!ornament_materialGroups.empty())
    {
        float startZ = 13.0f;           // Southmost position
        float spacing = 6.0f;           // Spacing between ornaments
        float ornamentX = -22.0f;       // Further left position
        float cutoffOffset = spacing / 2.0f; // Halfway between ornaments

        for (int i = 0; i < 4; ++i)
        {
            furnitureCollection.push_back({ornament_materialGroups,
                                           glm::vec3(ornamentX, groundLevel, startZ - (i * spacing)), // Consistent X, varying Z
                                           glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f)});

            // Add cutoffs between ornaments
            if (i < 3 && !cutoffs_materialGroups.empty())
            { // Add 3 cutoffs between 4 ornaments
                furnitureCollection.push_back({
                    cutoffs_materialGroups,
                    glm::vec3(ornamentX + 2.0f, groundLevel, startZ - (i * spacing) + cutoffOffset + 2.5f), // Position halfway
                    glm::vec3(0.0f, 180.0f, 0.0f),                                                              // Rotation
                    glm::vec3(1.0f)                                                                             // Scale
                });
            }
        }
    } // 13-14. Loading Dividers (2 instances)
    auto divider_materialGroups = loadObjModel("Objects/divider.obj", reader_config);
    if (!divider_materialGroups.empty())
    {
        // Divider 1
        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(0.0f, groundLevel, -7.3f), // Position (top side of cubic couch)
            glm::vec3(0.0f, 90.0f, 0.0f),        // Rotation
            glm::vec3(1.0f)                      // Scale
        });

        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(0.0f, groundLevel, 9.0f), // Position (behind first L-couch)
            glm::vec3(0.0f, 0.0f, 0.0f),        // Rotation
            glm::vec3(1.0f)                     // Scale
        });

        // Divider for second L-couch back
        furnitureCollection.push_back({
            divider_materialGroups,
            glm::vec3(2.0f, groundLevel, 8.5f), // Position (behind second L-couch)
            glm::vec3(0.0f, 90.0f, 0.0f),       // Rotation
            glm::vec3(1.0f)                     // Scale
        });
    }

    // 15-16. Loading CubicCouch (2 instances)
    auto cubicCouch_materialGroups = loadObjModel("Objects/cubicCouch.obj", reader_config);
    if (!cubicCouch_materialGroups.empty())
    {
        furnitureCollection.push_back({
            cubicCouch_materialGroups,
            glm::vec3(18.0f, groundLevel, -17.0f), // Position
            glm::vec3(0.0f, 85.0f, 0.0f),          // Rotation
            glm::vec3(1.0f)                        // Scale
        });
    }

    // 17-20. Loading Small Tables (4 instances)
    auto smallTable_materialGroups = loadObjModel("Objects/smallTable.obj", reader_config);
    if (!smallTable_materialGroups.empty())
    {
        // Small Table 1
        furnitureCollection.push_back({
            smallTable_materialGroups,
            glm::vec3(-2.0f, groundLevel, 7.0f), // Position
            glm::vec3(0.0f, 0.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                      // Scale
        });
    }

    // 21-24. Loading Tall Tables (4 instances - 2 pairs that are close to each other)
    auto tallTable_materialGroups = loadObjModel("Objects/tallTable.obj", reader_config);
    if (!tallTable_materialGroups.empty())
    {
        furnitureCollection.push_back({
            tallTable_materialGroups,
            glm::vec3(-1.0f, groundLevel, 12.0f), // Moved further south (positive Z)
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotation
            glm::vec3(1.0f)                       // Scale
        });

        // Tall Table 2 (south, spaced from first)
        furnitureCollection.push_back({
            tallTable_materialGroups,
            glm::vec3(3.0f, groundLevel, 12.0f), // Moved further south (positive Z), spaced
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotation
            glm::vec3(1.0f)                       // Scale
        });
    }
    auto potPlant_materialGroups = loadObjModel("Objects/potPlant.obj", reader_config);
    if (!potPlant_materialGroups.empty())
    {
        // Short Table 1 (north)
        furnitureCollection.push_back({
            potPlant_materialGroups,
            glm::vec3(5.0f, groundLevel, 15.0f), // Moved even further north (negative Z)
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotation
            glm::vec3(1.0f)                       // Scale
        });
    }

    // 25-26. Loading Short Tables (2 instances)
    auto shortTable_materialGroups = loadObjModel("Objects/shortTable.obj", reader_config);
    if (!shortTable_materialGroups.empty())
    {
        // Short Table 1
        furnitureCollection.push_back({
            shortTable_materialGroups,
            glm::vec3(-4.0f, groundLevel, -16.0f), // Moved even further north (negative Z)
            glm::vec3(0.0f, 0.0f, 0.0f),            // Rotation
            glm::vec3(1.0f)                         // Scale
        });

        // Short Table 2
        furnitureCollection.push_back({
            shortTable_materialGroups,
            glm::vec3(2.0f, groundLevel, -13.0f), // Moved even further north (negative Z)
            glm::vec3(0.0f, 90.0f, 0.0f),         // Rotation
            glm::vec3(1.0f)                       // Scale
        });
    }

    // 27-30. Loading Comfortable Chairs (4 instances)
    auto comfortableChair_materialGroups = loadObjModel("Objects/comfortableChair.obj", reader_config);
    if (!comfortableChair_materialGroups.empty())
    {
        // Near Short Table 1
        // Chair 1
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(-3.0f, groundLevel, -15.0f), // Position closer to table, facing table
            glm::vec3(0.0f, 180.0f, 0.0f),         // Rotate to face north (towards table)
            glm::vec3(1.0f)                        // Scale
        });

        // Chair 2
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(-9.5f, groundLevel, -14.5f), // Position closer to table, facing table
            glm::vec3(0.0f, 0.0f, 0.0f),           // Rotate to face south (towards table)
            glm::vec3(1.0f)                        // Scale
        });

        // Near Short Table 2
        // Chair 3
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(7.5f, groundLevel, -12.0f), // Position closer to table, facing table
            glm::vec3(0.0f, 180.0f, 0.0f),        // Rotate to face north (towards table)
            glm::vec3(1.0f)                       // Scale
        });

        // Chair 4
        furnitureCollection.push_back({
            comfortableChair_materialGroups,
            glm::vec3(0.0f, groundLevel, -13.0f), // Position closer to table, facing table
            glm::vec3(0.0f, 0.0f, 0.0f),          // Rotate to face south (towards table)
            glm::vec3(1.0f)                       // Scale
        });
        auto bigTable_materialGroups = loadObjModel("Objects/BigTable.obj", reader_config);
        if (!bigTable_materialGroups.empty())
        {
            float bigTableZ = 5.0f;
            float bigTableX = -3.0f;

            furnitureCollection.push_back({bigTable_materialGroups,
                                           glm::vec3(bigTableX, groundLevel, bigTableZ),
                                           glm::vec3(0.0f, 90.0f, 0.0f),
                                           glm::vec3(1.0f)});

            float bigChairOffsetZ = -1.5f;
            float bigChairOffsetX = 1.5f;

            furnitureCollection.push_back({comfortableChair_materialGroups,
                                           glm::vec3(bigTableX - bigChairOffsetX, groundLevel, bigTableZ + bigChairOffsetZ),
                                           glm::vec3(0.0f, 90.0f, 0.0f),
                                           glm::vec3(1.0f)});
            furnitureCollection.push_back({comfortableChair_materialGroups,
                                           glm::vec3(bigTableX + bigChairOffsetX, groundLevel, bigTableZ + bigChairOffsetZ),
                                           glm::vec3(0.0f, 90.0f, 0.0f),
                                           glm::vec3(1.0f)});

            furnitureCollection.push_back({comfortableChair_materialGroups,
                                           glm::vec3(bigTableX + 1.5f, groundLevel, bigTableZ - 10.0f),
                                           glm::vec3(0.0f, 270.0f, 0.0f),
                                           glm::vec3(1.0f)});
            furnitureCollection.push_back({comfortableChair_materialGroups,
                                           glm::vec3(bigTableX + 3.5f, groundLevel, bigTableZ - 10.0f),
                                           glm::vec3(0.0f, 270.0f, 0.0f),
                                           glm::vec3(1.0f)});
        }
    }

    auto tallChair_materialGroups = loadObjModel("Objects/tallChairs.obj", reader_config);
    if (!tallChair_materialGroups.empty())
    {
        // Chair for Tall Table 1
        // Tall Table 1 is at (-1.0f, groundLevel, 12.0f)
        furnitureCollection.push_back({tallChair_materialGroups,
                                       glm::vec3(-1.0f, groundLevel, 11.0f), // Slightly less Z to be behind/close to table
                                       glm::vec3(0.0f, 0.0f, 0.0f),          // Rotate to face towards the table (north)
                                       glm::vec3(1.0f)});

        // Chair for Tall Table 2
        // Tall Table 2 is at (3.0f, groundLevel, 12.0f)
        furnitureCollection.push_back({tallChair_materialGroups,
                                       glm::vec3(3.0f, groundLevel, 11.0f), // Slightly less Z to be behind/close to table
                                       glm::vec3(0.0f, 0.0f, 0.0f),          // Rotate to face towards the table (north)
                                       glm::vec3(1.0f)});
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

    WindowWall wall(8, 8, 0.9, 1.5); // default size is 8x8, but we can do this in a scene generator class
    Wall westWall(4.0f, 10.0f, 0.2f, 5, 8);

    LightingManager light;
    // Main loop
    do
    {
        glfwPollEvents();

        glClearColor(0.678f, 0.847f, 0.902f, 1.0f); // background color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Set up matrices uniforms
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Get uniform location for hasTexture
        GLint hasTextureLoc = glGetUniformLocation(shaderProgram, "hasTexture");
        if (hasTextureLoc == -1) {
            std::cerr << "Uniform 'hasTexture' not found!" << std::endl;
        }

        // --- Render Carpet ---
        if (!carpet0_materialGroups.empty())
        {
            glm::mat4 carpetModel = glm::mat4(1.0f);
            carpetModel = glm::scale(carpetModel, glm::vec3(0.15f));          // Adjust scale
            carpetModel = glm::translate(carpetModel, glm::vec3(0.0f, 50.0f, 0.0f)); // Adjust position
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(carpetModel));
            for (const auto &group : carpet0_materialGroups)
            {
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

        if (!carpet1_materialGroups.empty())
        {
            glm::mat4 carpetModel = glm::mat4(1.0f);
            carpetModel = glm::scale(carpetModel, glm::vec3(0.15f));            // Adjust scale
            carpetModel = glm::translate(carpetModel, glm::vec3(-15.0f, 50.0f, 0.0f)); // Adjust position
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(carpetModel));
            for (const auto &group : carpet1_materialGroups)
            {
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

        if (!carpet2_materialGroups.empty())
        {
            glm::mat4 carpetModel = glm::mat4(1.0f);
            carpetModel = glm::scale(carpetModel, glm::vec3(0.15f));            // Adjust scale
            carpetModel = glm::translate(carpetModel, glm::vec3(-30.0f, 50.0f, 0.0f)); // Adjust position
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(carpetModel));
            for (const auto &group : carpet2_materialGroups)
            {
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

        // --- Render Roof ---
        if (!roof_materialGroups.empty())
        {
            glm::mat4 roofModel = glm::mat4(1.0f);
            roofModel = glm::scale(roofModel, glm::vec3(0.15f));          // Adjust scale
            roofModel = glm::translate(roofModel, glm::vec3(0.0f, 10.0f, 0.0f)); // Adjust position
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(roofModel));
            for (const auto &group : roof_materialGroups)
            {
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

        // --- Render NorthWall ---
        if (!northwall_materialGroups.empty())
        {
            glm::mat4 northWallModel = glm::mat4(1.0f);            // Identity, or adjust if you want transforms
            northWallModel = glm::scale(northWallModel, glm::vec3(0.15f)); // Shrink it if it's too big
            northWallModel = glm::translate(northWallModel, glm::vec3(0.5f, 0.0f, 0.0f));
            northWallModel = glm::rotate(northWallModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(northWallModel));
            for (const auto &group : northwall_materialGroups)
            {
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

        // Render all furniture
        for (const auto &furniture : furnitureCollection)
        {
            furniture.render(shaderProgram);
        }

        // Render East and West Walls
        wall.draw(view, projection, shaderProgram); // Uncomment the draw call to see the wall
                                                    // westWall.draw(view, projection, shaderProgram);

        // --- Manual Time-of-Day Setup ---
        static float timeOfDay = 12.0f; // Start at midday
        const float deltaTime = 0.1f;    // Speed of time change per frame

        // User input to adjust time (left/right arrows)
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            timeOfDay -= deltaTime;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            timeOfDay += deltaTime;

        // Clamp/wrap time to 0–24 hours
        timeOfDay = fmod(timeOfDay + 24.0f, 24.0f);

        // Set light color and intensity based on time
        glm::vec3 sunColor;
        float intensity;

        if (timeOfDay >= 6.0f && timeOfDay < 9.0f)
        {
            sunColor = glm::vec3(1.0f, 0.65f, 0.4f); // Morning sunrise
            intensity = 0.5f;
        }
        else if (timeOfDay >= 9.0f && timeOfDay < 17.0f)
        {
            sunColor = glm::vec3(1.0f, 0.95f, 0.85f); // Midday soft sunlight
            intensity = 1.0f;
        }
        else if (timeOfDay >= 17.0f && timeOfDay < 20.0f)
        {
            sunColor = glm::vec3(1.0f, 0.5f, 0.25f); // Evening sunset
            intensity = 0.4f;
        }
        else
        {
            sunColor = glm::vec3(0.1f, 0.1f, 0.3f); // Night bluish tint
            intensity = 0.15f;
        }

        // Final light color to send to shader
        glm::vec3 lightColor = sunColor * intensity;

        light.upload(shaderProgram, lightColor);

        glfwSwapBuffers(window);
        glfwPollEvents();

    } while (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0);

    // Cleanup - Free all VAOs and VBOs
    for (const auto &furniture : furnitureCollection)
    {
        for (const auto &group : furniture.materialGroups)
        {
            glDeleteVertexArrays(1, &group.VAO);
            glDeleteBuffers(1, &group.VBO);
            if (group.hasTexture) {
                glDeleteTextures(1, &group.textureID);
            }
        }
    }
    // Cleanup for room objects
    for (const auto &group : carpet0_materialGroups)
    {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
        if (group.hasTexture) {
            glDeleteTextures(1, &group.textureID);
        }
    }
    for (const auto &group : carpet1_materialGroups)
    {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
        if (group.hasTexture) {
            glDeleteTextures(1, &group.textureID);
        }
    }
    for (const auto &group : carpet2_materialGroups)
    {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
        if (group.hasTexture) {
            glDeleteTextures(1, &group.textureID);
        }
    }
    for (const auto &group : roof_materialGroups)
    {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
        if (group.hasTexture) {
            glDeleteTextures(1, &group.textureID);
        }
    }
    for (const auto &group : northwall_materialGroups)
    {
        glDeleteVertexArrays(1, &group.VAO);
        glDeleteBuffers(1, &group.VBO);
        if (group.hasTexture) {
            glDeleteTextures(1, &group.textureID);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}