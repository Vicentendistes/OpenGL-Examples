#include <glad/glad.h>
#include <glcore/buffer.h>
#include <utility>

using vec = std::vector<float>;
using uvec = std::vector<unsigned int>;

Buffer::~Buffer() {}

/******************* VERTEX BUFFER **************************************/

VertexBuffer::VertexBuffer() {
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
}

VertexBuffer::VertexBuffer(const vec &vertices) : VertexBuffer() {
  vertices_ = vertices;
}
VertexBuffer::VertexBuffer(VertexBuffer &&o)
    : vao_(o.vao_), vbo_(o.vbo_), vertices_(std::move(o.vertices_)) {
  o.vao_ = 0;
  o.vbo_ = 0;
}

VertexBuffer &VertexBuffer::operator=(VertexBuffer &&o) {
  if (vao_) {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
  }
  vao_ = o.vao_;
  vbo_ = o.vbo_;
  vertices_ = std::move(o.vertices_);
  o.vao_ = 0;
  o.vbo_ = 0;
  return *this;
}

VertexBuffer::~VertexBuffer() {
  if (vao_) {
    glDeleteVertexArrays(1, &vao_);
    glDeleteBuffers(1, &vbo_);
  }
}

void VertexBuffer::draw() {
  bind();
  glDrawArrays(GL_POINTS, 0, vertices_.size()/3);
}

void VertexBuffer::build() {
  bind();
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_.size(),
               vertices_.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
}

void VertexBuffer::bind() {
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
}

/******************* BASIC BUFFER **************************************/

BasicBuffer::BasicBuffer() : VertexBuffer() { glGenBuffers(1, &ebo_); }

BasicBuffer::BasicBuffer(const vec &vertices) : BasicBuffer() {
  vertices_ = vertices;
}

BasicBuffer::BasicBuffer(const vec &vertices, const uvec &indices)
    : BasicBuffer(vertices) {
  indices_ = indices;
}

BasicBuffer::BasicBuffer(BasicBuffer &&o)
    : VertexBuffer(std::move(o)), indices_(std::move(o.indices_)), ebo_(o.ebo_) {
  o.ebo_ = 0;
}

BasicBuffer &BasicBuffer::operator=(BasicBuffer &&o) {
  if (vao_)
    glDeleteBuffers(1, &ebo_);
  VertexBuffer::operator=(std::move(o));
  ebo_ = o.ebo_;
  indices_ = std::move(o.indices_);
  o.ebo_ = 0;
  return *this;
}

BasicBuffer::~BasicBuffer() {
  if (vao_)
    glDeleteBuffers(1, &ebo_);
}

void BasicBuffer::draw() {
  bind();
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
}

void BasicBuffer::bind() {
  glBindVertexArray(vao_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
}

void BasicBuffer::build() {
  bind();
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_.size(),
               vertices_.data(), GL_STATIC_DRAW);

  if (indices_.size() == 0) {
    for (unsigned int i = 0; i < vertices_.size(); i++) {
      indices_.push_back(i);
    }
  }
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices_.size(),
               indices_.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
}

/******************* TEXTURE BUFFER **************************************/

void TextureBuffer::addTexture(Texture *texture) {
  textures_.push_back(texture);
}

void TextureBuffer::build() {
  bind();
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_.size(),
               vertices_.data(), GL_STATIC_DRAW);
  if (indices_.size() == 0) {
    for (unsigned int i = 0; i < vertices_.size(); i++) {
      indices_.push_back(i);
    }
  }
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices_.size(),
               indices_.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
}

void TextureBuffer::draw() {
  for (size_t i = 0; i < textures_.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    textures_[i]->bind();
  }
  bind();
  glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
}
