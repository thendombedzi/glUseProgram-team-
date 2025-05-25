#version 330 core

out vec4 FragColor;

in vec3 FragPos;    
in vec3 Normal;     
in vec2 TexCoord;   

uniform vec4 objectColor;    
uniform sampler2D textureSampler; 
uniform int hasTexture;      

uniform vec3 lightPos;       
uniform vec3 lightColor;

uniform vec3 lightDir;       // For directional light (night vision effect)
uniform vec3 viewPos;        

uniform int mode; // 0 = normal, 1 = night vision, 2 = grayscale, 3 = inverted

void main()
{
    // --- Lighting calculations ---
    float ambientStrength = 0.6;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 directionalLight = normalize(-lightDir); // For post-processing (night vision)
    vec3 lightDirection = normalize(lightPos - FragPos); // For point light (main shading)

    float diff = max(dot(norm, lightDirection), 0.0);
    vec3 diffuse = diff * lightColor * 0.4;

    vec3 lighting = min(ambient + diffuse, vec3(1.0));
    
    // --- Base color calculation ---
    vec4 baseColor;
    if (hasTexture == 1) {
        vec4 texColor = texture(textureSampler, TexCoord);
        baseColor = vec4(texColor.rgb + lighting * 0.05, texColor.a); // Slight lighting boost
    } else {
        baseColor = vec4(objectColor.rgb * lighting, objectColor.a);
    }

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
