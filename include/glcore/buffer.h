#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "glcore/texture.h"
#include <vector>

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
#endif // _BUFFER_H_
