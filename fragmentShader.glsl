#version 330 core

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec4 objectColor;

out vec4 FragColor;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir);

    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 ambient = 0.6 * lightColor;

    vec3 lighting = (ambient + diffuse) * objectColor.rgb;
    FragColor = vec4(lighting, objectColor.a);
}
