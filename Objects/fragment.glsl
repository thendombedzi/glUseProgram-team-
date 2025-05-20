#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 Color;

// Material properties from the model
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
uniform Material material;

// Light properties
uniform vec3 lightPos = vec3(5.0, 5.0, 5.0);
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);

// Camera position for specular lighting
uniform vec3 viewPos;

void main()
{
    // Ambient lighting
    vec3 ambient = material.ambient * lightColor;
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * material.diffuse;
    
    // Specular lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * lightColor * material.specular;
    
    // Combine all components
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}