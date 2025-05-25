#version 330 core

out vec4 FragColor;

in vec3 FragPos;    
in vec3 Normal;     
in vec2 TexCoord;   

uniform vec4 objectColor;            // Base RGBA object color
uniform sampler2D textureSampler;    
uniform int hasTexture;              // 0 = solid color, 1 = texture

uniform vec3 lightPos;               
uniform vec3 lightColor;
uniform vec3 lightDir;               // Used for night vision post-process

uniform float alpha;                 // Alpha override from material (e.g., MTL)

uniform int mode; // 0 = normal, 1 = night vision, 2 = grayscale, 3 = inverted

void main()
{
    // --- Lighting calculations ---
    float ambientStrength = 0.6;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDirection = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor * 0.4;

    vec3 lighting = min(ambient + diffuse, vec3(1.0));

    // --- Base color from texture or objectColor ---
    vec4 baseColor;
    if (hasTexture == 1) {
        vec4 texColor = texture(textureSampler, TexCoord);
        baseColor = vec4(texColor.rgb + lighting * 0.05, texColor.a); // Boost lighting slightly
    } else {
        baseColor = vec4(objectColor.rgb * lighting, objectColor.a);
    }

    // Override alpha if provided (assumes 0.0–1.0 valid input)
    baseColor.a = alpha;

    // --- Post-processing effects ---
    vec4 finalColor = baseColor;

    if (mode == 1) {
        // Night vision
        float intensity = dot(baseColor.rgb, vec3(0.2126, 0.7152, 0.0722)); 
        vec3 nightVision = vec3(0.1, 1.0, 0.1) * intensity * 1.5;
        finalColor = vec4(nightVision, baseColor.a);
    }
    else if (mode == 2) {
        // Grayscale
        float grey = dot(baseColor.rgb, vec3(0.299, 0.587, 0.114));
        finalColor = vec4(vec3(grey), baseColor.a);
    }
    else if (mode == 3) {
        // Inverted
        finalColor = vec4(vec3(1.0) - baseColor.rgb, baseColor.a);
    }

    FragColor = finalColor;
}
