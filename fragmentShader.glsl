#version 330 core

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec4 objectColor;

uniform int mode; // 0 = normal, 1 = night vision, 2 = grayscale, 3 = inverted

out vec4 FragColor;

void main() {
    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(-lightDir);

    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 ambient = 0.6 * lightColor;

    vec3 lighting = (ambient + diffuse) * objectColor.rgb;
    vec4 color = vec4(lighting, objectColor.a);

    // Night vision
    if (mode == 1) {
        float intensity = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722)); 
        vec3 nightVision = vec3(0.1, 1.0, 0.1) * intensity * 1.5;
        color = vec4(nightVision, color.a);
    }
    // Grey scale
    else if (mode == 2) {
        float grey = dot(color.rgb, vec3(0.299, 0.587, 0.114));
        color = vec4(vec3(grey), color.a);
    } 
    // Inverted
    else if (mode == 3) {
        color = vec4(vec3(1.0) - color.rgb, color.a);
    }

    FragColor = color;
}
