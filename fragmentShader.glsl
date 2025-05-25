#version 330 core

out vec4 FragColor;

in vec3 FragPos;    // Interpolated fragment position from vertex shader
in vec3 Normal;     // Interpolated normal vector from vertex shader
in vec2 TexCoord;   // Interpolated texture coordinates from vertex shader

uniform vec4 objectColor;    // Color of the object if no texture
uniform sampler2D textureSampler; // Texture sampler
uniform int hasTexture;      // Flag to indicate if the object has a texture (0 for color, 1 for texture)

uniform vec3 lightPos;       // Position of the light source in world space
uniform vec3 lightColor;     // Color of the light

uniform vec3 viewPos;        // Camera position in world space (for specular if implemented later)

void main()
{
    // Ambient lighting - higher base to preserve texture visibility
    float ambientStrength = 0.6; // Higher ambient preserves texture colors
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * 0.4; // Scale down diffuse to prevent over-brightening

    // Combine lighting - clamp to prevent over-saturation
    vec3 lighting = min(ambient + diffuse, vec3(1.0));

    if (hasTexture == 1)
    {
        // Get texture color
        vec4 texColor = texture(textureSampler, TexCoord);
        
        // Multiply texture by lighting - this preserves color relationships
        FragColor = vec4(texColor.rgb + lighting * 0.05, texColor.a);
    }
    else
    {
        // Use object color with lighting
        FragColor = vec4(objectColor.rgb * lighting, objectColor.a);
    }
}