#version 330 core

layout (location = 0) in vec3 aPos;    // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal
layout (location = 2) in vec2 aTexCoord; // Vertex texture coordinates

uniform mat4 model;      // Model matrix
uniform mat4 view;       // View matrix
uniform mat4 projection; // Projection matrix

out vec3 FragPos;      // Fragment position in world space
out vec3 Normal;       // Normal vector in world space
out vec2 TexCoord;     // Texture coordinates to fragment shader

void main()
{
    // Calculate fragment position in world space
    // It's important to transform aPos by the model matrix first.
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Calculate normal in world space
    // The inverse transpose of the model matrix correctly transforms normals.
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Pass texture coordinates directly
    TexCoord = aTexCoord;

    // Calculate final position in clip space
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}