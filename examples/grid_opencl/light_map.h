// LightMapPass.hpp
#pragma once
#include <glad/glad.h>
#include <memory>
#include <OpenGLLib/buffer.h>
#include <OpenGLLib/shader.h>

struct LightMapPass {
  // IDs de FBO y textura
  GLuint fbo     = 0;
  GLuint lightTex = 0;
  int    width   = 0, height = 0;

  // Shaders
  std::unique_ptr<Shader> lightShader;
  std::unique_ptr<Shader> compositeShader;

  // VAO/VBO para quad fullscreen
  GLuint quadVAO = 0, quadVBO = 0;

  // Debe llamarse una vez, tras crear el contexto GL
  void setup(int w, int h) {
    width = w; height = h;
    // 1) FBO + textura
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &lightTex);
    glBindTexture(GL_TEXTURE_2D, lightTex);
    // en lugar de GL_R16F usa GL_RG16F
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, nullptr);

    // glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D,
                           lightTex, 0);

    GLenum bufs = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &bufs);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2) Carga shaders
    lightShader     = std::make_unique<Shader>(SHADER_PATH "light_map.vert",
                                               SHADER_PATH "light_map.frag");
    compositeShader = std::make_unique<Shader>(SHADER_PATH "comp_vert.glsl",
                                               SHADER_PATH "comp_frag.glsl");

    // 3) Prepara quad fullscreen
    float quadVerts[] = {
      // pos   // uv
      -1, -1,  0, 0,
       1, -1,  1, 0,
      -1,  1,  0, 1,
      -1,  1,  0, 1,
       1, -1,  1, 0,
       1,  1,  1, 1,
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    // layout 0 = position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4*sizeof(float), (void*)0);
    // layout 1 = uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4*sizeof(float), (void*)(2*sizeof(float)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
  }

  // Si cambias tamaño de ventana
  void resize(int w, int h) {
    if (w==width && h==height) return;
    width = w; height = h;
    // re-crea la textura
    glBindTexture(GL_TEXTURE_2D, lightTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  // Paso 1: render al FBO para acumular luz
  void renderLightMap(InstanceBuffer& buf) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0,0,width,height);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    lightShader->use();
    buf.draw();

    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  // Paso 2: compositar la pantalla
  void composite(const glm::vec2 &gridSize, const float &radius, const float &intensity) {
    glViewport(0,0,width,height);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    compositeShader->use();
    compositeShader->set("uLightMap", 0);
    compositeShader->set("uGridSize", gridSize);

    compositeShader->set("uRadius", radius);   // ajusta a tu gusto
    compositeShader->set("uIntensity", intensity);   // ajusta a tu gusto

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, lightTex);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
  }

  ~LightMapPass() {
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &lightTex);
    glDeleteBuffers(1, &quadVBO);
    glDeleteVertexArrays(1, &quadVAO);
  }
};
