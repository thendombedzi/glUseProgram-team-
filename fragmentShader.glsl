#version 330 core

out vec4 FragColor;

in vec3  FragPos;    
in vec3  Normal;     
in vec2  TexCoord;
in vec4  FragPosLightSpace;

uniform vec4  objectColor;
uniform sampler2D textureSampler;
uniform int hasTexture;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 lightDir;

uniform float alpha;
uniform int mode;

uniform sampler2D shadowMap;
uniform float shadowBias;

// --- Shadow Calculation ---
float calculateShadow(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    //float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
    {
       for (int y = -1; y <= 1; ++y)
       {
          float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
          shadow += currentDepth - shadowBias > pcfDepth ? 1.0 : 0.0;
       }
    } 
    shadow /= 9.0;

    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{
    // --- Lighting ---
    float ambientStrength = 0.6;
    vec3 ambient = ambientStrength * lightColor;

    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor * 0.4;

    float shadow = calculateShadow(FragPosLightSpace);
    vec3 lighting = ambient + (1.0 - shadow) * diffuse;

    // --- Color Selection ---
    vec4 texColor = texture(textureSampler, TexCoord);
    vec4 baseColor = mix(objectColor, texColor, float(hasTexture));

    // --- Final Lit Color ---
    vec3 litRGB = baseColor.rgb * lighting;
    float finalAlpha = baseColor.a * alpha;
    vec4 finalColor = vec4(litRGB, finalAlpha);

    // --- Post FX ---
    if (mode == 1) {
        float intensity = dot(finalColor.rgb, vec3(0.2126, 0.7152, 0.0722));
        finalColor = vec4(vec3(0.1, 1.0, 0.1) * intensity * 1.5, finalAlpha);
    } else if (mode == 2) {
        float grey = dot(finalColor.rgb, vec3(0.299, 0.587, 0.114));
        finalColor = vec4(vec3(grey), finalAlpha);
    } else if (mode == 3) {
        finalColor = vec4(vec3(1.0) - finalColor.rgb, finalAlpha);
    }

    FragColor = finalColor;
}
