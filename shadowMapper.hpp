#ifndef SHADOW_MAPPER_HPP
#define SHADOW_MAPPER_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>

class ShadowMapper {
public:
    ShadowMapper(int width = 2048, int height = 2048);
    ~ShadowMapper();

    void beginDepthPass(GLuint depthShader, const glm::vec3& lightDir);
    void endDepthPass();

    void bindShadowUniforms(GLuint mainShader) const;
    glm::mat4 getLightSpaceMatrix() const;

private:
    GLuint depthFBO;
    GLuint depthMap;
    int shadowWidth, shadowHeight;
    glm::mat4 lightSpaceMatrix;
};

#endif
