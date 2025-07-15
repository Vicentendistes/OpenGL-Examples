#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <glad/glad.h>   // Provide GLuint, GLenum, etc.
#include "OpenGLLib/texture.h"
#include <vector>
#include <glm/glm.hpp>

class Buffer {
public:
  virtual void draw() = 0;
  virtual void build() = 0;
  virtual ~Buffer() = 0;
protected:
  virtual void bind() = 0;
};

class VertexBuffer: public Buffer {
public:
  VertexBuffer();
  VertexBuffer(const std::vector<float> &vertices);
  VertexBuffer(VertexBuffer &&);
  VertexBuffer &operator=(VertexBuffer &&);
  ~VertexBuffer() override;

  void draw() override;
  void build() override;

protected:
  void bind() override;
  unsigned int vao_, vbo_;
  std::vector<float> vertices_;
};

class BasicBuffer: public VertexBuffer {
public:
  BasicBuffer();
  BasicBuffer(const std::vector<float> &vertices);
  BasicBuffer(const std::vector<float> &vertices,
              const std::vector<unsigned int> &indices);
  BasicBuffer(BasicBuffer &&);
  BasicBuffer &operator=(BasicBuffer &&);
  ~BasicBuffer() override;

  void draw() override;
  void build() override;

protected:
  void bind() override;
  unsigned int ebo_;
  std::vector<unsigned int> indices_;
};

class TextureBuffer : public BasicBuffer {
public:
  using BasicBuffer::BasicBuffer; // inherit constructors

  void draw() override;
  void addTexture(Texture *texture);
  void build() override;

private:
  std::vector<Texture *> textures_;
};

class InstanceBuffer : public VertexBuffer {
public:
    // Solo vertices (2 floats por vértice) y offsets
    InstanceBuffer(const std::vector<float>& vertices,
                   const std::vector<glm::vec2>& offsets);
    InstanceBuffer(InstanceBuffer&&) noexcept;
    InstanceBuffer& operator=(InstanceBuffer&&) noexcept;
    ~InstanceBuffer() override;

    void build() override;
    void draw() override;

    // Para actualizar offsets en tiempo de ejecución
    void setInstanceData(const std::vector<glm::vec2>& offsets);
    void setStateData(const unsigned char* states, size_t count);
    
    GLuint getStateVBO() const { 
        return stateVBO_; 
    }
protected:
    void bind() override;

private:
    unsigned int instanceVBO_{0};
    unsigned int stateVBO_{0};    
    std::vector<glm::vec2> offsets_;
};

#endif
