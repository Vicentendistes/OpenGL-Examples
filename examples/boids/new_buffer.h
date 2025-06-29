#pragma once
#include <glad/glad.h>
#include <vector>
#include <cstddef>
#include <type_traits>

// === BUFFER TYPE ENUM ===
enum class BufferType
{
    Vertex,
    Element,
    Storage
};

// === GLBuffer ===
class GLBuffer
{
public:
    GLBuffer(BufferType type)
    {
        glGenBuffers(1, &bufferID_);
        switch (type)
        {
        case BufferType::Vertex:
            target_ = GL_ARRAY_BUFFER;
            break;
        case BufferType::Element:
            target_ = GL_ELEMENT_ARRAY_BUFFER;
            break;
        case BufferType::Storage:
            target_ = GL_SHADER_STORAGE_BUFFER;
            break;
        }
    }

    ~GLBuffer()
    {
        glDeleteBuffers(1, &bufferID_);
    }

    void bind() const { glBindBuffer(target_, bufferID_); }
    void unbind() const { glBindBuffer(target_, 0); }

    void setData(GLsizeiptr size, const void *data, GLenum usage) const
    {
        bind();
        glBufferData(target_, size, data, usage);
    }

    GLuint id() const { return bufferID_; }

private:
    GLuint bufferID_;
    GLenum target_;
};

class GLVertexArray
{
public:
    GLVertexArray()
    {
        glGenVertexArrays(1, &vaoID_);
    }

    ~GLVertexArray()
    {
        glDeleteVertexArrays(1, &vaoID_);
    }

    void bind() const { glBindVertexArray(vaoID_); }
    void unbind() const { glBindVertexArray(0); }
    GLuint id() const { return vaoID_; }

    // Link VBO to this VAO
    GLVertexArray &bindBuffer(const GLBuffer &vbo)
    {
        bind();
        vbo.bind();
        return *this;
    }

    // Add attribute
    template <typename T>
    GLVertexArray &push(int count, bool normalized = false)
    {
        static_assert(is_supported<T>(), "Unsupported type");
        GLboolean norm = normalized ? GL_TRUE : GL_FALSE;
        attributes_.push_back({currentIndex_++,
                               count,
                               glType<T>(),
                               norm,
                               stride_});
        stride_ += count * sizeof(T);
        return *this;
    }

    // Finalize and apply layout
    void apply()
    {
        bind();
        for (const auto &attr : attributes_)
        {
            glEnableVertexAttribArray(attr.index);
            glVertexAttribPointer(attr.index, attr.count, attr.type,
                                  attr.normalized, stride_, (const void *)attr.offset);
        }
    }

private:
    GLuint vaoID_;
    struct Attribute
    {
        GLuint index;
        GLint count;
        GLenum type;
        GLboolean normalized;
        GLsizei offset;
    };

    std::vector<Attribute> attributes_;
    GLsizei stride_ = 0;
    GLuint currentIndex_ = 0;

    // Type traits
    template <typename T>
    static constexpr bool is_supported();
    template <typename T>
    static GLenum glType();
};

// === Type support specializations
template <>
constexpr bool GLVertexArray::is_supported<float>() { return true; }
template <>
constexpr bool GLVertexArray::is_supported<int>() { return true; }
template <>
constexpr bool GLVertexArray::is_supported<unsigned int>() { return true; }
template <>
constexpr bool GLVertexArray::is_supported<char>() { return true; }

template <>
GLenum GLVertexArray::glType<float>() { return GL_FLOAT; }
template <>
GLenum GLVertexArray::glType<int>() { return GL_INT; }
template <>
GLenum GLVertexArray::glType<unsigned int>() { return GL_UNSIGNED_INT; }
template <>
GLenum GLVertexArray::glType<char>() { return GL_BYTE; }
