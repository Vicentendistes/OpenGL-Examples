#include <glad/glad.h>
#include <OpenGLLib/buffer.h>
#include <utility>
#include <iostream>

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



/******************* INSTANCE BUFFER **************************************/

InstanceBuffer::InstanceBuffer(const std::vector<float>& verts,
                               const std::vector<glm::vec2>& offs)
  : VertexBuffer(verts),
    offsets_(offs),
    instanceVBO_(0),
    stateVBO_(0)
{}

InstanceBuffer::InstanceBuffer(InstanceBuffer&& o) noexcept
  : VertexBuffer(std::move(o)),
    offsets_(std::move(o.offsets_)),
    instanceVBO_(o.instanceVBO_),
    stateVBO_(o.stateVBO_)
{
  o.instanceVBO_ = o.stateVBO_ = 0;
}

InstanceBuffer& InstanceBuffer::operator=(InstanceBuffer&& o) noexcept {
  if (this != &o) {
    if (instanceVBO_) glDeleteBuffers(1, &instanceVBO_);
    if (stateVBO_)    glDeleteBuffers(1, &stateVBO_);
    VertexBuffer::operator=(std::move(o));
    offsets_    = std::move(o.offsets_);
    instanceVBO_= o.instanceVBO_;
    stateVBO_   = o.stateVBO_;
    o.instanceVBO_ = o.stateVBO_ = 0;
  }
  return *this;
}

InstanceBuffer::~InstanceBuffer() {
  if (instanceVBO_) glDeleteBuffers(1, &instanceVBO_);
  if (stateVBO_)    glDeleteBuffers(1, &stateVBO_);
}

// build sólo una vez (vertices + offsets + reserva stateVBO_)
void InstanceBuffer::build() {
  // 1) vértices
  bind();  
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER,
               vertices_.size()*sizeof(float),
               vertices_.data(),
               GL_STATIC_DRAW);
  // atributo 0 = vec2 pos
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,
                        2*sizeof(float),(void*)0);

  // 2) offsets
  glGenBuffers(1,&instanceVBO_);
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
  glBufferData(GL_ARRAY_BUFFER,
               offsets_.size()*sizeof(glm::vec2),
               offsets_.data(),
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,
                        sizeof(glm::vec2),(void*)0);
  glVertexAttribDivisor(1,1);

  // 3) estados (reserva sólo espacio, datos vendrán por setStateData)
  glGenBuffers(1,&stateVBO_);
  glBindBuffer(GL_ARRAY_BUFFER, stateVBO_);
  glBufferData(GL_ARRAY_BUFFER,
               offsets_.size()*sizeof(uint8_t),
               nullptr,
               GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(2);
  // IPointer para enteros:
  glVertexAttribIPointer(2,1,GL_UNSIGNED_BYTE,
                         sizeof(uint8_t),(void*)0);
  glVertexAttribDivisor(2,1);

  // unbind final
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER,0);
}

// Actualizar dinámicamente los estados de la grilla
void InstanceBuffer::setStateData(const unsigned char* states, size_t count) {
  glBindBuffer(GL_ARRAY_BUFFER, stateVBO_);
  glBufferSubData(GL_ARRAY_BUFFER,
                  0,
                  count * sizeof(uint8_t),
                  states);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// draw
void InstanceBuffer::draw() {
  bind();
  glDrawArraysInstanced(GL_TRIANGLES,
                        0, 6,
                        static_cast<GLsizei>(offsets_.size()));
  glBindVertexArray(0);
}

void InstanceBuffer::bind() {
  glBindVertexArray(vao_);
}
