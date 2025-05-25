#include "ShadowMapper.hpp"
#include <glm/gtc/matrix_transform.hpp>

ShadowMapper::ShadowMapper(int width, int height)
    : shadowWidth(width), shadowHeight(height)
{
    glGenFramebuffers(1, &depthFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE); // No color buffer
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMapper::~ShadowMapper() {
    glDeleteFramebuffers(1, &depthFBO);
    glDeleteTextures(1, &depthMap);
}

void ShadowMapper::beginDepthPass(GLuint depthShader, const glm::vec3& lightDir)
{
    glViewport(0, 0, shadowWidth, shadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::vec3 lightPos = -lightDir * 50.0f;
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    glm::mat4 lightProj = glm::ortho(-100.0f, 100.0f, -80.0f, 80.0f, 1.0f, 150.0f);

    lightSpaceMatrix = lightProj * lightView;

    glUseProgram(depthShader);
    GLuint lightSpaceMatrixLoc = glGetUniformLocation(depthShader, "lightSpaceMatrix");
    glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
}

void ShadowMapper::endDepthPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMapper::bindShadowUniforms(GLuint mainShader) const
{
    GLuint lightSpaceMatrixLoc = glGetUniformLocation(mainShader, "lightSpaceMatrix");
    GLuint shadowMapLoc = glGetUniformLocation(mainShader, "shadowMap");

    glUniformMatrix4fv(lightSpaceMatrixLoc, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glUniform1i(shadowMapLoc, 1); // Shadow map bound to texture unit 1
}

glm::mat4 ShadowMapper::getLightSpaceMatrix() const {
    return lightSpaceMatrix;
}
